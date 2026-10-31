-- All dual-stick decisions live here or in translate.lua. The host only supplies
-- physical pads, local rider observations and short-lived controller/preference writes.
local engine = require("translate")
local config = require("config")
local ini = require("ini")
local controls = require("controls")
local M = {}
local names = {up=1, down=2, left=4, right=8, start=16, back=32, left_stick=64,
    right_stick=128, lb=256, rb=512, a=4096, b=8192, x=16384, y=32768}
local preferences = {"egf_holdfakiestance", "egf_holdfakiepush", "HeldFlipTricks",
    "egf_cameraenablemirrorinput", "egf_cameraallowautomirror"}
local function n(v) return if v then 1 else 0 end
local function round(v) return if v < 0 then math.ceil(v - 0.5) else math.floor(v + 0.5) end
local function raw_axis(v) return round(math.clamp(v or 0, -1, 1) * (if (v or 0) < 0 then 32768 else 32767)) end
local function axis(v) return v / (if v < 0 then 32768 else 32767) end
local function raw_pad(p)
    local out = engine.new_pad()
    for name, mask in names do if p.buttons[name] then out.buttons = bit32.bor(out.buttons, mask) end end
    out.lx, out.ly, out.rx, out.ry = raw_axis(p.left_x), raw_axis(p.left_y), raw_axis(p.right_x), raw_axis(p.right_y)
    out.lt, out.rt = round((p.left_trigger or 0)*255), round((p.right_trigger or 0)*255)
    return out
end
local function normalized(p)
    local buttons = {}
    for name, mask in names do buttons[name] = bit32.band(p.buttons, mask) ~= 0 end
    return {left_x=axis(p.lx), left_y=axis(p.ly), right_x=axis(p.rx), right_y=axis(p.ry),
        left_trigger=p.lt/255, right_trigger=p.rt/255, buttons=buttons}
end
local function feet_neutral(p, deadzone)
    return math.abs(axis(p.lx)) <= deadzone and math.abs(axis(p.ly)) <= deadzone and
        math.abs(axis(p.rx)) <= deadzone and math.abs(axis(p.ry)) <= deadzone
end
local function neutral(p, deadzone) return feet_neutral(p, deadzone) and p.lt < 8 and p.rt < 8 and p.buttons == 0 end
local function arr(v) return {[0]=v.x, [1]=v.y, [2]=v.z} end
local function observation(values, name, fallback)
    local field = values["Animation/Dingo/" .. name]
    return if field then field.value else fallback
