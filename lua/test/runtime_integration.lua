local now,allowed,connected=1000,true,true
local writes,holds={},{}
local pad={left_x=0,left_y=0,right_x=0,right_y=0,left_trigger=0,right_trigger=0,buttons={}}
local frame={serial=1,position={x=0,y=0,z=0},forward={x=0,y=0,z=1},fakie=false,mirrored=true,basis=false,backwards=false}
local telemetry={watch={valid=true,state=100,ragdoll=false}}
reskate={
    input={allowed=function() return allowed end,gamepad=function(slot) return if connected and slot==0 then pad else nil end,
        write=function(value,slot) table.insert(writes,{value=value,slot=slot}); return true end},
    skater={select_rail_target=function() return true end,rail_target=function() return {available=true,targeting=false} end,get=function() return {} end,telemetry=function() return telemetry end,frame=function() return frame end,
        goofy=function() return false end,animation_values=function() return {} end,set_animation_value=function() return true end},
    settings={hold_gameplay=function(name,value) table.insert(holds,{name=name,value=value}); return true end},
    camera={get=function() return {position={x=2,y=1,z=-3}} end},time={now=function() return now end}}
local ini,config,controls,rt=require("ini"),require("config"),require("controls"),require("runtime")
local doc=config.fresh()
local runtime=rt.new()
local function tick(ms) now+=ms or 16; runtime:tick(doc) end
local function last() return writes[#writes] end
-- Exact axis/button round trips, including the asymmetric signed endpoints.
for _,v in {-32768,-12000,-1,0,1,12000,32767} do
    local raw={buttons=0x9201,lt=37,rt=255,lx=v,ly=v,rx=v,ry=v}
    local again=rt.raw_pad(rt.normalized(raw))
    assert(again.lx==v and again.rx==v and again.buttons==raw.buttons and again.lt==37 and again.rt==255)
end
tick()
assert(runtime.frames==1 and last().slot==0 and last().value~=nil,"neutral startup writes a selected physical controller")
pad.right_y=-0.8; tick(); assert(last().value~=nil,"live controls translated")
allowed=false; tick(); assert(last().value==nil and runtime.paused,"menus release output")
assert(#holds>0 and holds[#holds].value==nil,"menus release settings holds")
allowed=true; tick(); assert(runtime.paused and last().value==nil,"held setup does not resume after menu")
pad.right_y=0; tick(); assert(not runtime.paused and last().value~=nil,"neutral resumes")
connected=false; tick(); assert(last().value==nil,"disconnect releases output")
connected=true; tick(); assert(last().value~=nil,"reconnect resumes from neutral")
frame.serial=2; pad.left_y=0.8; tick(); assert(runtime.paused and last().value==nil,"new rider does not inherit gestures")
pad.left_y=0; tick(); assert(not runtime.paused,"new rider resumes neutral")
telemetry.watch.state=504; tick(); assert(last().value==nil,"walking uses standard controls")
telemetry.watch.state=100; tick()
runtime:set_enabled(false); tick(); assert(last().value==nil and not runtime.enabled,"live off releases")
runtime:set_enabled(true); tick(); assert(last().value~=nil,"live on resumes")
-- A stale lease conflict must stop output rather than retain an old translation.
reskate.input.write=function(value,slot) table.insert(writes,{value=value,slot=slot}); if value then return false,"another writer" end; return true end
tick(); assert(runtime.paused and last().value==nil,"conflicting writer suspended")
reskate.input.write=function(value,slot) table.insert(writes,{value=value,slot=slot}); return true end
tick(); runtime:release(); assert(last().value==nil,"unload releases")
local passed,failures=require("tests").run(); assert(#failures==0,table.concat(failures,"\n")); assert(passed>=17)
-- A completed XL turn survives walking even when native stance flags reset.
controls.preset(doc,true)
frame.serial=3;frame.fakie=false;telemetry.watch.state=100
local xl=rt.new()
local function xtick(state,fakie) now+=40;telemetry.watch.state=state;frame.fakie=fakie;xl:tick(doc) end
xtick(100,false);xtick(103,false);xtick(200,false);xtick(201,true);xtick(100,true)
assert(xl.switched==1,"completed XL turn is retained")
xtick(504,false);xtick(100,false)
assert(xl.switched==1,"normal dismount does not discard physical foot identity")
xl:release()
-- 6.6: calibrate both physical-foot layouts, while Front/Back stays role based.
for _,layout in {"left-right","xl","front-back"} do
    local d=config.fresh()
    ini.set(d,"SessionControls","FrontBackFoot",if layout=="front-back" then 1 else 0)
    ini.set(d,"SessionControls","SwitchStance",if layout=="front-back" then 1 else 3)
    ini.set(d,"SessionControls","XLFootDirections",if layout=="xl" then 1 else 0)
    frame.serial+=1;frame.fakie=false;frame.mirrored=true;frame.basis=false
    frame.position={x=0,y=0,z=0};telemetry.watch.state=100
    pad.left_x,pad.left_y,pad.right_x,pad.right_y=0,0,0,0
    local physical=rt.new()
    for i=1,20 do now+=40;frame.position.z+=0.16;physical:tick(d) end
    if layout=="front-back" then
        assert(physical.state.game_mirror_known==0,"Front/Back does not inherit physical-foot evidence")
    else
        assert(physical.agreement.baseline_valid==1,"both physical layouts learn the native mirror baseline")
        assert(physical.state.xl_absolute_raw==0 and physical.state.game_mirror_known==1,"Left/Right and XL publish calibrated physical feet")
    end
    telemetry.watch.state=701;pad.right_x=1;now+=40;physical:tick(d)
    assert(physical.state.grind==1 and physical.state.split_grind_active==1,"state 701 keeps foot setup active")
    assert(config.read(d).native_grind_entry==0,"native targeting is opt-in")
    physical:release()
end
-- Capability checks give a useful status on the released host, without writing.
reskate.input.write=nil
local old=rt.new();old:tick(doc);assert(not old.available and old.status:find("Requires"))
