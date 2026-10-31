-- Dual Stick Controls v6.6 Luau port. No companion DLL is loaded by this mod.
local ini = require("ini")
local controls = require("controls")

local runtime = require("runtime").new()
local configuration = require("config")
local file = "session_controls.ini"
local doc = nil          -- the file as last read, parsed
local read_at = -math.huge
local missing = false
local notice = nil       -- what the last change did, for the page
local slot = 1
local overwrite = 0      -- the slot a second Save replaces

-- Reload at most once a second so external settings edits are picked up.
local function refresh(force)
    local now = reskate.time.now()
    if not force and now - read_at < 1000 then return doc end
    read_at = now
    local ok, text, why = pcall(reskate.files.read, file)
    if ok and type(text) == "string" then
        doc, missing = ini.parse(text), false
    elseif ok and why == "not found" then
        local fresh = configuration.fresh()
        local saved, problem = reskate.files.write(file, ini.dump(fresh))
        if saved then doc, missing = fresh, false else doc, missing, notice = nil, true, tostring(problem) end
    else doc, missing, notice = nil, true, tostring(if ok then why else text) end
    return doc
end
-- Changes the file with `change(doc)` and writes it; what happened, for the page and the console.
local function change(what)
    local current = refresh(true)
    if not current then return "Settings are unavailable; see the plugin page." end
    if what(current) == false then return "nothing changed." end
    local ok, why = reskate.files.write(file, ini.dump(current))
    if not ok then
        refresh(true)
        return "session_controls.ini could not be saved: " .. tostring(why)
    end
    runtime:suspend("Settings changed; release controls to resume")
    return "saved. Controls update immediately."
end
local function apply(what)
    overwrite = 0
    notice = change(what)
end

-- ---- the page ---------------------------------------------------------------------------------------
local function dropdown(ui, id, label, names, chosen, help)
    local picked = ui:dropdown(id, label, names, chosen, help)
    return if picked ~= chosen then picked else nil
end

reskate.menu.page("Dual Stick", function(ui)
    local d = refresh(false)
    ui:card("DUAL STICK CONTROLS", runtime.status)
    if not runtime.available then ui:warn("Install the ReSkate build with the controller API update before enabling this mod.") end
    if not d then
        ui:warn("Settings could not be loaded: " .. tostring(notice))
        return
    end

    local live = ui:toggle("enabled", "Dual Stick enabled", runtime.enabled ~= false, "Live switch. Hold LB + RB + L3 + R3 for three seconds to toggle from the pad.")
    if live ~= (runtime.enabled ~= false) then runtime:set_enabled(live) end
    ui:card("PRESETS")
    if ui:button("session", if controls.active(d, false) then "Session-style preset (active)" else "Apply the Session-style preset") then
        apply(function(x) return controls.preset(x, false) end)
    end
    if ui:button("xl", if controls.active(d, true) then "XL-style preset (active)" else "Apply the XL-style preset") then
        apply(function(x) return controls.preset(x, true) end)
    end
    ui:note("Both presets translate Skate controls across two sticks. Their inputs differ from Session and Skater XL.")
    ui:note("Session: front/back feet, flick pop, trigger steering, one-stick flips blocked. XL: fixed physical setup directions, " ..
        "jobs swap in switch, release pop, left-stick steering, one-stick flips allowed.")

    ui:card("CUSTOM SLOTS")
    local names = {}
    for i = 1, controls.slots do names[i] = "Slot " .. i .. (if controls.filled(d, i) then "" else " (empty)") end
    local picked = ui:dropdown("slot", "Custom slot", names, slot)
    if picked ~= slot then slot, overwrite = picked, 0 end
    if controls.filled(d, slot) and ui:button("load", "Load slot " .. slot) then
        apply(function(x) return controls.load(x, slot) end)
    end
    if ui:button("save", if overwrite == slot then "Confirm: replace slot " .. slot else "Save to slot " .. slot) then
        if controls.filled(d, slot) and overwrite ~= slot then
            overwrite = slot
            notice = "slot " .. slot .. " holds controls already: press again to replace them."
        else
            apply(function(x) return controls.save(x, slot) end)
        end
    end
    ui:note("Loading restores the controls below; the controller, your base stance, menu preferences and other tuning stay as they are.")

    ui:card("YOUR SETTINGS")
    local controllers = {"Automatic", "XInput 1", "XInput 2", "XInput 3", "XInput 4", "Native PlayStation"}
    local current_controller = ini.get(d,"SessionControls","Controller") or "auto"
    local cnum = tonumber(current_controller)
    local controller_index = if current_controller=="playstation" then 6 elseif cnum and cnum==math.floor(cnum) and cnum>=0 and cnum<=3 then cnum+2 else 1
    local picked_controller = dropdown(ui,"controller","Controller",controllers,controller_index)
    if picked_controller then apply(function(x) ini.set(x,"SessionControls","Controller",if picked_controller==1 then "auto" elseif picked_controller==6 then "playstation" else tostring(picked_controller-2)) end) end
    local stance = dropdown(ui, "stance", "Stance", controls.stances, controls.stance(d) + 1)
    if stance then apply(function(x) controls.set_stance(x, stance - 1) end) end
    local layout = controls.layout(d)
    local layouts = {controls.layouts[1], controls.layouts[2], controls.layouts[3]}
    if layout == 3 then layouts[4] = controls.layouts[4] end
    local picked_layout = dropdown(ui, "layout", "Stick layout", layouts, layout + 1)
    if picked_layout and picked_layout <= 3 then apply(function(x) return controls.set_layout(x, picked_layout - 1) end) end
    for i, c in controls.choices do
        local value, index = controls.choice(d, c)
        local labels = table.clone(c.labels)
        if not index then
            table.insert(labels, "Custom (" .. value .. ")")
            index = #labels
        end
        local chosen = dropdown(ui, "c" .. i, c.label, labels, index)
        if chosen and chosen <= #c.values then apply(function(x) controls.set_choice(x, c, chosen) end) end
    end
    local angle = controls.angle(d)
    local moved = math.floor(ui:slider("angle", "Stick steering angle", math.clamp(angle, 5, 60), 5, 60, "%d deg") + 0.5)
    if moved ~= math.clamp(angle, 5, 60) then apply(function(x) controls.set_angle(x, moved) end) end
    local ramp = dropdown(ui, "ramp", "Stock ramp controls", controls.ramps, controls.ramp(d) + 1,
        "Experimental. Qualified transitions use physical Skate stick controls and trigger grabs; flat tricks keep foot controls. Pumping stays on R3.")
    if ramp then apply(function(x) controls.set_ramp(x, ramp - 1) end) end
    local start = ui:toggle("start", "Dual Stick on next launch", controls.enabled_at_start(d),
        "Startup preference; use Dual Stick enabled above to change it now.")
    if start ~= controls.enabled_at_start(d) then apply(function(x) controls.set_enabled_at_start(x, start) end) end
    ui:note("Fakie return: When neutral waits for released controls; After landing also requests return after air/rail landings. " ..
        "Held manual/setup input and handplants can defer return.")
    ui:note("D-pad camera sides: tap left/right for the camera; hold half a second for the original action. " ..
        "Native L3 camera switching is suppressed while enabled; trick and catch clicks remain available.")
    ui:note("Manual Catch uses L3/R3; some tricks settle at an angle and still need catch timing.")
    ui:note("Release held inputs after closing a menu. Settings live in this plugin’s data folder; existing INI files can be copied there.")
    if notice then ui:info("Last change", string.sub(notice, 1, 200)) end
end)