end
function M.new()
    local self = {enabled=nil, status="Waiting for controller", controller=nil, owner=nil, config_key=nil,
        state=engine.new_state(), catch=engine.new_catch(), camera=engine.new_camera(),
        tracking=engine.new_stance(), motion=engine.new_motion(), agreement=engine.new_agreement(),
        absolute=engine.new_absolute(), switched=0, paused=true, air_since=nil, last_state=0,
        landing_until=0, neutral_since=nil, return_until=0, return_sent=false, writes=0,
        toggle_since=nil, toggle_latched=false, held={}, frames=0, max_tick=0}
    self.available = type(reskate.skater.select_rail_target) == "function" and type(reskate.skater.rail_target) == "function" and type(reskate.input.write) == "function" and type(reskate.skater.frame) == "function" and
        type(reskate.skater.animation_values) == "function" and type(reskate.skater.set_animation_value) == "function" and
        type(reskate.settings.hold_gameplay) == "function"
    function self:release()
        if self.available then
            reskate.skater.select_rail_target(nil)
            if self.controller ~= nil then reskate.input.write(nil, self.controller) end
            for key in self.held do reskate.settings.hold_gameplay(key, nil) end
        end
        table.clear(self.held)
    end
    function self:suspend(reason)
        self:release()
        self.status = reason
        if not self.paused then
            self.state, self.catch, self.camera = engine.new_state(), engine.new_catch(), engine.new_camera()
        end
        self.paused, self.air_since, self.neutral_since = true, nil, nil
        self.landing_until, self.return_until, self.return_sent, self.writes = 0, 0, false, 0
        self.toggle_since, self.toggle_latched = nil, false
        self.last_state = 0
    end
    function self:hold(name, value)
        if value == nil then
            if self.held[name] then reskate.settings.hold_gameplay(name, nil); self.held[name] = nil end
        else
            local ok = reskate.settings.hold_gameplay(name, value)
            if ok then self.held[name] = true end
        end
    end
    function self:set_enabled(value)
        self.enabled = value
        self:suspend(if value then "Waiting for neutral controls" else "Off: standard controls")
    end
    function self:read_controller(doc)
        local setting = ini.get(doc, "SessionControls", "Controller", "auto")
        local selected = tonumber(setting)
        if setting == "playstation" then selected = setting end
        if selected ~= nil and (selected == "playstation" or (selected == math.floor(selected) and selected >= 0 and selected <= 3)) then
            return reskate.input.gamepad(selected), selected
        end
        if self.controller ~= nil then
            local p = reskate.input.gamepad(self.controller)
            if p then return p, self.controller end
        end
        for i = 0, 3 do local p = reskate.input.gamepad(i); if p then return p, i end end
        return reskate.input.gamepad("playstation"), "playstation"
    end
    function self:tick(doc)
        if not doc then return self:suspend("Settings could not be read") end
        if not self.available then self.status = "Requires the controller API update for ReSkate"; return end
        if self.enabled == nil then self.enabled = controls.enabled_at_start(doc) end
        local now = reskate.time.now()
        if self.last_tick then self.max_tick = math.max(self.max_tick, now-self.last_tick) end
        self.last_tick = now
        if not reskate.input.allowed() then return self:suspend("Paused: game or ReSkate menu owns input") end
        local physical, controller = self:read_controller(doc)
        if not physical then return self:suspend("Controller disconnected") end
        if controller ~= self.controller then
            self:suspend("Controller changed")
            self.controller, self.owner = controller, nil
        end
        local raw = raw_pad(physical)
        -- Hold all four shoulder/stick buttons for three seconds. One toggle per hold.
        local chord = bit32.band(raw.buttons, 0x03c0) == 0x03c0
        if chord then
            self.toggle_since = self.toggle_since or now
            if now-self.toggle_since >= 3000 and not self.toggle_latched then
                self.enabled, self.toggle_latched = not self.enabled, true
                self:release()
                self.state, self.catch = engine.new_state(), engine.new_catch()
                self.paused = true
            end
        else self.toggle_since, self.toggle_latched = nil, false end
        if not self.enabled then self:release(); self.status="Off: standard controls"; return end
        local rider = reskate.skater.get()
        local telemetry = reskate.skater.telemetry()
        local frame = reskate.skater.frame()
        if not telemetry or not telemetry.watch or not telemetry.watch.valid then
            return self:suspend("Waiting for local skater observations")
        end
        local state = telemetry.watch.state
        if telemetry.watch.ragdoll == true or state == 300 or state == 700 then
            self.owner = nil
            return self:suspend("Loading or bailed: standard controls")
        end
        if state == 504 then
            -- Retain completed physical-foot turns through a normal dismount.
            -- The native frame can reset while walking; it is not a new rider.
            local previous = self.last_config
            if previous then
                self.switched = engine.resolve_stance(self.tracking,4,previous.goofy,previous.switch_flip,previous.front_back,504,-1,0,self.switched,now)
                engine.motion_step(self.motion,504,0,0,0,0,0,-1,now)
            end
            return self:suspend("Walking: standard controls")
        end
        if not rider or not frame then return self:suspend("Waiting for local rider frame") end
        if not ((state >= 100 and state < 500) or state == 601 or state == 701) then
            return self:suspend("Unknown skater state: standard controls")
        end
        local c = config.read(doc)
        self.last_config = c
        local stance = controls.stance(doc)
        local automatic = reskate.skater.goofy()
        c.goofy = if stance == 0 and automatic ~= nil then n(automatic) elseif stance == 2 or (stance == 0 and string.lower(ini.get(doc,"SessionControls","Stance") or "regular")=="goofy") then 1 else 0
        local config_key = tostring(c.goofy)..":"..c.xl_layout..":"..c.front_back..":"..c.switch_flip
        if self.owner ~= frame.serial or self.config_key ~= config_key then
            self:suspend("Synchronizing rider and stance")
            self.owner, self.config_key = frame.serial, config_key
            self.tracking, self.motion, self.agreement, self.absolute = engine.new_stance(), engine.new_motion(), engine.new_agreement(), engine.new_absolute()
            self.switched = n(frame.fakie)
        end
        if self.paused then
            -- A held setup when a menu closes must be released before gestures can start.
            if not neutral(raw, c.deadzone) then self.status="Release controls to resume"; return end
            self.paused = false
            engine.camera_suspend(self.camera, raw.buttons)
        end
        local s = self.state
        local is_air = state >= 200 and state < 300
        if is_air then self.air_since = self.air_since or now else self.air_since = nil end
        s.air = n(is_air and now-self.air_since < 6000 and now-self.air_since >= math.clamp(ini.number(doc,"SessionControls","AirDelayMs",120),0,6000))
        s.takeoff, s.game_known, s.game_crouch = n(is_air), 1, n(state==103)
        s.grind = engine.is_grind(state)
        s.stock_ramp_valid, s.stock_ramp_vertical = n(state==100), frame.forward.y
        s.stock_ramp_departing = n(state==103 or state==200 or state==201)
        local native = n(frame.fakie)
        s.sw_raw, s.game_switch_known, s.game_switch = n(frame.backwards), 1, native
        s.game_mirror_known, s.xl_absolute_raw = 0, -1
        local idle, feet_idle = neutral(raw,c.deadzone), feet_neutral(raw,c.deadzone)
        local f = engine.new_frame()
        -- These are opaque lifecycle IDs used only for equality in the pure state machine.
        f.selector, f.core, f.context, f.source = frame.serial, frame.serial, frame.serial, frame.serial
        f.basis, f.native_frame, f.mirrored = n(frame.basis), native, n(frame.mirrored)
        f.position, f.forward = arr(frame.position), arr(frame.forward)
        local physical_feet = engine.physical_foot_layout(c.front_back,c.switch_flip) ~= 0
        local agreement = -1
        if physical_feet then agreement=engine.agreement_step(self.agreement,f,state,n(idle),c.goofy,now)
        else engine.agreement_missing(self.agreement) end
        local physical_raw = -1
        if physical_feet and self.agreement.baseline_valid ~= 0 then
            physical_raw = engine.physical_raw(native,n(frame.mirrored),self.agreement.mirror)
            s.xl_absolute_raw = physical_raw
            s.game_mirror_known, s.game_mirror = 1, bit32.bxor(n(frame.mirrored),self.agreement.mirror)
        end
        self.tracking.direct_neutral = n(idle)
        local motion_raw = -1
        if ini.number(doc,"SessionControls","RemountStanceMotion",1)~=0 and c.goofy == 0 and not(c.switch_flip==3 and c.front_back==0) then
            motion_raw=engine.motion_step(self.motion,state,1,frame.position.x,frame.position.z,frame.forward.x,frame.forward.z,native,now)
        end
        local raw_stance = if motion_raw >= 0 then motion_raw else native
        local switched = engine.resolve_stance(self.tracking,4,c.goofy,c.switch_flip,c.front_back,state,raw_stance,0,self.switched,now)
        if physical_feet then
            switched=engine.absolute_step(self.absolute,physical_raw,switched,now)
            if agreement >= 0 and agreement ~= switched and state==100 and feet_idle then
                switched=agreement
                self.tracking.stance,self.tracking.reference,self.tracking.takeoff_stance,self.tracking.pending_reference=agreement,-1,-1,-1
                self.tracking.saw_air_edge,self.tracking.air_zero_pending,self.tracking.turn_time=0,0,0
                self.tracking.prev_state,self.tracking.prev_raw,self.tracking.rode_before_walk=100,raw_stance,1
                self.absolute.valid,self.absolute.stance,self.absolute.pending=1,agreement,0
            end
        end
        if motion_raw>=0 or c.xl_layout~=0 then
            s.game_switch=engine.output_frame(c.xl_layout,switched,if self.motion.bias_valid~=0 then self.motion.bias else 0,native)
        end
        self.switched, s.switched = switched, switched
        local rail = reskate.skater.rail_target()
        s.entry_rail_target = n(c.native_grind_entry~=0 and rail ~= nil and rail.available and rail.targeting and state==201)
        local values = reskate.skater.animation_values()
        local camera_active=ini.number(doc,"SessionControls","DpadCameraSide",1)~=0
        local request=0
        local translated_raw=table.clone(raw)
        translated_raw.buttons=engine.camera_step(self.camera,raw.buttons,now,n(camera_active),{get=function() return request end,set=function(v) request=v end})
        if request~=0 then self.camera.pending=request end
        if camera_active and self.camera.pending~=0 and now>=self.camera.next_toggle then
            local view=reskate.camera.get()
            local observed=if view then engine.camera_side(arr(frame.position),arr(frame.forward),arr(view.position)) else 0
            if engine.camera_toggle(self.camera,self.camera.pending,observed,now)~=0 then
                self.camera.pulse_until,self.camera.next_toggle=now+80,now+500
            end
            self.camera.pending=0
        end
        local trick=observation(values,"EnumGS.Anim.CurrentFlipTrick",-1)
        local doing=observation(values,"Bool.Anim.DoingTrick",nil)
        local out=engine.catch(c,s,self.catch,translated_raw,now,state,trick,if doing==nil then -1 else n(doing))
        out.buttons=engine.camera_buttons(out.buttons,n(camera_active),n(camera_active and now<self.camera.pulse_until))
        local plan=engine.entry_plan(c,s)
        if plan.active~=0 then reskate.skater.select_rail_target(plan.type,plan.yaw<0) else reskate.skater.select_rail_target(nil) end
        local protected=engine.protect_fakie(c,s,raw,out)~=0
        self:hold(preferences[1],if c.disable_hold_fakie~=0 then protected else nil)
        self:hold(preferences[2],if c.disable_hold_fakie~=0 then false else nil)
        self:hold(preferences[3],if c.manual_catch~=0 then true else nil)
        self:hold(preferences[4],if camera_active then true else nil)
        self:hold(preferences[5],if camera_active then false else nil)
        -- Accelerate the game's own return timer. Never force stance or pose flags.
        if ((self.last_state>=200 and self.last_state<300) or (engine.is_grind(self.last_state)~=0)) and state>=100 and state<200 then
            self.landing_until=now+1200
        end
        local automatic_return=c.fakie_return==2 and now<self.landing_until
        local return_eligible=c.disable_hold_fakie~=0 and c.fakie_return~=0 and not protected and
            (state==100 or (automatic_return and state==103)) and (idle or automatic_return)
        local timer=observation(values,"Float.Physics.TimeRidingFakie",nil)
        local riding=observation(values,"Bool.Physics.RidingFakie",nil)
        if not frame.fakie then
            self.return_sent,self.writes,self.return_until=false,0,0
        elseif return_eligible and riding==true and type(timer)=="number" then
            self.neutral_since=self.neutral_since or now
            if (automatic_return or now-self.neutral_since>=20) and not self.return_sent then
                self.return_sent,self.writes,self.return_until=true,0,now+120
            end
            if self.return_sent and now<=self.return_until and self.writes<3 and timer>0 and timer<1.6 then
                if reskate.skater.set_animation_value("Animation/Dingo/Float.Physics.TimeRidingFakie",1.6) then self.writes+=1 end
            end
        else self.neutral_since,self.return_until=nil,0 end
        self.last_state=state
        local ok,why=reskate.input.write(normalized(out),self.controller)
        if not ok then return self:suspend("Output unavailable: "..tostring(why)) end
        self.frames+=1
        self.status=string.format("%s; %s; controller %s",if self.enabled then "On" else "Off",if switched~=0 then "switch" else "normal",tostring(self.controller))
    end
    return self
end
M.raw_pad, M.normalized = raw_pad, normalized
return M
