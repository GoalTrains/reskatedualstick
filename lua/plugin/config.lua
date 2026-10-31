local ini = require("ini")
local engine = require("translate")
local M = {}
local fields = {
    {"flip_x", "MirrorFlicks", 0, 1},
    {"switch_invert_x", "SwitchInvertX", 0, 1},
    {"switch_invert_y", "SwitchInvertY", 0, 1},
    {"grab_mode", "GrabMode", 1, 1},
    {"grab_threshold", "GrabTriggerThreshold", 250, 1},
    {"grab_double_ms", "GrabDoubleTapMs", 300, 1},
    {"grab_hold_ms", "GrabHoldMs", 100, 1},
    {"powerslide", "Powerslides", 1, 1},
    {"revert_ms", "RevertDoubleTapMs", 300, 1},
    {"revert_input", "RevertInput", 2, 1},
    {"ps_flip", "PowerslideFlip", 0, 1},
    {"revert_flip", "RevertFlip", 0, 1},
    {"revert_sweep_ms", "RevertSpinMs", 300, 1},
    {"revert_gap_ms", "RevertGapMs", 0, 1},
    {"revert_min_gap_ms", "RevertMinGapMs", 0, 1},
    {"revert_shape", "RevertShape", 5, 1},
    {"switch_flip", "SwitchStance", 1, 1},
    {"prewind", "SpinWindUp", 1, 1},
    {"body_flips", "BodyFlips", 1, 1},
    {"stick_steer", "StickSteering", 0, 1},
    {"stick_steer_angle", "StickSteeringAngleDegrees", 34, 1},
    {"manual_catch", "ManualCatch", 0, 1},
    {"manual_catch_replay", "ManualCatchReplay", 0, 1},
    {"manual_catch_ranges", "ManualCatchNativeRanges", 0, 1},
    {"front_back", "FrontBackFoot", 1, 1},
    {"xl_layout", "XLFootDirections", 0, 1},
    {"disable_hold_fakie", "DisableHoldFakie", 1, 1},
    {"fakie_return", "FakieReturn", 1, 1},
    {"one_foot_flips", "OneFootFlips", 0, 1},
    {"switch_goofy_flip", "SwitchFlipGoofy", 0, 1},
    {"bf_grab_ms", "BodyFlipGrabMs", 80, 1},
    {"bump_ms", "GrindBumpMs", 250, 1},
    {"prewind_steer", "SpinWindUpSteer", 0, 1},
    {"prewind_percent", "SpinWindUpTriggerPercent", 70, 1},
    {"prewind_ms", "SpinWindUpMs", 60, 1},
    {"deadzone", "FootDeadzonePercent", 15, 100},
    {"steer_scale", "SteerStrengthPercent", 100, 100},
    {"steer_curve", "SteerCurvePercent", 100, 100},
    {"active", "PopFlickPercent", 30, 100},
    {"point_ms", "TrickPointMs", 16, 1},
    {"pop_release_ollie", "PopReleaseOllie", 1, 1},
    {"lean_ms", "ManualLeanMs", 120, 1},
    {"ms_per_unit", "TrickDrawMsPerUnit", 28, 1},
    {"min_path_ms", "TrickDrawMinMs", 60, 1},
    {"hold_end_ms", "TrickHoldEndMs", 50, 1},
    {"low_pop_ms", "LowPopMsPerUnit", 80, 1},
    {"combo_wait_ms", "ComboWaitMs", 130, 1},
    {"recenter_ms", "RecenterMs", 34, 1},
    {"scoop_mirror", "SwapShoveIts", 0, 1},
    {"shuv_degrees", "ShuvScoopDegrees", 50, 1},
    {"shuv360_degrees", "Shuv360ScoopDegrees", 150, 1},
    {"windup_deg", "WindUpDegrees", 60, 1},
    {"quick_pops", "QuickPops", 1, 1},
    {"pop_on_release", "PopOnRelease", 0, 1},
    {"landing_manuals", "LandingManuals", 2, 1},
    {"manual_max", "ManualMaxPercent", 85, 100},
    {"stock_grab_air", "StockGrabAir", 0, 1},
    {"stock_transition_ground", "StockTransitionGround", 0, 1},
    {"air_controls", "AirControls", 1, 1},
    {"fine_grinds", "FineGrindControls", 0, 1},
    {"split_grinds", "SplitGrindControls", 1, 1},
    {"grind_sensitivity", "GrindSensitivityPercent", 120, 100},
    {"native_grind_entry", "NativeGrindEntry", 0, 1},
    {"enabled", "EnabledAtStart", 1, 1},
    {"feet_swapped", "PresetFeetSwapped", 0, 1},
}
function M.read(doc)
    local c = engine.new_config()
    for _, field in fields do
        local value = ini.number(doc,"SessionControls",field[2],field[3])
        if value ~= value or math.abs(value) == math.huge then value = field[3] end
        c[field[1]] = value / field[4]
    end
    c.pure, c.direct, c.enabled = 1, 0, 1
    -- Native asset-range experiments have no sandbox API in this port.
    c.manual_catch_ranges = 0
    c.manual_max = math.clamp(c.manual_max, 0.21, 0.94)
    c.deadzone = math.clamp(c.deadzone, 0, 0.9)
    c.active = math.clamp(c.active, 0.1, 0.9)
    c.steer_curve = math.clamp(c.steer_curve, 1, 3)
    c.stick_steer_angle = math.clamp(c.stick_steer_angle, 5, 60)
    c.grind_sensitivity = math.clamp(c.grind_sensitivity, 0.5, 2)
    c.prewind_percent = math.clamp(c.prewind_percent, 50, 90)
    c.revert_ms = math.clamp(c.revert_ms, 100, 600)
    if c.xl_layout ~= 0 then c.front_back, c.switch_flip = 0, 3 end
    if c.front_back ~= 0 then c.switch_flip = 1 end
    return c
end
function M.fresh()
    local doc = ini.parse("[SessionControls]\n")
    for _, field in fields do ini.set(doc, "SessionControls", field[2], field[3]) end
    require("controls").preset(doc, false)
    ini.set(doc, "SessionControls", "SplitGrindControls", 1)
    ini.set(doc, "SessionControls", "Controller", "auto")
    ini.set(doc, "SessionControls", "AutoStance", 1)
    ini.set(doc, "SessionControls", "Stance", "regular")
    ini.set(doc, "SessionControls", "EnabledAtStart", 1)
    ini.set(doc, "SessionControls", "DpadCameraSide", 1)
    ini.set(doc, "SessionControls", "FakieReturn", 1)
    return doc
end
return M