-- ---- the console ------------------------------------------------------------------------------------
reskate.console.command("dualstick", "Dual Stick Controls' settings: status, on|off, preset session|xl, load <slot>, save <slot>, test", function(args)
    local function say(text) reskate.console.print("dualstick: " .. text) end
    local what = args[1]
    if what == "test" then
        local passed, failures = require("tests").run()
        for _, failure in failures do say("FAIL " .. failure) end
        return say(string.format("%d passed, %d failed", passed, #failures))
    end
    if what == "on" or what == "off" then runtime:set_enabled(what == "on"); return say(runtime.status) end
    local d = refresh(true)
    if not d then return say("session_controls.ini is not in Mods/.reskate_plugin_data/" .. reskate.id .. "/: settings could not be loaded.") end
    if what == nil or what == "status" then
        local preset = if controls.active(d, false) then "Session-style" elseif controls.active(d, true) then "XL-style" else "custom"
        say(runtime.status .. string.format("; %d translated frames; longest tick %.1f ms", runtime.frames, runtime.max_tick))
        return say(string.format("%s controls, %s layout, %s stance, steering angle %d deg", preset, controls.layouts[controls.layout(d) + 1],
            string.lower(controls.stances[controls.stance(d) + 1]), controls.angle(d)))
    elseif what == "preset" and (args[2] == "session" or args[2] == "xl") then
        return say(change(function(x) return controls.preset(x, args[2] == "xl") end))
    elseif (what == "load" or what == "save") and tonumber(args[2]) then
        local n = tonumber(args[2])
        if n ~= math.floor(n) or n < 1 or n > controls.slots then return say("a slot is 1 to " .. controls.slots) end
        if what == "load" and not controls.filled(d, n) then return say("slot " .. n .. " is empty") end
        return say(change(function(x) return if what == "load" then controls.load(x, n) else controls.save(x, n) end))
    end
    say("say dualstick status, on|off, preset session|xl, load <slot>, save <slot> or test")
end)

reskate.on("tick", function() runtime:tick(refresh(false)) end)
reskate.on("unload", function() runtime:release() end)
reskate.on("level_unloading", function() runtime:suspend("Level unloading") end)
refresh(true)
reskate.log("Dual Stick Lua loaded; " .. (if runtime.available then "controller API available" else "requires the ReSkate controller API update"))
