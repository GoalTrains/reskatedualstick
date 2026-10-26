-- Ported from Dual Stick v6.6 pure control state. Regenerate with tools/port_translate.py.
local M = {}
local NAN, INFINITY = 0/0, math.huge
local function finite(v) return if v == v and math.abs(v) < math.huge then 1 else 0 end
local SC_LTHUMB, SC_RTHUMB, SC_LSHOULDER, SC_RSHOULDER, SC_DPAD_UP = 64, 128, 256, 512, 1
local function truth(v) return v ~= nil and v ~= false and v ~= 0 end
local function bool(v) return if v then 1 else 0 end
local function trunc(v) return if v < 0 then math.ceil(v) else math.floor(v) end
local float_buffer = buffer.create(4)
local function f32(v) buffer.writef32(float_buffer, 0, v); return buffer.readf32(float_buffer, 0) end
local function copy(v) if type(v) ~= "table" then return v end; local t = {}; for k,x in v do t[k] = copy(x) end; return t end
local function replace(target, source) table.clear(target); for k,v in source do target[k] = copy(v) end end
local function new_sc_pad() return {buttons=0, lt=0, rt=0, lx=0, ly=0, rx=0, ry=0} end
local function new_sc_config() return {enabled=0, goofy=0, feet_swapped=0, flip_x=0, grab_mode=0, grab_threshold=0, deadzone=0, steer_scale=0, steer_curve=0, active=0, point_ms=0, pop_release_ollie=0, shuv_degrees=0, shuv360_degrees=0, lean_ms=0, walk_ms=0, ms_per_unit=0, min_path_ms=0, hold_end_ms=0, direct=0, low_pop_ms=0, combo_wait_ms=0, recenter_ms=0, scoop_mirror=0, windup_deg=0, quick_pops=0, air_controls=0, fine_grinds=0, split_grinds=0, grind_sensitivity=0, grab_double_ms=0, powerslide=0, ps_delay_ms=0, revert_ms=0, ps_flip=0, revert_flip=0, revert_sweep_ms=0, revert_gap_ms=0, revert_min_gap_ms=0, revert_shape=0, bump_ms=0, prewind=0, prewind_steer=0, prewind_percent=0, prewind_ms=0, grab_hold_ms=0, switch_flip=0, switch_invert_x=0, switch_invert_y=0, fakie_return=0, disable_hold_fakie=0, xl_layout=0, pop_on_release=0, landing_manuals=0, manual_max=0, pure=0, revert_input=0, switch_goofy_flip=0, bf_grab_ms=0, one_foot_flips=0, front_back=0, stick_steer=0, stick_steer_angle=0, manual_catch=0, manual_catch_replay=0, manual_catch_ranges=0, stock_grab_air=0, stock_transition_ground=0, native_grind_entry=0, body_flips=0} end
local function new_sc_state() return {phase=0, pop_back=0, sweep=0, last_angle=0, tracking=0, side=0, trick=0, emit_start=0, emit_index=0, last_name=nil, fired=0, lean_since=0, walk_since=0, walk_request=0, pop_start=0, pop_angle=0, pop_mag=0, max_mag=0, event=0, ev_angle=0, ev_mag=0, ev_foot_back=0, ev_logged=0, ox=0, oy=0, from_x=0, from_y=0, seg=0, hold_start=0, emit_ms_per_unit=0, emit_last=0, game_known=0, emit_wait=0, emit_wait_t=0, emit_crouch_y=0, recenter_until=0, path=nil, path_n=0, d_front_on=0, d_back_on=0, d_front_spent=0, d_back_spent=0, d_front_seq=0, d_back_seq=0, d_seq=0, d_front_flick=0, d_back_flick=0, d_two=0, d_t=0, d_front_ignored=0, d_back_ignored=0, d_pop=0, d_sweep=0, d_last_ang=0, d_side=0, d_track=0, d_pending=0, d_pend_since=0, d_pend_peak=0, d_latch=0, d_lx=0, d_ly=0, d_windup_t=0, d_back_full=0, d_front_full=0, rp_two_foot=0, rp_t=0, rp_y=0, d_back_peak=0, d_front_peak=0, d_back_was_down=0, d_front_was_up=0, d_back_rel_t=0, d_front_rel_t=0, d_windup=0, ps_side=0, ps_tap_side=0, rv_side=0, ps_logged=0, ps_pop_t=0, ps_start=0, ps_tap_t=0, rv_start=0, tr_down={[0]=0, [1]=0}, rv_block=0, rv_hold=0, rv_both=0, tr_press={[0]=0, [1]=0}, tr_rel={[0]=0, [1]=0}, g_down={[0]=0, [1]=0}, g_grab={[0]=0, [1]=0}, g_press={[0]=0, [1]=0}, g_rel={[0]=0, [1]=0}, g_pulse={[0]=0, [1]=0}, switched=0, xl_absolute_raw=0, game_mirror_known=0, game_mirror=0, fine_exit_stage=0, fine_front_started=0, fine_back_started=0, board_frame_valid=0, board_frame_left=0, board_frame_mirror=0, game_switch_known=0, game_switch=0, feet_map_ok=0, feet_map_left=0, sw_lat=0, sw_lat_ok=0, sw_prev=0, sw_blend_until=0, sw_t=0, sw_x=0, sw_y=0, pw_side=0, pw_logged=0, pw_nollie=0, pw_block=0, pw_was_air=0, pw_t=0, pw_crouch_t=0, bump_side=0, bump_y=0, bump_prev=0, bump_until=0, bump_neutral_until=0, bump_output_time=0, bump_output_x=0, bump_output_y=0, stock_air_active=0, stock_air_block_l=0, stock_air_block_r=0, stock_air_langle=0, stock_air_rangle=0, pump_r3=0, pump_press_t=0, pump_release_t=0, pump_level=0, pump_release_level=0, pf_t={[0]=0, [1]=0}, pf_hold=0, lp_blk={[0]=0, [1]=0}, lp_dip={[0]=0, [1]=0}, lp_ang={[0]=0, [1]=0}, lp_mag={[0]=0, [1]=0}, bf_arm=0, bf_side=0, bf_was_air=0, bf_arm_t=0, bf_t=0, bf_air_t=0, game_crouch=0, stock_ramp_valid=0, stock_ramp_eligible=0, stock_ramp_was_air=0, stock_ramp_departing=0, stock_ramp_vertical=0, stock_ramp_since=0, stock_ramp_last_ground=0, stock_ramp_evidence=0, stock_ramp_history_t={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0}, stock_ramp_history_y={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0}, stock_ramp_history_n=0, stock_ramp_history_next=0, stock_ramp_curve=0, stock_early_active=0, air=0, d_was_air=0, d_front_carry=0, d_back_carry=0, d_front_cign=0, d_back_cign=0, d_land_t=0, sw_raw=0, sw_takeoff=0, ps_lt=0, ps_rt=0, d_takeoff_t=0, takeoff=0, d_took_off=0, d_land_hold=0, grind=0, grind_prev=0, grind_pop=0, grind_fpop=0, grind_bpop=0, split_grind_active=0, split_grind_lean=0, grind_setup_override=0, entry_back_active=0, entry_side_back=0, entry_back_x=0, entry_back_y=0, entry_rail_target=0, entry_rotation_active=0, split_rail_front_load=0, split_rail_back_load=0, split_rear_turn=0, split_rear_adjusted=0, split_rear_angle=0, split_rear_target=0, split_air_setup=0, split_landing_setup=0, rail_scoop_track=0, rail_scoop_exit=0, rail_scoop_candidate=0, rail_scoop_angle=0, rail_scoop_sweep=0, rail_scoop_x=0, rail_scoop_y=0, rail_scoop_last=0, rail_scoop_turn=0, grind_fflick=0, grind_bflick=0, grind_fpop_t=0, grind_bpop_t=0, hold_foot=0, hold_x=0, hold_y=0, hold_scoop=0, tk_fa=0, tk_ba=0, lone_a0={[0]=0, [1]=0}, lone_last={[0]=0, [1]=0}, lone_sw={[0]=0, [1]=0}, lone_out={[0]=0, [1]=0}, lone_unl={[0]=0, [1]=0}, lone_seen={[0]=0, [1]=0}, lone_from={[0]=0, [1]=0}, lone_side={[0]=0, [1]=0}, split_pop_t=0, dp_t=0, radial_passthrough=0, st_latch=0, d_log=0, d_log_a=0, d_log_b=0} end
local function new_sc_manual_catch() return {phase=0, seen=0, front_left=0, requested=0, was_air=0, candidate=0, recentred=0, candidate_kind=0, scoop_track=0, hold_window_id=0, observed_seen=0, observed_id=0, observed_doing=0, emitted_seen=0, id_seen=0, id_previous=0, held_recentred=0, previous=0, x=0, y=0, scoop_last=0, scoop_sweep=0, emitted_x=0, emitted_y=0, candidate_at=0, release_until=0, started=0, scoop_time=0, emitted_at=0, replay_x={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0, [25]=0, [26]=0, [27]=0, [28]=0, [29]=0, [30]=0, [31]=0}, replay_y={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0, [25]=0, [26]=0, [27]=0, [28]=0, [29]=0, [30]=0, [31]=0}, replay_head=0, replay_count=0, replay_running=0, replay_sample_at=0, replay_started=0} end
local function new_sc_camera_dpad() return {previous=0, blocked=0, started={[0]=0, [1]=0}, active=0, held=0, cancelled=0, synced=0, selected=0, pending=0, selected_at=0, pulse_until=0, last_time=0, next_toggle=0, owner=0} end
local function new_sc_stance_track() return {rode_before_walk=0, initialized=0, prev_state=0, prev_raw=0, reference=0, takeoff_stance=0, stance=0, turn_time=0, landing_turn_ms=0, landed_at=0, late_turn_ms=0, late_slide_ms=0, pending_revert=0, saw_air_edge=0, pending_reference=0, pending_stance=0, settle_touchdown=0, air_zero_pending=0, air_zero_since=0, direct_initialized=0, direct_stance=0, direct_candidate=0, direct_since=0, direct_neutral=0} end
local function new_nr_frame() return {selector=0, core=0, context=0, source=0, basis=0, native_frame=0, mirrored=0, position={[0]=0, [1]=0, [2]=0}, forward={[0]=0, [1]=0, [2]=0}} end
local function new_sc_xl_agreement() return {previous=new_nr_frame(), time=0, baseline_since=0, candidate_since=0, sampled=0, profile_known=0, profile=0, baseline_pending=0, baseline_valid=0, mirror=0, candidate=0} end
local function new_sc_xl_absolute() return {valid=0, stance=0, candidate=0, pending=0, since=0} end
local function new_sc_motion_stance() return {time=0, x=0, z=0, candidate_since=0, sampled=0, candidate=0, candidate_valid=0, bias=0, bias_valid=0, seen_riding=0, remount_pending=0, remount_started=0} end
local function new_sc_entry_plan() return {active=0, type=0, accepted_only=0, yaw=0, pitch=0} end
local SC_TRICK_COUNT = 64
local sc_ovr_xy = {}
local sc_ovr_n = {}
local sc_ovr_ms = {}
local sc_tricks = {}; for i = 0, 63 do sc_tricks[i] = {name="", count=0, xy={}} end
local function sc_is_grind_state(state)
    do
        return trunc((if truth((if truth(bool(state >= 400)) then bool(truth(bool(state < 500))) else 0)) then 1 else bool(truth(bool(state == 701)))))
    end
end
local function sc_remap_foot_history(s)
    do
        s.split_pop_t = 0
        s.split_rear_turn = trunc(0)
        s.rail_scoop_candidate = trunc(0)
        s.rail_scoop_exit = trunc(s.rail_scoop_candidate)
        s.rail_scoop_track = trunc(s.rail_scoop_exit)
        s.rail_scoop_turn = 0
        s.rail_scoop_last = s.rail_scoop_turn
        repeat
            do
                local v = trunc(s.d_front_on)
                s.d_front_on = trunc(s.d_back_on)
                s.d_back_on = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_spent)
                s.d_front_spent = trunc(s.d_back_spent)
                s.d_back_spent = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_flick)
                s.d_front_flick = trunc(s.d_back_flick)
                s.d_back_flick = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_seq)
                s.d_front_seq = trunc(s.d_back_seq)
                s.d_back_seq = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_ignored)
                s.d_front_ignored = trunc(s.d_back_ignored)
                s.d_back_ignored = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_carry)
                s.d_front_carry = trunc(s.d_back_carry)
                s.d_back_carry = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.d_front_cign)
                s.d_front_cign = trunc(s.d_back_cign)
                s.d_back_cign = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.tk_fa)
                s.tk_fa = f32(s.tk_ba)
                s.tk_ba = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = s.pf_t[0]
                s.pf_t[0] = s.pf_t[1]
                s.pf_t[1] = v
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lp_blk[0])
                s.lp_blk[0] = trunc(s.lp_blk[1])
                s.lp_blk[1] = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lp_dip[0])
                s.lp_dip[0] = trunc(s.lp_dip[1])
                s.lp_dip[1] = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lp_ang[0])
                s.lp_ang[0] = f32(s.lp_ang[1])
                s.lp_ang[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lp_mag[0])
                s.lp_mag[0] = f32(s.lp_mag[1])
                s.lp_mag[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lone_a0[0])
                s.lone_a0[0] = f32(s.lone_a0[1])
                s.lone_a0[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lone_last[0])
                s.lone_last[0] = f32(s.lone_last[1])
                s.lone_last[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lone_sw[0])
                s.lone_sw[0] = f32(s.lone_sw[1])
                s.lone_sw[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = f32(s.lone_out[0])
                s.lone_out[0] = f32(s.lone_out[1])
                s.lone_out[1] = f32(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lone_unl[0])
                s.lone_unl[0] = trunc(s.lone_unl[1])
                s.lone_unl[1] = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lone_seen[0])
                s.lone_seen[0] = trunc(s.lone_seen[1])
                s.lone_seen[1] = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lone_from[0])
                s.lone_from[0] = trunc(s.lone_from[1])
                s.lone_from[1] = trunc(v)
            end
        until not truth(0)
        repeat
            do
                local v = trunc(s.lone_side[0])
                s.lone_side[0] = trunc(s.lone_side[1])
                s.lone_side[1] = trunc(v)
            end
        until not truth(0)
        if truth(s.hold_foot) then
            s.hold_foot = trunc((3 - s.hold_foot))
        end
        if truth(s.d_pop) then
            s.d_pop = trunc((3 - s.d_pop))
        end
        s.pf_hold = trunc((-s.pf_hold))
        s.d_back_full = trunc(0)
        s.d_front_full = trunc(s.d_back_full)
        s.d_back_peak = f32(0)
        s.d_front_peak = f32(s.d_back_peak)
        s.d_back_was_down = trunc(0)
        s.d_front_was_up = trunc(s.d_back_was_down)
    end
end
local function sc_abs(v)
    do
        return f32((if truth(bool(v < 0)) then (-v) else v))
    end
end
local function sc_sqrt(v)
    do
        return f32((if truth(bool(v <= 0)) then 0 else math.sqrt(v)))
    end
end
local function sc_sin_deg(d)
    do
        while truth(bool(d > 180)) do
            d = f32((d - 360))
        end
        while truth(bool(d < (-180))) do
            d = f32((d + 360))
        end
        local x = f32((d * 0.0174532925))
        local x2 = f32((x * x))
        return f32((x * (1 - ((x2 / 6) * (1 - ((x2 / 20) * (1 - ((x2 / 42) * (1 - ((x2 / 72) * (1 - (x2 / 110))))))))))))
    end
end
local function sc_axis(v)
    do
        return f32((if truth(bool(v < 0)) then (f32(v) / 32768.0) else (f32(v) / 32767.0)))
    end
end
local function sc_stick_steer_angle(c)
    do
        return trunc((if truth((if truth(bool(c.stick_steer_angle >= 5)) then bool(truth(bool(c.stick_steer_angle <= 60))) else 0)) then c.stick_steer_angle else 34))
    end
end
local function sc_next_stick_steer_angle(value, dir)
    do
        local angles = {[0]=trunc(5), [1]=trunc(10), [2]=trunc(15), [3]=trunc(20), [4]=trunc(25), [5]=trunc(30), [6]=trunc(34), [7]=trunc(40), [8]=trunc(45), [9]=trunc(50), [10]=trunc(55), [11]=trunc(60)}
        if truth(bool(dir < 0)) then
            do
                do
                    local i = trunc(11)
                    while truth(bool(i >= 0)) do
                        if truth(bool(angles[i] < value)) then
                            return trunc(angles[i])
                        end
                        i = trunc((i - 1))
                    end
                end
                return trunc(60)
            end
        end
        do
            local i = trunc(0)
            while truth(bool(i < 12)) do
                if truth(bool(angles[i] > value)) then
                    return trunc(angles[i])
                end
                i = trunc((i + 1))
            end
        end
        return trunc(5)
    end
end
local function sc_short(v)
    do
        if truth(bool(v > 1.0)) then
            v = f32(1.0)
        end
        if truth(bool(v < (-1.0))) then
            v = f32((-1.0))
        end
        return trunc(trunc((if truth(bool(v < 0)) then (v * 32768.0) else (v * 32767.0))))
    end
end
local function sc_pow(b, e)
    do
        if truth(bool(b <= 0)) then
            return f32(0)
        end
        if truth(bool(e <= 1.0)) then
            return f32(b)
        end
        local r = f32(b)
        local whole = f32(f32(trunc(e)))
        do
            local i = trunc(1)
            while truth(bool(i < trunc(whole))) do
                r = f32((r * b))
                i = trunc((i + 1))
            end
        end
        local frac = f32((e - whole))
        if truth(bool(frac > 0)) then
            r = f32((r * ((1.0 - frac) + (frac * b))))
        end
        return f32(r)
    end
end
local function sc_angle(x, y)
    do
        local dy = f32((-y))
        local ax = f32(sc_abs(x))
        local ay = f32(sc_abs(dy))
        local mx = f32((if truth(bool(ax > ay)) then ax else ay))
        local mn = f32((if truth(bool(ax > ay)) then ay else ax))
        if truth(bool(mx <= 0)) then
            return f32(0)
        end
        local a = f32((mn / mx))
        local s = f32((a * a))
        local r = f32(((((((((-0.0464964749) * s) + 0.15931422) * s) - 0.327622764) * s) * a) + a))
        if truth(bool(ax > ay)) then
            r = f32((1.57079637 - r))
        end
        if truth(bool(dy < 0)) then
            r = f32((3.14159274 - r))
        end
        if truth(bool(x < 0)) then
            r = f32((6.28318548 - r))
        end
        return f32((r * 57.2957795))
    end
end
local function sc_wrap(d)
    do
        while truth(bool(d > 180)) do
            d = f32((d - 360))
        end
        while truth(bool(d < (-180))) do
            d = f32((d + 360))
        end
        return f32(d)
    end
end
local function sc_streq(a, b)
    return bool(a == b)
end
local function sc_find(name)
    for i = 0, 63 do if sc_tricks[i].name == name then return i end end
    return -1
end
local function sc_deadzone(x, y, dz)
    do
        local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
        if truth((if truth(bool(m <= dz)) then 1 else bool(truth(bool(m <= 0))))) then
            do
                y.set(f32(0))
                x.set(f32(y.get()))
                return
            end
        end
        local scaled = f32(((m - dz) / (1.0 - dz)))
        if truth(bool(scaled > 1.0)) then
            scaled = f32(1.0)
        end
        x.set(f32(((x.get() / m) * scaled)))
        y.set(f32(((y.get() / m) * scaled)))
    end
end
local function sc_trick_name(pop_back, scoop, scoop_side, flip)
    do
        local ollie = {[0]={[0]={[0]="ollie", [1]="ollie"}, [1]={[0]="fspopshuvit", [1]="popshuvit"}, [2]={[0]="fs360popshuvit", [1]="360popshuvit"}}, [1]={[0]={[0]="kickflip", [1]="kickflip"}, [1]={[0]="hardflip", [1]="varialkickflip"}, [2]={[0]="360hardflip", [1]="360flip"}}, [2]={[0]={[0]="heelflip", [1]="heelflip"}, [1]={[0]="varialheelflip", [1]="inwardheelflip"}, [2]={[0]="laserflip", [1]="360inwardheelflip"}}}
        local nollie = {[0]={[0]={[0]="nollie", [1]="nollie"}, [1]={[0]="n_fspopshuvit", [1]="n_popshuvit"}, [2]={[0]="n_fs360popshuvit", [1]="n_360popshuvit"}}, [1]={[0]={[0]="n_kickflip", [1]="n_kickflip"}, [1]={[0]="n_hardflip", [1]="n_varialkickflip"}, [2]={[0]="n_360hardflip", [1]="n_360flip"}}, [2]={[0]={[0]="n_heelflip", [1]="n_heelflip"}, [1]={[0]="n_varialheelflip", [1]="n_inwardheelflip"}, [2]={[0]="n_laserflip", [1]="n_360inwardheelflip"}}}
        local f = trunc((if truth(bool(flip > 0)) then 1 else (if truth(bool(flip < 0)) then 2 else 0)))
        local side = trunc((if truth(bool(scoop_side > 0)) then 1 else 0))
        return (if truth(pop_back) then ollie[f][scoop][side] else nollie[f][scoop][side])
    end
end
local function sc_fire(s, name, now)
    do
        s.event = trunc(2)
        s.d_latch = trunc(0)
        local i = trunc(sc_find(name))
        if truth(bool(i < 0)) then
            do
                s.phase = trunc(3)
                return
            end
        end
        s.trick = trunc(i)
        s.emit_start = now
        s.emit_index = trunc(0)
        s.from_x = f32(s.ox)
        s.from_y = f32(s.oy)
        s.seg = trunc(1)
        s.hold_start = 0
        s.emit_last = f32(0)
        s.emit_wait = trunc(1)
        s.emit_wait_t = now
        s.emit_crouch_y = f32((if truth(sc_tricks[i].nollie) then 0.95 else (-0.95)))
        s.emit_ms_per_unit = f32(0)
        s.recenter_until = 0
        s.path_n = trunc(0)
        s.phase = trunc(2)
        s.last_name = name
        s.fired = trunc((s.fired + 1))
    end
end
local function sc_emit_step(c, s, now, ox, oy)
    do
        if truth(s.emit_wait) then
            do
                local ready = trunc((if truth((if truth((if truth((if truth((if truth(bool(not truth(s.game_known))) then 1 else bool(truth(bool(s.path_n > 0))))) then 1 else bool(truth(bool(s.game_crouch > 0))))) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(s.air)))) then 1 else bool(truth(bool(s.takeoff == 1)))))
                if truth((if truth(ready) then 1 else bool(truth(bool((now - s.emit_wait_t) >= 100.0))))) then
                    do
                        s.emit_wait = trunc(0)
                        if truth(bool(now > s.emit_wait_t)) then
                            do
                                if truth(bool(s.recenter_until > 0)) then
                                    s.recenter_until = (now + (s.recenter_until - s.emit_wait_t))
                                else
                                    do
                                        s.from_x = f32(0)
                                        s.from_y = f32(s.emit_crouch_y)
                                    end
                                end
                            end
                        end
                        s.emit_start = now
                    end
                else
                    do
                        ox.set(f32(0))
                        oy.set(f32(s.emit_crouch_y))
                        s.emit_start = now
                        return
                    end
                end
            end
        end
        if truth(bool(s.recenter_until > now)) then
            do
                ox.set(f32(0))
                oy.set(f32(0))
                s.emit_start = now
                return
            end
        end
        local rec = trunc((if truth(bool(s.path_n <= 0)) then bool(truth(bool(sc_ovr_n[s.trick] > 0))) else 0))
        local pts = (if truth(bool(s.path_n > 0)) then s.path else (if truth(rec) then sc_ovr_xy[s.trick] else sc_tricks[s.trick].xy))
        local n = trunc((if truth(bool(s.path_n > 0)) then s.path_n else (if truth(rec) then sc_ovr_n[s.trick] else sc_tricks[s.trick].count)))
        local vx = {[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0}
        local vy = {[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0}
        local cum = {[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0}
        vx[0] = f32(s.from_x)
        vy[0] = f32(s.from_y)
        cum[0] = f32(0)
        do
            local k = trunc(0)
            while truth(bool(k < n)) do
                do
                    vx[(k + 1)] = f32(pts[k][0])
                    vy[(k + 1)] = f32(pts[k][1])
                    local dx = f32((vx[(k + 1)] - vx[k]))
                    local dy = f32((vy[(k + 1)] - vy[k]))
                    cum[(k + 1)] = f32((cum[k] + sc_sqrt(((dx * dx) + (dy * dy)))))
                end
                k = trunc((k + 1))
            end
        end
        local total = f32(cum[n])
        local dur = (total * (if truth(bool(s.emit_ms_per_unit > 0)) then s.emit_ms_per_unit else c.ms_per_unit))
        if truth((if truth(rec) then bool(truth(bool(sc_ovr_ms[s.trick] > 0))) else 0)) then
            do
                dur = sc_ovr_ms[s.trick]
                if truth((if truth(bool(s.emit_ms_per_unit > c.ms_per_unit)) then bool(truth(bool(c.ms_per_unit > 0))) else 0)) then
                    dur = (dur * (s.emit_ms_per_unit / c.ms_per_unit))
                end
            end
        end
        if truth(bool(dur < c.min_path_ms)) then
            dur = c.min_path_ms
        end
        local want = f32((if truth(bool(dur > 0)) then (f32(((now - s.emit_start) / dur)) * total) else total))
        if truth(bool(want > total)) then
            want = f32(total)
        end
        if truth(bool(want > (s.emit_last + 0.5))) then
            want = f32((s.emit_last + 0.5))
        end
        if truth(bool(s.seg <= n)) then
            do
                local limit = f32(cum[s.seg])
                local p = f32((if truth(bool(want < limit)) then want else limit))
                local k = trunc(0)
                while truth((if truth(bool(k < n)) then bool(truth(bool(cum[(k + 1)] < p))) else 0)) do
                    k = trunc((k + 1))
                end
                local segl = f32((cum[(k + 1)] - cum[k]))
                local u = f32((if truth(bool(segl > 0)) then ((p - cum[k]) / segl) else 1))
                ox.set(f32((vx[k] + ((vx[(k + 1)] - vx[k]) * u))))
                oy.set(f32((vy[k] + ((vy[(k + 1)] - vy[k]) * u))))
                s.emit_last = f32(p)
                if truth(bool(p >= (limit - 1e-4))) then
                    s.seg = trunc((s.seg + 1))
                end
            end
        else
            do
                ox.set(f32(vx[n]))
                oy.set(f32(vy[n]))
                if truth(bool(s.hold_start <= 0)) then
                    s.hold_start = (now + 1)
                end
                if truth(bool(((now + 1) - s.hold_start) >= c.hold_end_ms)) then
                    do
                        s.phase = trunc(3)
                    end
                end
            end
        end
    end
end
local function sc_feet_to_flick(c, s, now, fx, fy, bx, by, out_x, out_y)
    do
        sc_deadzone({get=function() return fx end, set=function(value) fx = f32(value) end}, {get=function() return fy end, set=function(value) fy = f32(value) end}, c.deadzone)
        sc_deadzone({get=function() return bx end, set=function(value) bx = f32(value) end}, {get=function() return by end, set=function(value) by = f32(value) end}, c.deadzone)
        if truth(bool(bool(c.flip_x ~= c.goofy) ~= (if truth(bool(c.switch_flip == 3)) then bool(truth(s.switched)) else 0))) then
            do
                fx = f32((-fx))
                bx = f32((-bx))
            end
        end
        local fm = f32(sc_sqrt(((fx * fx) + (fy * fy))))
        local bm = f32(sc_sqrt(((bx * bx) + (by * by))))
        local on = f32(c.active)
        local off = f32((c.active * 0.6))
        local x = f32(0)
        local y = f32(0)
        if truth(bool(s.phase == 0)) then
            do
                local ba = f32(sc_angle(bx, by))
                local fa = f32(sc_angle(fx, fy))
                local back_pop = trunc((if truth(bool(bm >= on)) then bool(truth((if truth(bool(ba <= 60)) then 1 else bool(truth(bool(ba >= 300)))))) else 0))
                local front_pop = trunc((if truth(bool(fm >= on)) then bool(truth((if truth(bool(fa <= 60)) then 1 else bool(truth(bool(fa >= 300)))))) else 0))
                if truth((if truth(back_pop) then 1 else bool(truth(front_pop)))) then
                    do
                        s.phase = trunc(1)
                        s.pop_back = trunc(back_pop)
                        s.sweep = f32(0)
                        s.last_angle = f32((if truth(back_pop) then ba else fa))
                        s.tracking = trunc(0)
                        s.side = f32(0)
                        s.pop_start = now
                        s.pop_angle = f32((if truth(back_pop) then ba else fa))
                        s.pop_mag = f32((if truth(back_pop) then bm else fm))
                        s.max_mag = f32(s.pop_mag)
                        s.event = trunc(1)
                    end
                else
                    if truth((if truth((if truth(bool(bm < on)) then bool(truth(bool(fm < on))) else 0)) then bool(truth((if truth(bool(bm > 0)) then 1 else bool(truth(bool(fm > 0)))))) else 0)) then
                        do
                            if truth(bool(s.lean_since <= 0)) then
                                s.lean_since = (now + 1)
                            end
                            if truth(bool(((now + 1) - s.lean_since) >= c.lean_ms)) then
                                do
                                    if truth(bool(bm >= fm)) then
                                        do
                                            x = f32(bx)
                                            y = f32((-sc_abs(by)))
                                        end
                                    else
                                        do
                                            x = f32(fx)
                                            y = f32(sc_abs(fy))
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
                if truth((if truth((if truth(bool(s.phase ~= 0)) then 1 else bool(truth((if truth(bool(bm >= on)) then 1 else bool(truth(bool(fm >= on)))))))) then 1 else bool(truth((if truth(bool(bm <= 0)) then bool(truth(bool(fm <= 0))) else 0))))) then
                    s.lean_since = 0
                end
                if truth((if truth((if truth(bool(s.phase == 0)) then bool(truth((if truth(bool(bm >= on)) then 1 else bool(truth(bool(fm >= on)))))) else 0)) then bool(truth(bool(not truth(s.ev_logged)))) else 0)) then
                    do
                        s.event = trunc(3)
                        s.ev_foot_back = trunc(bool(bm >= fm))
                        s.ev_angle = f32((if truth(bool(bm >= fm)) then ba else fa))
                        s.ev_mag = f32((if truth(bool(bm >= fm)) then bm else fm))
                        s.ev_logged = trunc(1)
                    end
                end
                if truth((if truth(bool(bm < off)) then bool(truth(bool(fm < off))) else 0)) then
                    s.ev_logged = trunc(0)
                end
            end
        end
        if truth(bool(s.phase == 1)) then
            do
                local px = f32((if truth(s.pop_back) then bx else fx))
                local py = f32((if truth(s.pop_back) then by else fy))
                local pm = f32((if truth(s.pop_back) then bm else fm))
                local ox = f32((if truth(s.pop_back) then fx else bx))
                local oy = f32((if truth(s.pop_back) then fy else by))
                local om = f32((if truth(s.pop_back) then fm else bm))
                if truth(bool(pm >= 0.7)) then
                    do
                        local a = f32(sc_angle(px, py))
                        if truth(s.tracking) then
                            do
                                local d = f32(sc_wrap((a - s.last_angle)))
                                if truth(bool(sc_abs(d) <= 75.0)) then
                                    s.sweep = f32((s.sweep + d))
                                end
                            end
                        end
                        s.tracking = trunc(1)
                        s.last_angle = f32(a)
                        if truth(bool(sc_abs(px) > sc_abs(s.side))) then
                            s.side = f32(px)
                        end
                    end
                else
                    do
                        s.tracking = trunc(0)
                    end
                end
                if truth(bool(pm > s.max_mag)) then
                    s.max_mag = f32(pm)
                end
                local swept = f32(sc_abs(s.sweep))
                local scoop = trunc((if truth(bool(swept >= c.shuv360_degrees)) then 2 else (if truth(bool(swept >= c.shuv_degrees)) then 1 else 0)))
                local scoop_side = trunc((if truth(bool(s.sweep > 0)) then 1 else (-1)))
                if truth((if truth(bool(scoop == 0)) then bool(truth(bool(sc_abs(s.side) >= 0.75))) else 0)) then
                    do
                        scoop = trunc(1)
                        scoop_side = trunc((if truth(bool(s.side > 0)) then 1 else (-1)))
                    end
                end
                if truth(bool(swept >= 290.0)) then
                    do
                        if truth(s.pop_back) then
                            sc_fire(s, (if truth(bool(s.sweep > 0)) then "impossible" else "frontfoot_impossible"), now)
                        else
                            sc_fire(s, (if truth(bool(s.sweep > 0)) then "n_impossible" else "n_backfoot_impossible"), now)
                        end
                    end
                else
                    if truth(bool(om >= on)) then
                        do
                            local nx = f32((ox / om))
                            local flip = trunc((if truth(bool(nx > 0.45)) then 1 else (if truth(bool(nx < (-0.45))) then (-1) else 0)))
                            sc_fire(s, sc_trick_name(s.pop_back, scoop, scoop_side, flip), now)
                        end
                    else
                        if truth(bool(pm < off)) then
                            do
                                if truth(scoop) then
                                    sc_fire(s, sc_trick_name(s.pop_back, scoop, scoop_side, 0), now)
                                else
                                    if truth(c.pop_release_ollie) then
                                        sc_fire(s, (if truth(s.pop_back) then "ollie" else "nollie"), now)
                                    else
                                        s.phase = trunc(0)
                                    end
                                end
                            end
                        else
                            do
                                local m = f32((if truth(bool(pm > 0.95)) then 0.95 else pm))
                                x = f32(0)
                                y = f32((if truth(s.pop_back) then (-m) else m))
                            end
                        end
                    end
                end
            end
        end
        if truth(bool(s.phase == 2)) then
            sc_emit_step(c, s, now, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
        end
        if truth(bool(s.phase == 3)) then
            do
                y = f32(0)
                x = f32(y)
                if truth((if truth(bool(bm < off)) then bool(truth(bool(fm < off))) else 0)) then
                    s.phase = trunc(0)
                end
            end
        end
        local m = f32(sc_sqrt(((x * x) + (y * y))))
        if truth(bool(m > 1.0)) then
            do
                x = f32((x / m))
                y = f32((y / m))
            end
        end
        out_x.set(f32(x))
        out_y.set(f32(y))
    end
end
local function sc_maybe_recenter(c, s, now)
    do
        local t = sc_tricks[s.trick]
        if truth(bool(sc_ovr_n[s.trick] > 0)) then
            return
        end
        if truth((if truth(bool(c.recenter_ms <= 0)) then 1 else bool(truth(bool(t.count < 1))))) then
            return
        end
        local fx = f32(s.from_x)
        local fy = f32(s.from_y)
        local fm = f32(sc_sqrt(((fx * fx) + (fy * fy))))
        local px = f32(t.xy[0][0])
        local py = f32(t.xy[0][1])
        local pm = f32(sc_sqrt(((px * px) + (py * py))))
        if truth((if truth(bool(fm < 0.2)) then 1 else bool(truth(bool(pm < 0.2))))) then
            return
        end
        local cosang = f32((((fx * px) + (fy * py)) / (fm * pm)))
        if truth(bool(cosang > 0.5)) then
            return
        end
        s.recenter_until = (now + c.recenter_ms)
        s.from_x = f32(0)
        s.from_y = f32(0)
    end
end
local function sc_direct(c, s, now, fx, fy, bx, by, out_x, out_y)
    do
        sc_deadzone({get=function() return fx end, set=function(value) fx = f32(value) end}, {get=function() return fy end, set=function(value) fy = f32(value) end}, c.deadzone)
        sc_deadzone({get=function() return bx end, set=function(value) bx = f32(value) end}, {get=function() return by end, set=function(value) by = f32(value) end}, c.deadzone)
        local mirror = trunc(c.flip_x)
        if truth((if truth(c.goofy) then bool(truth(bool(not truth((if truth((if truth(c.air_controls) then bool(truth(s.air)) else 0)) then bool(truth(bool(s.phase ~= 2))) else 0))))) else 0)) then
            mirror = trunc(bool(not truth(mirror)))
        end
        if truth((if truth((if truth(bool(c.switch_flip == 3)) then bool(truth(s.switched)) else 0)) then bool(truth(bool(not truth((if truth((if truth(c.air_controls) then bool(truth(s.air)) else 0)) then bool(truth(bool(s.phase ~= 2))) else 0))))) else 0)) then
            mirror = trunc(bool(not truth(mirror)))
        end
        if truth(mirror) then
            do
                fx = f32((-fx))
                bx = f32((-bx))
            end
        end
        local ffx = f32(fx)
        local ffy = f32(sc_abs(fy))
        local bbx = f32(bx)
        local bby = f32((-sc_abs(by)))
        local fm = f32(sc_sqrt(((ffx * ffx) + (ffy * ffy))))
        local bm = f32(sc_sqrt(((bbx * bbx) + (bby * bby))))
        local on = f32(c.active)
        local off = f32((c.active * 0.6))
        if truth((if truth(s.d_front_carry) then bool(truth(bool(fm < off))) else 0)) then
            s.d_front_carry = trunc(0)
        end
        if truth((if truth(s.d_back_carry) then bool(truth(bool(bm < off))) else 0)) then
            s.d_back_carry = trunc(0)
        end
        if truth((if truth(bool(s.takeoff > 0)) then bool(truth(bool(not truth(s.d_took_off)))) else 0)) then
            do
                s.d_took_off = trunc(1)
                s.d_takeoff_t = now
                s.sw_takeoff = trunc(s.sw_raw)
                s.d_front_carry = trunc(bool(fm >= c.deadzone))
                s.d_back_carry = trunc(bool(bm >= c.deadzone))
            end
        else
            if truth((if truth(bool(s.takeoff > 0)) then bool(truth(bool((now - s.d_takeoff_t) < 150.0))) else 0)) then
                do
                    if truth(bool(fm >= c.active)) then
                        s.d_front_carry = trunc(1)
                    end
                    if truth(bool(bm >= c.active)) then
                        s.d_back_carry = trunc(1)
                    end
                end
            else
                if truth((if truth(bool(s.takeoff == 0)) then bool(truth(s.d_took_off)) else 0)) then
                    do
                        s.d_took_off = trunc(0)
                        if truth((if truth((if truth(bool(not truth(s.air))) then bool(truth(bool(not truth(s.d_was_air)))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0)) then
                            do
                                if truth((if truth(s.d_front_carry) then 1 else bool(truth((if truth(bool(fm >= 0.9)) then bool(truth(bool(fy > (0.5 * fm)))) else 0))))) then
                                    do
                                        s.d_front_ignored = trunc(1)
                                        s.d_front_on = trunc(0)
                                        s.d_front_cign = trunc(1)
                                    end
                                end
                                if truth((if truth(s.d_back_carry) then 1 else bool(truth((if truth(bool(bm >= 0.9)) then bool(truth(bool(by < ((-0.5) * bm)))) else 0))))) then
                                    do
                                        s.d_back_ignored = trunc(1)
                                        s.d_back_on = trunc(0)
                                        s.d_back_cign = trunc(1)
                                    end
                                end
                                s.d_back_carry = trunc(0)
                                s.d_front_carry = trunc(s.d_back_carry)
                            end
                        end
                    end
                end
            end
        end
        if truth((if truth((if truth(bool(not truth(c.air_controls))) then bool(truth(s.air)) else 0)) then bool(truth(bool(s.phase ~= 2))) else 0)) then
            do
                s.d_was_air = trunc(1)
                s.phase = trunc(0)
                s.d_pop = trunc(0)
                s.d_pending = trunc(0)
                s.d_latch = trunc(0)
                s.d_windup = trunc(0)
                s.path_n = trunc(0)
                s.d_back_on = trunc(0)
                s.d_front_on = trunc(s.d_back_on)
                s.d_back_spent = trunc(0)
                s.d_front_spent = trunc(s.d_back_spent)
                s.d_two = trunc(0)
                local mm = f32((if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))
                local lx2 = f32(0)
                local ly2 = f32(0)
                local allow = trunc((if truth(bool(c.landing_manuals >= 2)) then 1 else bool(truth((if truth(bool(c.landing_manuals == 1)) then bool(truth((if truth((if truth(bool(s.sw_raw < 0)) then 1 else bool(truth(bool(s.sw_takeoff < 0))))) then 1 else bool(truth(bool(s.sw_raw ~= s.sw_takeoff)))))) else 0)))))
                if truth(allow) then
                    do
                        if truth((if truth((if truth((if truth(bool(not truth(s.d_front_carry))) then bool(truth(bool(fy > 0))) else 0)) then bool(truth(bool(fm >= c.deadzone))) else 0)) then bool(truth(bool(fm < mm))) else 0)) then
                            do
                                lx2 = f32(0)
                                ly2 = f32(fy)
                            end
                        end
                        if truth((if truth((if truth((if truth((if truth(bool(not truth(s.d_back_carry))) then bool(truth(bool(by < 0))) else 0)) then bool(truth(bool(bm >= c.deadzone))) else 0)) then bool(truth(bool(bm < mm))) else 0)) then bool(truth(bool(bm > sc_abs(ly2)))) else 0)) then
                            do
                                lx2 = f32(0)
                                ly2 = f32(by)
                            end
                        end
                    end
                end
                s.d_t = now
                out_x.set(f32(lx2))
                out_y.set(f32(ly2))
                return
            end
        end
        if truth((if truth((if truth(c.air_controls) then bool(truth(s.air)) else 0)) then bool(truth(bool(s.phase ~= 2))) else 0)) then
            do
                if truth((if truth(bool(not truth(s.d_was_air))) then bool(truth(bool(s.takeoff < 0))) else 0)) then
                    do
                        s.d_front_carry = trunc((if truth(bool(fm >= 0.7)) then bool(truth(bool(fy > (0.5 * fm)))) else 0))
                        s.d_back_carry = trunc((if truth(bool(bm >= 0.7)) then bool(truth(bool(by < ((-0.5) * bm)))) else 0))
                    end
                end
                if truth((if truth(bool(fm >= 0.9)) then bool(truth(bool(fy > (0.5 * fm)))) else 0)) then
                    s.d_front_carry = trunc(1)
                end
                if truth((if truth(bool(bm >= 0.9)) then bool(truth(bool(by < ((-0.5) * bm)))) else 0)) then
                    s.d_back_carry = trunc(1)
                end
                if truth((if truth(bool(fy > (0.85 * fm))) then bool(truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then
                    ffy = f32(0)
                end
                if truth((if truth(bool(by < ((-0.85) * bm))) then bool(truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then
                    bby = f32(0)
                end
                if truth(s.d_front_carry) then
                    fm = f32(0)
                end
                if truth(s.d_back_carry) then
                    bm = f32(0)
                end
                do
                    local allow = trunc((if truth(bool(c.landing_manuals >= 2)) then 1 else bool(truth((if truth(bool(c.landing_manuals == 1)) then bool(truth((if truth((if truth(bool(s.sw_raw < 0)) then 1 else bool(truth(bool(s.sw_takeoff < 0))))) then 1 else bool(truth(bool(s.sw_raw ~= s.sw_takeoff)))))) else 0)))))
                    if truth(bool(not truth(allow))) then
                        do
                            ffy = f32(0)
                            bby = f32(0)
                        end
                    end
                end
                s.d_was_air = trunc(1)
                s.phase = trunc(0)
                s.d_pop = trunc(0)
                s.d_pending = trunc(0)
                s.d_latch = trunc(0)
                s.d_windup = trunc(0)
                s.path_n = trunc(0)
                s.d_back_on = trunc(0)
                s.d_front_on = trunc(s.d_back_on)
                s.d_back_spent = trunc(0)
                s.d_front_spent = trunc(s.d_back_spent)
                s.d_two = trunc(0)
                local x = f32(((if truth(bool(fm >= c.deadzone)) then ffx else 0) + (if truth(bool(bm >= c.deadzone)) then bbx else 0)))
                local y = f32(((if truth(bool(fm >= c.deadzone)) then ffy else 0) + (if truth(bool(bm >= c.deadzone)) then bby else 0)))
                local m = f32(sc_sqrt(((x * x) + (y * y))))
                if truth(bool(m > 1.0)) then
                    do
                        x = f32((x / m))
                        y = f32((y / m))
                    end
                end
                s.d_t = now
                out_x.set(f32(x))
                out_y.set(f32(y))
                return
            end
        end
        if truth(s.d_was_air) then
            do
                s.d_was_air = trunc(0)
                s.oy = f32(0)
                s.ox = f32(s.oy)
                s.d_t = now
                if truth(bool(not truth(s.grind))) then
                    do
                        local keep_lean = trunc((if truth((if truth(bool(c.landing_manuals >= 2)) then 1 else bool(truth((if truth((if truth((if truth(bool(c.landing_manuals == 1)) then bool(truth(bool(s.sw_raw >= 0))) else 0)) then bool(truth(bool(s.sw_takeoff >= 0))) else 0)) then bool(truth(bool(s.sw_raw ~= s.sw_takeoff))) else 0))))) then 1 else bool(truth((if truth(bool(c.landing_manuals == 1)) then bool(truth((if truth(bool(s.sw_raw < 0)) then 1 else bool(truth(bool(s.sw_takeoff < 0)))))) else 0)))))
                        local f_lean = trunc((if truth(bool(not truth(keep_lean))) then bool(truth(bool(fm >= c.deadzone))) else 0))
                        local b_lean = trunc((if truth(bool(not truth(keep_lean))) then bool(truth(bool(bm >= c.deadzone))) else 0))
                        if truth((if truth((if truth(s.d_front_carry) then 1 else bool(truth(f_lean)))) then 1 else bool(truth((if truth(bool(fm >= 0.9)) then bool(truth(bool(fy > (0.5 * fm)))) else 0))))) then
                            do
                                s.d_front_ignored = trunc(1)
                                s.d_front_on = trunc(0)
                                s.d_front_cign = trunc(1)
                            end
                        end
                        if truth((if truth((if truth(s.d_back_carry) then 1 else bool(truth(b_lean)))) then 1 else bool(truth((if truth(bool(bm >= 0.9)) then bool(truth(bool(by < ((-0.5) * bm)))) else 0))))) then
                            do
                                s.d_back_ignored = trunc(1)
                                s.d_back_on = trunc(0)
                                s.d_back_cign = trunc(1)
                            end
                        end
                        if truth((if truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then bool(truth(bool(fy > (0.85 * fm)))) else 0)) then
                            do
                                s.d_front_ignored = trunc(1)
                                s.d_front_on = trunc(0)
                                s.d_front_cign = trunc(3)
                            end
                        end
                        if truth((if truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then bool(truth(bool(by < ((-0.85) * bm)))) else 0)) then
                            do
                                s.d_back_ignored = trunc(1)
                                s.d_back_on = trunc(0)
                                s.d_back_cign = trunc(3)
                            end
                        end
                        s.d_land_t = now
                    end
                end
                s.d_back_carry = trunc(0)
                s.d_front_carry = trunc(s.d_back_carry)
            end
        end
        local fa = f32(sc_angle(fx, fy))
        local ba = f32(sc_angle(bx, by))
        local f_off = f32(sc_abs(sc_wrap((fa - 180.0))))
        local b_off = f32(sc_abs(sc_wrap(ba)))
        local pop_lim = f32((if truth((if truth(bool(c.windup_deg > 0)) then bool(truth(bool(c.windup_deg < 75.0))) else 0)) then c.windup_deg else 75.0))
        local front_pop_dir = trunc((if truth(bool(fy > 0)) then bool(truth(bool(f_off <= pop_lim))) else 0))
        local back_pop_dir = trunc((if truth(bool(by < 0)) then bool(truth(bool(b_off <= pop_lim))) else 0))
        local front_windup = trunc((if truth((if truth(bool(c.windup_deg > 0)) then bool(truth(bool(not truth(front_pop_dir)))) else 0)) then bool(truth(bool(f_off <= 120.0))) else 0))
        local back_windup = trunc((if truth((if truth(bool(c.windup_deg > 0)) then bool(truth(bool(not truth(back_pop_dir)))) else 0)) then bool(truth(bool(b_off <= 120.0))) else 0))
        if truth(bool(s.d_front_cign == 3)) then
            do
                if truth(bool(fm < off)) then
                    do
                        s.d_front_cign = trunc(0)
                        s.d_front_ignored = trunc(0)
                    end
                else
                    if truth(bool((now - s.d_land_t) >= 150.0)) then
                        do
                            if truth((if truth(front_pop_dir) then bool(truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then
                                do
                                    s.d_front_cign = trunc(0)
                                    s.d_front_ignored = trunc(0)
                                end
                            else
                                s.d_front_cign = trunc(1)
                            end
                        end
                    end
                end
            end
        end
        if truth(bool(s.d_back_cign == 3)) then
            do
                if truth(bool(bm < off)) then
                    do
                        s.d_back_cign = trunc(0)
                        s.d_back_ignored = trunc(0)
                    end
                else
                    if truth(bool((now - s.d_land_t) >= 150.0)) then
                        do
                            if truth((if truth(back_pop_dir) then bool(truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then
                                do
                                    s.d_back_cign = trunc(0)
                                    s.d_back_ignored = trunc(0)
                                end
                            else
                                s.d_back_cign = trunc(1)
                            end
                        end
                    end
                end
            end
        end
        if truth((if truth(bool(s.d_front_cign == 1)) then bool(truth(bool(fm < 0.75))) else 0)) then
            s.d_front_cign = trunc(2)
        end
        if truth((if truth(bool(s.d_back_cign == 1)) then bool(truth(bool(bm < 0.75))) else 0)) then
            s.d_back_cign = trunc(2)
        end
        if truth((if truth(s.d_front_cign) then bool(truth((if truth(bool(fm < off)) then 1 else bool(truth((if truth((if truth(bool(s.d_front_cign == 2)) then bool(truth(front_pop_dir)) else 0)) then bool(truth(bool(fm >= 0.9))) else 0)))))) else 0)) then
            do
                s.d_front_cign = trunc(0)
                s.d_front_ignored = trunc(0)
            end
        end
        if truth((if truth(s.d_back_cign) then bool(truth((if truth(bool(bm < off)) then 1 else bool(truth((if truth((if truth(bool(s.d_back_cign == 2)) then bool(truth(back_pop_dir)) else 0)) then bool(truth(bool(bm >= 0.9))) else 0)))))) else 0)) then
            do
                s.d_back_cign = trunc(0)
                s.d_back_ignored = trunc(0)
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(fm >= on))) else 0)) then bool(truth(bool(not truth(s.d_front_ignored)))) else 0)) then
            do
                if truth((if truth((if truth(s.d_back_on) then 1 else bool(truth(front_pop_dir)))) then 1 else bool(truth(front_windup)))) then
                    do
                        s.d_front_on = trunc(1)
                        s.d_front_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                        if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(not truth(front_pop_dir)))) else 0)) then
                            do
                                s.d_windup = trunc(1)
                                s.d_windup_t = now
                                s.d_log = trunc(1)
                                s.d_log_a = f32(fa)
                                s.d_log_b = f32(0)
                            end
                        end
                    end
                else
                    s.d_front_ignored = trunc(1)
                end
            end
        else
            if truth(bool(fm < off)) then
                do
                    if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(s.d_front_was_up)) else 0)) then
                        s.d_front_rel_t = now
                    end
                    s.d_front_on = trunc(0)
                    s.d_front_spent = trunc(0)
                    s.d_front_ignored = trunc(0)
                    s.d_front_was_up = trunc(0)
                end
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(bm >= on))) else 0)) then bool(truth(bool(not truth(s.d_back_ignored)))) else 0)) then
            do
                if truth((if truth((if truth(s.d_front_on) then 1 else bool(truth(back_pop_dir)))) then 1 else bool(truth(back_windup)))) then
                    do
                        s.d_back_on = trunc(1)
                        s.d_back_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                        if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(back_pop_dir)))) else 0)) then
                            do
                                s.d_windup = trunc(1)
                                s.d_windup_t = now
                                s.d_log = trunc(1)
                                s.d_log_a = f32(ba)
                                s.d_log_b = f32(1)
                            end
                        end
                    end
                else
                    s.d_back_ignored = trunc(1)
                end
            end
        else
            if truth(bool(bm < off)) then
                do
                    if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(s.d_back_was_down)) else 0)) then
                        s.d_back_rel_t = now
                    end
                    s.d_back_on = trunc(0)
                    s.d_back_spent = trunc(0)
                    s.d_back_ignored = trunc(0)
                    s.d_back_was_down = trunc(0)
                end
            end
        end
        if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then
            do
                if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then
                    s.d_two = trunc(0)
                end
            end
        else
            if truth((if truth(s.d_front_on) then bool(truth(s.d_back_on)) else 0)) then
                s.d_two = trunc(1)
            end
        end
        if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(bool(by < (-0.5)))) else 0)) then
            s.d_back_was_down = trunc(1)
        end
        if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(bool(fy > 0.5))) else 0)) then
            s.d_front_was_up = trunc(1)
        end
        if truth((if truth((if truth((if truth(c.quick_pops) then bool(truth(bool(s.phase == 0))) else 0)) then bool(truth(bool(not truth(s.d_windup)))) else 0)) then bool(truth(bool(s.d_pop >= 0))) else 0)) then
            do
                local quick = trunc(0)
                local scooping = trunc((if truth(s.d_pop) then bool(truth(bool(sc_abs(s.d_sweep) > 60.0))) else 0))
                if truth(scooping) then
                    do
                    end
                else
                    if truth((if truth((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(by > 0.45))) else 0)) then bool(truth(bool(sc_abs(bx) < (0.5 * by)))) else 0)) then bool(truth((if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(s.d_back_was_down)) else 0)) then 1 else bool(truth((if truth(bool(s.d_back_rel_t > 0)) then bool(truth(bool((now - s.d_back_rel_t) < 200.0))) else 0)))))) else 0)) then
                        quick = trunc(1)
                    else
                        if truth((if truth((if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(fy < (-0.45)))) else 0)) then bool(truth(bool(sc_abs(fx) < ((-0.5) * fy)))) else 0)) then bool(truth((if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(s.d_front_was_up)) else 0)) then 1 else bool(truth((if truth(bool(s.d_front_rel_t > 0)) then bool(truth(bool((now - s.d_front_rel_t) < 200.0))) else 0)))))) else 0)) then
                            quick = trunc(2)
                        end
                    end
                end
                if truth(quick) then
                    do
                        local qo = {[0]={[0]=f32(0), [1]=f32(0)}, [1]={[0]=f32(0), [1]=f32((-0.95))}, [2]={[0]=f32(0), [1]=f32((-0.95))}}
                        local qn = {[0]={[0]=f32(0), [1]=f32(0)}, [1]={[0]=f32(0), [1]=f32(0.95)}, [2]={[0]=f32(0), [1]=f32(0.95)}}
                        s.d_front_rel_t = 0
                        s.d_back_rel_t = s.d_front_rel_t
                        if truth(bool(quick == 1)) then
                            do
                                s.d_back_on = trunc(0)
                                s.d_back_ignored = trunc(1)
                                s.d_back_was_down = trunc(0)
                            end
                        else
                            do
                                s.d_front_on = trunc(0)
                                s.d_front_ignored = trunc(1)
                                s.d_front_was_up = trunc(0)
                            end
                        end
                        s.d_pop = trunc(0)
                        s.d_pending = trunc(0)
                        s.d_latch = trunc(0)
                        sc_fire(s, (if truth(bool(quick == 1)) then "quickollie" else "quicknollie"), now)
                        s.path = (if truth(bool(quick == 1)) then qo else qn)
                        s.path_n = trunc(3)
                        s.emit_ms_per_unit = f32(c.ms_per_unit)
                        if truth(bool(quick == 2)) then
                            do
                                s.recenter_until = (now + 70.0)
                                s.from_x = f32(0)
                                s.from_y = f32(0)
                            end
                        end
                        local ex = f32(0)
                        local ey = f32(0)
                        sc_emit_step(c, s, now, {get=function() return ex end, set=function(value) ex = f32(value) end}, {get=function() return ey end, set=function(value) ey = f32(value) end})
                        s.d_t = now
                        out_x.set(f32(ex))
                        out_y.set(f32(ey))
                        return
                    end
                end
            end
        end
        if truth(c.pop_on_release) then
            do
                local bmr = f32(sc_sqrt(((bx * bx) + (by * by))))
                local fmr = f32(sc_sqrt(((fx * fx) + (fy * fy))))
                local busy = trunc((if truth((if truth(bool(s.phase ~= 0)) then 1 else bool(truth(s.d_windup)))) then 1 else bool(truth(s.d_pending))))
                if truth((if truth((if truth(busy) then 1 else bool(truth(s.d_front_on)))) then 1 else bool(truth(s.d_back_spent)))) then
                    do
                        s.d_back_full = trunc(0)
                        s.d_back_peak = f32(0)
                    end
                else
                    if truth((if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(bmr >= 0.8))) else 0)) then bool(truth(bool(by < 0))) else 0)) then bool(truth(bool(sc_abs(bx) <= (-by)))) else 0)) then
                        do
                            s.d_back_full = trunc(1)
                            if truth(bool(bmr > s.d_back_peak)) then
                                s.d_back_peak = f32(bmr)
                            end
                        end
                    end
                end
                if truth((if truth((if truth(busy) then 1 else bool(truth(s.d_back_on)))) then 1 else bool(truth(s.d_front_spent)))) then
                    do
                        s.d_front_full = trunc(0)
                        s.d_front_peak = f32(0)
                    end
                else
                    if truth((if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(fmr >= 0.8))) else 0)) then bool(truth(bool(fy > 0))) else 0)) then bool(truth(bool(sc_abs(fx) <= fy))) else 0)) then
                        do
                            s.d_front_full = trunc(1)
                            if truth(bool(fmr > s.d_front_peak)) then
                                s.d_front_peak = f32(fmr)
                            end
                        end
                    end
                end
                local rel = trunc(0)
                if truth((if truth((if truth((if truth(bool(not truth(busy))) then bool(truth(s.d_back_full)) else 0)) then bool(truth(bool(bmr < (s.d_back_peak - 0.12)))) else 0)) then bool(truth((if truth(bool(by > ((-0.85) * s.d_back_peak))) then 1 else bool(truth(bool(bmr < 0.75)))))) else 0)) then
                    rel = trunc(1)
                else
                    if truth((if truth((if truth((if truth(bool(not truth(busy))) then bool(truth(s.d_front_full)) else 0)) then bool(truth(bool(fmr < (s.d_front_peak - 0.12)))) else 0)) then bool(truth((if truth(bool(fy < (0.85 * s.d_front_peak))) then 1 else bool(truth(bool(fmr < 0.75)))))) else 0)) then
                        rel = trunc(2)
                    end
                end
                if truth(rel) then
                    do
                        if truth(bool(rel == 1)) then
                            do
                                s.d_back_on = trunc(0)
                                s.d_back_ignored = trunc(bool(bmr >= (c.active * 0.6)))
                                s.d_back_full = trunc(0)
                                s.d_back_peak = f32(0)
                                s.d_back_was_down = trunc(0)
                            end
                        else
                            do
                                s.d_front_on = trunc(0)
                                s.d_front_ignored = trunc(bool(fmr >= (c.active * 0.6)))
                                s.d_front_full = trunc(0)
                                s.d_front_peak = f32(0)
                                s.d_front_was_up = trunc(0)
                            end
                        end
                        s.d_front_rel_t = 0
                        s.d_back_rel_t = s.d_front_rel_t
                        s.d_pop = trunc(0)
                        s.d_pending = trunc(0)
                        s.d_latch = trunc(0)
                        s.rp_t = now
                        s.rp_y = f32((if truth(bool(rel == 1)) then 0.95 else (-0.95)))
                        s.last_name = (if truth(bool(rel == 1)) then "ollie (let go)" else "nollie (let go)")
                        s.fired = trunc((s.fired + 1))
                    end
                end
            end
        end
        if truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70.0))) else 0)) then
            do
                s.d_t = now
                out_x.set(f32(0))
                out_y.set(f32(s.rp_y))
                return
            end
        end
        s.rp_t = 0
        if truth(bool(s.phase == 2)) then
            do
                local ex = f32(0)
                local ey = f32(0)
                sc_emit_step(c, s, now, {get=function() return ex end, set=function(value) ex = f32(value) end}, {get=function() return ey end, set=function(value) ey = f32(value) end})
                s.d_t = now
                out_x.set(f32(ex))
                out_y.set(f32(ey))
                return
            end
        end
        if truth(bool(s.phase == 3)) then
            do
                s.d_t = now
                out_x.set(f32(0))
                out_y.set(f32(0))
                if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then
                    do
                        s.phase = trunc(0)
                        s.d_pop = trunc(0)
                        s.d_pending = trunc(0)
                        s.d_latch = trunc(0)
                        s.d_windup = trunc(0)
                        s.path_n = trunc(0)
                    end
                end
                return
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(not truth(s.d_pending)))) else 0)) then
            do
                s.d_pop = trunc(0)
                s.d_windup = trunc(0)
            end
        else
            if truth(bool(not truth(s.d_pop))) then
                do
                    if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then
                        do
                            s.d_pop = trunc(1)
                            s.d_last_ang = f32(0)
                        end
                    else
                        if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then
                            do
                                s.d_pop = trunc(2)
                                s.d_last_ang = f32(180)
                            end
                        end
                    end
                    s.d_sweep = f32(0)
                    s.d_side = f32(0)
                    s.d_track = trunc(0)
                    s.d_pending = trunc(0)
                end
            end
        end
        if truth((if truth(s.d_pop) then bool(truth(bool(not truth(s.d_windup)))) else 0)) then
            do
                local back = trunc(bool(s.d_pop == 1))
                local px = f32((if truth(back) then bx else fx))
                local py = f32((if truth(back) then by else fy))
                local pm = f32(sc_sqrt(((px * px) + (py * py))))
                local pop_live = trunc((if truth(back) then (if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0) else (if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)))
                local other_on = trunc((if truth(back) then s.d_front_on else s.d_back_on))
                if truth((if truth((if truth((if truth((if truth(pop_live) then bool(truth(other_on)) else 0)) then bool(truth(s.d_pending)) else 0)) then bool(truth(s.d_track)) else 0)) then bool(truth(bool(pm >= 0.7))) else 0)) then
                    do
                        local a = f32(sc_angle(px, py))
                        local d = f32(sc_wrap((a - s.d_last_ang)))
                        if truth(bool(sc_abs(d) <= 110.0)) then
                            do
                                s.d_sweep = f32((s.d_sweep + d))
                                s.d_last_ang = f32(a)
                            end
                        end
                        if truth(bool(sc_abs(px) > sc_abs(s.d_side))) then
                            s.d_side = f32(px)
                        end
                    end
                end
                if truth((if truth(pop_live) then bool(truth(bool(not truth(other_on)))) else 0)) then
                    do
                        if truth(bool(pm >= 0.7)) then
                            do
                                local a = f32(sc_angle(px, py))
                                if truth(s.d_track) then
                                    do
                                        local d = f32(sc_wrap((a - s.d_last_ang)))
                                        if truth(bool(sc_abs(d) <= 110.0)) then
                                            s.d_sweep = f32((s.d_sweep + d))
                                        end
                                    end
                                end
                                s.d_track = trunc(1)
                                s.d_last_ang = f32(a)
                                if truth(bool(sc_abs(px) > sc_abs(s.d_side))) then
                                    s.d_side = f32(px)
                                end
                            end
                        else
                            s.d_track = trunc(0)
                        end
                        local swept = f32(sc_abs(s.d_sweep))
                        if truth(bool(swept >= 290.0)) then
                            do
                                local right = trunc(bool(bool(s.d_side > 0) ~= bool(c.scoop_mirror ~= 0)))
                                sc_fire(s, (if truth(back) then (if truth(right) then "impossible" else "frontfoot_impossible") else (if truth(right) then "n_impossible" else "n_backfoot_impossible")), now)
                                s.emit_ms_per_unit = f32(c.low_pop_ms)
                                sc_maybe_recenter(c, s, now)
                                local ex = f32(0)
                                local ey = f32(0)
                                sc_emit_step(c, s, now, {get=function() return ex end, set=function(value) ex = f32(value) end}, {get=function() return ey end, set=function(value) ey = f32(value) end})
                                s.d_t = now
                                out_x.set(f32(ex))
                                out_y.set(f32(ey))
                                return
                            end
                        end
                        if truth((if truth(bool(not truth(s.d_pending))) then bool(truth((if truth(bool(swept >= c.shuv_degrees)) then 1 else bool(truth(bool(sc_abs(s.d_side) >= 0.75)))))) else 0)) then
                            do
                                s.d_pending = trunc(1)
                                s.d_pend_since = now
                                s.d_pend_peak = f32(0)
                            end
                        end
                    end
                end
                if truth(s.d_pending) then
                    do
                        local swept = f32(sc_abs(s.d_sweep))
                        local scoop = trunc((if truth(bool(swept >= c.shuv360_degrees)) then 2 else 1))
                        local side = trunc((if truth(bool(s.d_side >= 0)) then 1 else (-1)))
                        if truth(c.scoop_mirror) then
                            side = trunc((-side))
                        end
                        local ox2 = f32((if truth(back) then fx else bx))
                        local oy2 = f32((if truth(back) then fy else by))
                        local om2 = f32(sc_sqrt(((ox2 * ox2) + (oy2 * oy2))))
                        local name = 0
                        local full = trunc(0)
                        if truth((if truth(other_on) then bool(truth(bool(om2 >= on))) else 0)) then
                            do
                                local nx = f32((ox2 / om2))
                                local flip = trunc((if truth(bool(nx > 0.45)) then 1 else (if truth(bool(nx < (-0.45))) then (-1) else 0)))
                                name = sc_trick_name(back, scoop, side, flip)
                                full = trunc((if truth(back) then bool(oy2 >= 0) else bool(oy2 <= 0)))
                                s.d_log = trunc(3)
                                s.d_log_a = f32(s.d_sweep)
                                s.d_log_b = f32(nx)
                            end
                        else
                            if truth(c.pop_on_release) then
                                do
                                    if truth(bool(pm > s.d_pend_peak)) then
                                        s.d_pend_peak = f32(pm)
                                    end
                                    if truth((if truth(bool(not truth(pop_live))) then 1 else bool(truth((if truth(bool(s.d_pend_peak >= 0.7)) then bool(truth(bool(pm < (s.d_pend_peak - 0.12)))) else 0))))) then
                                        do
                                            name = sc_trick_name(back, scoop, side, 0)
                                            full = trunc(1)
                                        end
                                    end
                                end
                            else
                                if truth((if truth(bool(not truth(pop_live))) then 1 else bool(truth(bool((now - s.d_pend_since) >= c.combo_wait_ms))))) then
                                    do
                                        name = sc_trick_name(back, scoop, side, 0)
                                        full = trunc(1)
                                    end
                                end
                            end
                        end
                        if truth(name) then
                            do
                                sc_fire(s, name, now)
                                s.emit_ms_per_unit = f32((if truth(full) then c.ms_per_unit else c.low_pop_ms))
                                s.d_pending = trunc(0)
                                sc_maybe_recenter(c, s, now)
                                local ex = f32(0)
                                local ey = f32(0)
                                sc_emit_step(c, s, now, {get=function() return ex end, set=function(value) ex = f32(value) end}, {get=function() return ey end, set=function(value) ey = f32(value) end})
                                s.d_t = now
                                out_x.set(f32(ex))
                                out_y.set(f32(ey))
                                return
                            end
                        end
                        local cm = f32((if truth(bool(pm > 1.0)) then 1.0 else pm))
                        s.d_t = now
                        s.ox = f32(0)
                        s.oy = f32((if truth(back) then (-cm) else cm))
                        out_x.set(f32(0))
                        out_y.set(f32((if truth(back) then (-cm) else cm)))
                        return
                    end
                end
            end
        end
        local x = f32(0)
        local y = f32(0)
        local full = trunc(0)
        local fl = trunc((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0))
        local bl = trunc((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0))
        if truth((if truth(fl) then bool(truth(bl)) else 0)) then
            do
                if truth(bool(s.d_front_seq > s.d_back_seq)) then
                    do
                        s.d_back_spent = trunc(1)
                        x = f32(ffx)
                        y = f32(ffy)
                        full = trunc(bool(fy >= 0))
                    end
                else
                    do
                        s.d_front_spent = trunc(1)
                        x = f32(bbx)
                        y = f32(bby)
                        full = trunc(bool(by <= 0))
                    end
                end
                if truth(s.d_windup) then
                    do
                        s.d_log = trunc(2)
                        s.d_log_a = f32(sc_angle(x, y))
                        s.d_log_b = f32(sc_angle(s.ox, s.oy))
                        full = trunc(1)
                    end
                end
            end
        else
            if truth(fl) then
                do
                    x = f32(ffx)
                    y = f32(ffy)
                    full = trunc((if truth((if truth(s.d_windup) then 1 else bool(truth(bool(not truth(s.d_two)))))) then 1 else bool(truth((if truth(s.d_back_spent) then bool(truth(bool(fy >= 0))) else 0)))))
                end
            else
                if truth(bl) then
                    do
                        x = f32(bbx)
                        y = f32(bby)
                        full = trunc((if truth((if truth(s.d_windup) then 1 else bool(truth(bool(not truth(s.d_two)))))) then 1 else bool(truth((if truth(s.d_front_spent) then bool(truth(bool(by <= 0))) else 0)))))
                    end
                else
                    if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then
                        do
                            s.d_land_hold = trunc(0)
                            if truth((if truth(s.d_land_hold) then bool(truth((if truth((if truth(s.d_front_ignored) then bool(truth(bool(fm >= 0.85))) else 0)) then 1 else bool(truth((if truth(s.d_back_ignored) then bool(truth(bool(bm >= 0.85))) else 0)))))) else 0)) then
                                do
                                    s.d_land_hold = trunc(0)
                                    s.d_front_ignored = trunc(0)
                                    s.d_back_ignored = trunc(0)
                                end
                            end
                            local lh = trunc(s.d_land_hold)
                            local front_lean = trunc((if truth(bool(fy > 0)) then bool(truth((if truth(bool(not truth(s.d_front_ignored))) then 1 else bool(truth(lh))))) else 0))
                            local back_lean = trunc((if truth(bool(by < 0)) then bool(truth((if truth(bool(not truth(s.d_back_ignored))) then 1 else bool(truth(lh))))) else 0))
                            if truth((if truth(front_lean) then bool(truth((if truth(bool(not truth(back_lean))) then 1 else bool(truth(bool(fm >= bm)))))) else 0)) then
                                do
                                    x = f32(ffx)
                                    y = f32(ffy)
                                end
                            else
                                if truth(back_lean) then
                                    do
                                        x = f32(bbx)
                                        y = f32(bby)
                                    end
                                end
                            end
                        end
                    else
                        full = trunc(bool(not truth(s.d_latch)))
                    end
                end
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_two))) then bool(truth(bool(not truth(s.d_windup)))) else 0)) then bool(truth(bool(fl ~= bl))) else 0)) then
            do
                local m = f32(sc_sqrt(((x * x) + (y * y))))
                x = f32(0)
                y = f32((if truth(fl) then m else (-m)))
                if truth((if truth(bool(m >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then bool(truth(bool(not truth(s.d_latch)))) else 0)) then
                    y = f32((if truth(fl) then 0.97 else (-0.97)))
                end
            end
        end
        if truth((if truth((if truth((if truth(s.d_windup) then bool(truth(bool(not truth((if truth(fl) then bool(truth(bl)) else 0))))) else 0)) then bool(truth(c.powerslide)) else 0)) then bool(truth(bool((now - s.d_windup_t) < 40.0))) else 0)) then
            do
                x = f32(0)
                y = f32(0)
                full = trunc(1)
            end
        end
        local m = f32(sc_sqrt(((x * x) + (y * y))))
        if truth(bool(m > 1.0)) then
            do
                x = f32((x / m))
                y = f32((y / m))
            end
        end
        local dt = (if truth(bool(s.d_t > 0)) then (now - s.d_t) else 0)
        s.d_t = now
        if truth(full) then
            s.d_latch = trunc(0)
        else
            if truth((if truth(bool(m >= on)) then bool(truth((if truth((if truth(bool(not truth(s.d_latch))) then 1 else bool(truth(bool(m >= (sc_sqrt(((s.d_lx * s.d_lx) + (s.d_ly * s.d_ly))) - 0.01)))))) then 1 else bool(truth(bool(((x * s.d_lx) + (y * s.d_ly)) < ((0.5 * m) * sc_sqrt(((s.d_lx * s.d_lx) + (s.d_ly * s.d_ly)))))))))) else 0)) then
                do
                    s.d_latch = trunc(1)
                    s.d_lx = f32(x)
                    s.d_ly = f32(y)
                end
            else
                if truth(bool(m >= on)) then
                    do
                        x = f32(s.d_lx)
                        y = f32(s.d_ly)
                        local ex = f32((s.d_lx - s.ox))
                        local ey = f32((s.d_ly - s.oy))
                        if truth(bool(sc_sqrt(((ex * ex) + (ey * ey))) < 0.08)) then
                            do
                                s.d_latch = trunc(0)
                                x = f32(s.ox)
                                y = f32(s.oy)
                            end
                        end
                    end
                else
                    if truth(s.d_latch) then
                        do
                            local ex = f32((s.d_lx - s.ox))
                            local ey = f32((s.d_ly - s.oy))
                            if truth(bool(sc_sqrt(((ex * ex) + (ey * ey))) < 0.08)) then
                                s.d_latch = trunc(0)
                            else
                                do
                                    x = f32(s.d_lx)
                                    y = f32(s.d_ly)
                                end
                            end
                        end
                    end
                end
            end
        end
        if truth((if truth(bool(not truth(full))) then bool(truth(bool(c.low_pop_ms > 0))) else 0)) then
            do
                local dx = f32((x - s.ox))
                local dy = f32((y - s.oy))
                local d = f32(sc_sqrt(((dx * dx) + (dy * dy))))
                local step = f32(f32((dt / c.low_pop_ms)))
                if truth((if truth(bool(d > step)) then bool(truth(bool(d > 0))) else 0)) then
                    do
                        x = f32((s.ox + ((dx / d) * step)))
                        y = f32((s.oy + ((dy / d) * step)))
                    end
                end
            end
        end
        out_x.set(f32(x))
        out_y.set(f32(y))
    end
end
local function sc_own_off(k, a)
    do
        return f32((if truth(k) then sc_abs(sc_wrap(a)) else sc_abs(sc_wrap((a - 180.0)))))
    end
end
local function sc_vec_at(k, off_signed, m, x, y)
    do
        local o = f32((if truth(bool(off_signed < 0)) then (-off_signed) else off_signed))
        local sx = f32((if truth(bool(off_signed < 0)) then (-1.0) else 1.0))
        local sn = f32(sc_sin_deg(o))
        local cs = f32(sc_sin_deg((90.0 - o)))
        x.set(f32(((sx * m) * sn)))
        y.set(f32((if truth(k) then ((-m) * cs) else (m * cs))))
    end
end
local function sc_lone_start(s, k, x, y)
    do
        local a = f32(sc_angle(x, y))
        s.lone_a0[k] = f32(a)
        s.lone_last[k] = f32(a)
        s.lone_sw[k] = f32(0)
        s.lone_unl[k] = trunc(0)
        s.lone_out[k] = f32(0)
        s.lone_seen[k] = trunc(0)
        s.lone_from[k] = trunc(0)
    end
end
local function sc_lone_clamp(s, k, x, y)
    do
        local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
        if truth(bool(m < 0.05)) then
            return
        end
        local a = f32(sc_angle(x.get(), y.get()))
        s.lone_sw[k] = f32((s.lone_sw[k] + sc_wrap((a - s.lone_last[k]))))
        s.lone_last[k] = f32(a)
        local start_side = trunc(s.lone_side[k])
        local off = f32(sc_own_off(k, a))
        local soff = f32((if truth(bool(x.get() < 0)) then (-off) else off))
        local base = f32((if truth(k) then 0.0 else 180.0))
        if truth((if truth((if truth((if truth((if truth(bool(not truth(start_side))) then bool(truth(bool(sc_own_off(k, s.lone_a0[k]) <= 35.0))) else 0)) then bool(truth(bool(sc_abs(s.lone_sw[k]) >= 240.0))) else 0)) then bool(truth(bool(m >= 0.8))) else 0)) then bool(truth(bool(not truth(s.lone_unl[k])))) else 0)) then
            do
                local dir = f32((if truth(bool(s.lone_sw[k] > 0)) then 1.0 else (-1.0)))
                s.lone_unl[k] = trunc(1)
                s.lone_out[k] = f32(((dir * 75.0) + sc_wrap((base - s.lone_a0[k]))))
            end
        end
        if truth(s.lone_unl[k]) then
            do
                local d = f32((s.lone_sw[k] - s.lone_out[k]))
                if truth(bool(d > 30.0)) then
                    d = f32(30.0)
                end
                if truth(bool(d < (-30.0))) then
                    d = f32((-30.0))
                end
                s.lone_out[k] = f32((s.lone_out[k] + d))
                local aa = f32((s.lone_a0[k] + s.lone_out[k]))
                while truth(bool(aa < 0)) do
                    aa = f32((aa + 360.0))
                end
                while truth(bool(aa >= 360.0)) do
                    aa = f32((aa - 360.0))
                end
                x.set(f32((m * sc_sin_deg(aa))))
                y.set(f32(((-m) * sc_sin_deg((90.0 - aa)))))
                return
            end
        end
        local lim = f32(75.0)
        do
            local side_now = trunc((if truth(bool(x.get() > 0)) then 1 else (-1)))
            if truth(bool(off >= 30.0)) then
                do
                    if truth((if truth((if truth(s.lone_seen[k]) then bool(truth(bool(side_now ~= s.lone_seen[k]))) else 0)) then bool(truth(bool(s.lone_sw[k] ~= 0))) else 0)) then
                        do
                        end
                    end
                    s.lone_seen[k] = trunc(side_now)
                end
            end
            if truth((if truth(bool(off < 25.0)) then bool(truth(s.lone_seen[k])) else 0)) then
                s.lone_from[k] = trunc(s.lone_seen[k])
            end
            local from = trunc((if truth(s.lone_from[k]) then s.lone_from[k] else start_side))
            if truth((if truth(from) then bool(truth(bool(side_now ~= from))) else 0)) then
                lim = f32(65.0)
            end
        end
        if truth(bool(sc_abs(s.lone_sw[k]) > 90.0)) then
            do
                local sd = f32((if truth(bool(s.lone_sw[k] > 0)) then 1.0 else (-1.0)))
                if truth(bool(not truth(k))) then
                    sd = f32((-sd))
                end
                if truth(bool((sd * soff) < 0)) then
                    off = f32((360.0 - off))
                end
                soff = f32((sd * off))
            end
        end
        if truth(bool(off <= lim)) then
            return
        end
        sc_vec_at(k, (if truth(bool(soff < 0)) then (-lim) else lim), m, x, y)
    end
end
local function sc_rail_scoop(s, now, fx, fy, bx, by, off)
    do
        local fm = f32(sc_sqrt(((fx * fx) + (fy * fy))))
        local bm = f32(sc_sqrt(((bx * bx) + (by * by))))
        local foot = trunc((if truth((if truth(bool(fm >= .5)) then bool(truth(bool(bm < off))) else 0)) then 1 else (if truth((if truth(bool(bm >= .5)) then bool(truth(bool(fm < off))) else 0)) then 2 else 0)))
        if truth((if truth((if truth(bool(not truth(s.grind))) then 1 else bool(truth(s.grind_pop)))) then 1 else bool(truth(bool(not truth(foot)))))) then
            do
                s.rail_scoop_candidate = trunc(0)
                s.rail_scoop_track = trunc(s.rail_scoop_candidate)
                return trunc(0)
            end
        end
        local x = f32((if truth(bool(foot == 1)) then fx else bx))
        local y = f32((if truth(bool(foot == 1)) then fy else by))
        local own = f32((if truth(bool(foot == 1)) then y else (-y)))
        local a = f32(sc_angle(x, y))
        local dt = (now - s.rail_scoop_last)
        if truth((if truth((if truth(bool(s.rail_scoop_track == foot)) then bool(truth(bool(dt > 0))) else 0)) then bool(truth(bool(dt <= 100))) else 0)) then
            do
                local delta = f32(sc_wrap((a - s.rail_scoop_angle)))
                if truth(bool(sc_abs(delta) <= 110)) then
                    do
                        s.rail_scoop_sweep = f32((s.rail_scoop_sweep + delta))
                        if truth((if truth(bool(sc_abs(s.rail_scoop_sweep) >= 2)) then bool(truth(bool((sc_abs(delta) * 1000.0) >= (400.0 * dt)))) else 0)) then
                            s.rail_scoop_candidate = trunc(foot)
                        end
                        if truth(bool(sc_abs(s.rail_scoop_sweep) < 8)) then
                            s.rail_scoop_turn = 0
                        else
                            if truth(bool(not truth(s.rail_scoop_turn))) then
                                s.rail_scoop_turn = s.rail_scoop_last
                            end
                        end
                        if truth(s.rail_scoop_turn) then
                            do
                                local elapsed = (now - s.rail_scoop_turn)
                                if truth((if truth(bool(elapsed > 0)) then bool(truth(bool((sc_abs(s.rail_scoop_sweep) * 1000.0) >= (400.0 * elapsed)))) else 0)) then
                                    s.rail_scoop_candidate = trunc(foot)
                                end
                                if truth((if truth((if truth(bool(sc_abs(s.rail_scoop_sweep) >= 35)) then bool(truth(bool(elapsed > 0))) else 0)) then bool(truth(bool((sc_abs(s.rail_scoop_sweep) * 1000.0) >= (400.0 * elapsed)))) else 0)) then
                                    do
                                        s.rail_scoop_exit = trunc(foot)
                                        s.rail_scoop_track = trunc(0)
                                        return trunc(foot)
                                    end
                                end
                            end
                        end
                        s.rail_scoop_angle = f32(a)
                        s.rail_scoop_last = now
                        if truth((if truth((if truth(bool(sc_abs(s.rail_scoop_sweep) < 35)) then 1 else bool(truth(bool(own > .6))))) then 1 else bool(truth(bool(s.rail_scoop_candidate == foot))))) then
                            return trunc(0)
                        end
                    end
                end
                s.rail_scoop_candidate = trunc(0)
                s.rail_scoop_track = trunc(s.rail_scoop_candidate)
            end
        end
        if truth((if truth(bool(own >= .35)) then bool(truth(bool(sc_abs(x) <= (.7 * own)))) else 0)) then
            do
                s.rail_scoop_candidate = trunc(0)
                s.rail_scoop_track = trunc(foot)
                s.rail_scoop_angle = f32(a)
                s.rail_scoop_last = now
                s.rail_scoop_sweep = f32(0)
                s.rail_scoop_turn = 0
                s.rail_scoop_x = f32(x)
                s.rail_scoop_y = f32(y)
            end
        else
            s.rail_scoop_candidate = trunc(0)
            s.rail_scoop_track = trunc(s.rail_scoop_candidate)
        end
        return trunc(0)
    end
end
local function sc_split_board(fx, fy, bx, by, gain, x, y, lean)
    do
        local rear = f32((if truth(bool(by < 0)) then (-by) else 0))
        local front = f32((if truth(bool(fy > 0)) then fy else 0))
        x.set(f32((((fx - bx) / ((1.0 + sc_abs(fy)) + sc_abs(by))) * gain)))
        lean.set(f32(0))
        y.set(f32((((fy + by) + (.7 * ((sc_abs(bx) * front) - (sc_abs(fx) * rear)))) * gain)))
        local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
        if truth(bool(m > 1)) then
            do
                x.set(f32((x.get() / m)))
                y.set(f32((y.get() / m)))
            end
        end
    end
end
local function sc_split_position(c, fx, fy, bx, by, gain, x, y)
    do
        local lean = 0
        if truth(c.one_foot_flips) then
            do
                sc_split_board(fx, fy, bx, by, gain, x, y, {get=function() return lean end, set=function(value) lean = f32(value) end})
                return
            end
        end
        local fm = f32(((fx * fx) + (fy * fy)))
        local bm = f32(((bx * bx) + (by * by)))
        if truth(bool(fy < 0)) then
            fy = f32(0)
        end
        if truth(bool(by > 0)) then
            by = f32(0)
        end
        sc_split_board(fx, fy, bx, by, gain, x, y, {get=function() return lean end, set=function(value) lean = f32(value) end})
        local off = f32((c.active * .6))
        off = f32((off * off))
        local front = trunc((if truth(bool(fm >= off)) then bool(truth(bool(bm < off))) else 0))
        local back = trunc((if truth(bool(bm >= off)) then bool(truth(bool(fm < off))) else 0))
        if truth((if truth(front) then 1 else bool(truth(back)))) then
            do
                local own = f32((if truth(back) then (-y.get()) else y.get()))
                if truth(bool(own < 0)) then
                    y.set(f32(0))
                end
                local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
                if truth(bool(m > 1)) then
                    do
                        x.set(f32((x.get() / m)))
                        y.set(f32((y.get() / m)))
                    end
                end
            end
        end
    end
end
local function sc_split_rail_position(c, s, fx, fy, bx, by, gain, x, y)
    do
        local raw_back_angle = f32(sc_angle(bx, by))
        local off = f32((c.active * .6))
        local off2 = f32((off * off))
        local solo_back = trunc((if truth(bool(((fx * fx) + (fy * fy)) < off2)) then bool(truth(bool(((bx * bx) + (by * by)) >= off2))) else 0))
        if truth(bool(not truth(c.one_foot_flips))) then
            do
                local fm = f32(((fx * fx) + (fy * fy)))
                local bm = f32(((bx * bx) + (by * by)))
                if truth((if truth(bool(fm < off2)) then bool(truth(bool(bm >= off2))) else 0)) then
                    do
                        s.split_rail_front_load = f32(0)
                        if truth(bool((-by) > s.split_rail_back_load)) then
                            s.split_rail_back_load = f32((-by))
                        end
                        if truth(bool(s.split_rail_back_load > 0)) then
                            by = f32((-s.split_rail_back_load))
                        end
                    end
                else
                    if truth((if truth(bool(bm < off2)) then bool(truth(bool(fm >= off2))) else 0)) then
                        do
                            s.split_rail_back_load = f32(0)
                            if truth(bool(fy > s.split_rail_front_load)) then
                                s.split_rail_front_load = f32(fy)
                            end
                            if truth(bool(s.split_rail_front_load > 0)) then
                                fy = f32(s.split_rail_front_load)
                            end
                        end
                    else
                        if truth((if truth(bool(fm >= off2)) then bool(truth(bool(bm >= off2))) else 0)) then
                            do
                                fy = f32(sc_abs(fy))
                                by = f32((-sc_abs(by)))
                                if truth((if truth(bool(s.split_rail_back_load > 0)) then bool(truth(bool(by > (-s.split_rail_back_load)))) else 0)) then
                                    by = f32((-s.split_rail_back_load))
                                end
                                if truth((if truth(bool(s.split_rail_front_load > 0)) then bool(truth(bool(fy < s.split_rail_front_load))) else 0)) then
                                    fy = f32(s.split_rail_front_load)
                                end
                            end
                        else
                            s.split_rail_back_load = f32(0)
                            s.split_rail_front_load = f32(s.split_rail_back_load)
                        end
                    end
                end
            end
        end
        sc_split_position(c, fx, fy, bx, by, gain, x, y)
        if truth((if truth(bool(not truth(c.one_foot_flips))) then bool(truth(solo_back)) else 0)) then
            do
                if truth(bool(not truth(s.split_rear_turn))) then
                    do
                        if truth(bool(((x.get() * x.get()) + (y.get() * y.get())) > .0001)) then
                            do
                                s.split_rear_turn = trunc(1)
                                s.split_rear_adjusted = trunc(0)
                                s.split_rear_angle = f32(raw_back_angle)
                                s.split_rear_target = f32(sc_wrap(sc_angle(x.get(), y.get())))
                            end
                        end
                    end
                else
                    do
                        local delta = f32(sc_wrap((raw_back_angle - s.split_rear_angle)))
                        s.split_rear_angle = f32(raw_back_angle)
                        if truth(bool(sc_abs(delta) > .001)) then
                            do
                                s.split_rear_adjusted = trunc(1)
                                s.split_rear_target = f32((s.split_rear_target + delta))
                                s.split_rear_target = f32(sc_wrap(s.split_rear_target))
                            end
                        end
                        if truth(s.split_rear_adjusted) then
                            do
                                local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
                                sc_vec_at(1, s.split_rear_target, m, x, y)
                                y.set(f32((-sc_abs(y.get()))))
                            end
                        end
                    end
                end
            end
        else
            s.split_rear_turn = trunc(0)
        end
    end
end
local function sc_fine_board(fx, fy, bx, by, gain, x, y)
    do
        x.set(f32((((fx - bx) / ((1.0 + sc_abs(fy)) + sc_abs(by))) * gain)))
        local rear = f32((if truth(bool(by < 0)) then (-by) else 0))
        local front = f32((if truth(bool(fy > 0)) then fy else 0))
        y.set(f32(((((fy * sc_abs(fy)) + (by * sc_abs(by))) + (.7 * ((sc_abs(bx) * front) - (sc_abs(fx) * rear)))) * gain)))
        local m = f32(sc_sqrt(((x.get() * x.get()) + (y.get() * y.get()))))
        if truth(bool(m > 1)) then
            do
                x.set(f32((x.get() / m)))
                y.set(f32((y.get() / m)))
            end
        end
    end
end
local function sc_pure(c, s, now, fx, fy, bx, by, out_x, out_y)
    do
        local airborne = trunc((if truth(s.air) then 1 else bool(truth(bool(s.takeoff > 0)))))
        if truth((if truth((if truth((if truth((if truth((if truth((if truth(airborne) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(bool(not truth(c.split_grinds)))))) then 1 else bool(truth(bool(not truth(c.air_controls)))))) then 1 else bool(truth(c.one_foot_flips)))) then 1 else bool(truth(bool(now < s.split_pop_t))))) then 1 else bool(truth(bool((now - s.split_pop_t) > 350.0))))) then
            s.split_pop_t = 0
        end
        local previous_split_setup = trunc(s.split_air_setup)
        s.split_air_setup = trunc(0)
        local rail_bx = f32((if truth(c.flip_x) then (-bx) else bx))
        local rail_by = f32(by)
        local entry_lateral = f32(sc_abs(bx))
        local position_deadzone = f32((if truth((if truth((if truth((if truth((if truth(c.fine_grinds) then 1 else bool(truth(c.split_grinds)))) then bool(truth(c.air_controls)) else 0)) then bool(truth((if truth((if truth((if truth(airborne) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(s.split_landing_setup)))) then 1 else bool(truth(previous_split_setup))))) else 0)) then bool(truth(bool(c.deadzone > .10))) else 0)) then .10 else c.deadzone))
        sc_deadzone({get=function() return fx end, set=function(value) fx = f32(value) end}, {get=function() return fy end, set=function(value) fy = f32(value) end}, position_deadzone)
        sc_deadzone({get=function() return bx end, set=function(value) bx = f32(value) end}, {get=function() return by end, set=function(value) by = f32(value) end}, position_deadzone)
        local mirror = trunc(c.flip_x)
        if truth(mirror) then
            do
                fx = f32((-fx))
                bx = f32((-bx))
            end
        end
        local fm = f32(sc_sqrt(((fx * fx) + (fy * fy))))
        local bm = f32(sc_sqrt(((bx * bx) + (by * by))))
        if truth(bool(fm >= 0.95)) then
            do
                fx = f32((fx / fm))
                fy = f32((fy / fm))
                fm = f32(1.0)
            end
        end
        if truth(bool(bm >= 0.95)) then
            do
                bx = f32((bx / bm))
                by = f32((by / bm))
                bm = f32(1.0)
            end
        end
        local on = f32(c.active)
        local off = f32((c.active * 0.6))
        s.phase = trunc(0)
        s.d_pending = trunc(0)
        s.d_latch = trunc(0)
        s.path_n = trunc(0)
        if truth((if truth((if truth(c.pop_on_release) then bool(truth(bool(not truth(airborne)))) else 0)) then bool(truth((if truth((if truth(bool(fm >= on)) then bool(truth((if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then 1 else bool(truth(s.d_back_full)))) then 1 else bool(truth((if truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70))) else 0)) then bool(truth(bool(s.rp_y > 0))) else 0)))))) else 0)) then 1 else bool(truth((if truth(bool(bm >= on)) then bool(truth((if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then 1 else bool(truth(s.d_front_full)))) then 1 else bool(truth((if truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70))) else 0)) then bool(truth(bool(s.rp_y < 0))) else 0)))))) else 0)))))) else 0)) then
            do
                s.rp_two_foot = trunc(1)
                s.rp_t = 0
            end
        end
        if truth(s.rp_two_foot) then
            do
                s.d_back_full = trunc(0)
                s.d_front_full = trunc(s.d_back_full)
                s.d_back_peak = f32(0)
                s.d_front_peak = f32(s.d_back_peak)
                if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then
                    s.rp_two_foot = trunc(0)
                end
            end
        end
        if truth((if truth((if truth((if truth(bool(fm >= on)) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(s.d_back_on)) else 0)) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then
            s.d_front_flick = trunc(1)
        end
        if truth((if truth((if truth((if truth(bool(bm >= on)) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(s.d_front_on)) else 0)) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then
            s.d_back_flick = trunc(1)
        end
        if truth(bool(fm < off)) then
            s.d_front_flick = trunc(0)
        end
        if truth(bool(bm < off)) then
            s.d_back_flick = trunc(0)
        end
        if truth(bool(fm < off)) then
            do
                s.d_front_on = trunc(0)
                s.d_front_spent = trunc(0)
                s.d_front_ignored = trunc(0)
            end
        end
        if truth(bool(bm < off)) then
            do
                s.d_back_on = trunc(0)
                s.d_back_spent = trunc(0)
                s.d_back_ignored = trunc(0)
            end
        end
        if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then
            s.d_windup = trunc(0)
        end
        if truth((if truth(airborne) then bool(truth(bool(not truth(s.d_took_off)))) else 0)) then
            do
                local session_scoop = trunc((if truth((if truth((if truth(c.front_back) then bool(truth(bool(not truth(c.xl_layout)))) else 0)) then bool(truth(bool(not truth(c.pop_on_release)))) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0))
                local fscoop = trunc((if truth((if truth((if truth((if truth((if truth((if truth(session_scoop) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(fm >= .8))) else 0)) then bool(truth(bool(bm < off))) else 0)) then bool(truth(s.d_front_on)) else 0)) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(bool(sc_abs(s.lone_sw[0]) >= 35))) else 0))
                local bscoop = trunc((if truth((if truth((if truth((if truth((if truth((if truth(session_scoop) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(bm >= .8))) else 0)) then bool(truth(bool(fm < off))) else 0)) then bool(truth(s.d_back_on)) else 0)) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(bool(sc_abs(s.lone_sw[1]) >= 35))) else 0))
                s.d_took_off = trunc(1)
                if truth(bool(fm >= off)) then
                    do
                        s.d_front_on = trunc(1)
                        s.d_front_spent = trunc(1)
                        s.tk_fa = f32(sc_angle(fx, fy))
                    end
                end
                if truth(bool(bm >= off)) then
                    do
                        s.d_back_on = trunc(1)
                        s.d_back_spent = trunc(1)
                        s.tk_ba = f32(sc_angle(bx, by))
                    end
                end
                s.d_windup = trunc(0)
                s.rp_t = 0
                s.d_front_full = trunc(0)
                s.d_back_full = trunc(s.d_front_full)
                s.hold_scoop = trunc(0)
                s.hold_foot = trunc(s.hold_scoop)
                do
                    local fh = trunc((if truth(bool(fm >= 0.8)) then bool(truth((if truth((if truth(fscoop) then 1 else bool(truth(s.d_front_flick)))) then 1 else bool(truth((if truth(c.one_foot_flips) then bool(truth(bool(fy < ((-0.2) * fm)))) else 0)))))) else 0))
                    local bh = trunc((if truth(bool(bm >= 0.8)) then bool(truth((if truth((if truth(bscoop) then 1 else bool(truth(s.d_back_flick)))) then 1 else bool(truth((if truth(c.one_foot_flips) then bool(truth(bool(by > (0.2 * bm)))) else 0)))))) else 0))
                    if truth((if truth(fh) then bool(truth((if truth(bool(not truth(bh))) then 1 else bool(truth(bool(s.d_front_seq > s.d_back_seq)))))) else 0)) then
                        s.hold_foot = trunc(1)
                    else
                        if truth(bh) then
                            s.hold_foot = trunc(2)
                        end
                    end
                    local om = f32(sc_sqrt(((s.ox * s.ox) + (s.oy * s.oy))))
                    local ha = f32((if truth(bool(s.hold_foot == 1)) then sc_angle(fx, fy) else sc_angle(bx, by)))
                    local scoop = trunc((if truth(bool(s.hold_foot == 1)) then fscoop else (if truth(bool(s.hold_foot == 2)) then bscoop else 0)))
                    if truth((if truth((if truth(s.hold_foot) then bool(truth(bool(om >= 0.8))) else 0)) then bool(truth((if truth(scoop) then 1 else bool(truth(bool(sc_abs(sc_wrap((ha - sc_angle(s.ox, s.oy)))) < 45.0)))))) else 0)) then
                        do
                            s.hold_x = f32((s.ox / om))
                            s.hold_y = f32((s.oy / om))
                            s.hold_scoop = trunc(scoop)
                        end
                    else
                        s.hold_foot = trunc(0)
                    end
                end
            end
        else
            if truth(bool(not truth(airborne))) then
                do
                    s.d_took_off = trunc(0)
                    s.hold_scoop = trunc(0)
                    s.hold_foot = trunc(s.hold_scoop)
                end
            end
        end
        local fa = f32(sc_angle(fx, fy))
        local ba = f32(sc_angle(bx, by))
        local f_off = f32(sc_abs(sc_wrap((fa - 180.0))))
        local b_off = f32(sc_abs(sc_wrap(ba)))
        local pop_lim = f32((if truth((if truth(bool(c.windup_deg > 0)) then bool(truth(bool(c.windup_deg < 75.0))) else 0)) then c.windup_deg else 75.0))
        local front_pop_dir = trunc((if truth(bool(fy > 0)) then bool(truth(bool(f_off <= pop_lim))) else 0))
        local back_pop_dir = trunc((if truth(bool(by < 0)) then bool(truth(bool(b_off <= pop_lim))) else 0))
        local side_lim = f32((if truth(bool(c.windup_deg > 0)) then 120.0 else pop_lim))
        if truth(bool(not truth(c.air_controls))) then
            do
                s.rail_scoop_exit = trunc(0)
                s.rail_scoop_track = trunc(s.rail_scoop_exit)
                s.fine_exit_stage = trunc(0)
                s.fine_back_started = 0
                s.fine_front_started = s.fine_back_started
            end
        end
        if truth(c.air_controls) then
            do
                local fpop = trunc((if truth(bool(fy >= 0.70)) then bool(truth(bool(sc_abs(fx) < (0.65 * fm)))) else 0))
                local bpop = trunc((if truth(bool(by <= (-0.70))) then bool(truth(bool(sc_abs(bx) < (0.65 * bm)))) else 0))
                local fflick = trunc((if truth(bool(fm >= on)) then bool(truth((if truth(bool(fy > (0.2 * fm))) then 1 else bool(truth(bool(sc_abs(fx) > (0.7 * fm))))))) else 0))
                local bflick = trunc((if truth(bool(bm >= on)) then bool(truth((if truth(bool(by < ((-0.2) * bm))) then 1 else bool(truth(bool(sc_abs(bx) > (0.7 * bm))))))) else 0))
                local release_back = trunc(0)
                local release_front = trunc(0)
                local scoop_foot = trunc(sc_rail_scoop(s, now, fx, fy, bx, by, off))
                local fine_release_front = trunc(0)
                local fine_release_back = trunc(0)
                if truth((if truth((if truth(c.fine_grinds) then 1 else bool(truth(c.split_grinds)))) then bool(truth(s.grind)) else 0)) then
                    do
                        if truth(bool(not truth(s.grind_prev))) then
                            do
                                s.fine_exit_stage = trunc((if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then 1 else 0))
                                s.fine_front_started = (if truth(bool(fm < off)) then 0 else (-1))
                                s.fine_back_started = (if truth(bool(bm < off)) then 0 else (-1))
                            end
                        end
                        if truth(bool(fm < off)) then
                            s.fine_front_started = 0
                        else
                            if truth(bool(s.fine_front_started == 0)) then
                                s.fine_front_started = now
                            end
                        end
                        if truth(bool(bm < off)) then
                            s.fine_back_started = 0
                        else
                            if truth(bool(s.fine_back_started == 0)) then
                                s.fine_back_started = now
                            end
                        end
                        fine_release_front = trunc(bool(s.fine_exit_stage == 2))
                        fine_release_back = trunc(bool(s.fine_exit_stage == 3))
                        fflick = trunc((if truth((if truth((if truth((if truth((if truth((if truth(fflick) then bool(truth(bool(fm >= .85))) else 0)) then bool(truth(bool(s.fine_exit_stage == 3))) else 0)) then bool(truth(bpop)) else 0)) then bool(truth(bool(bm >= .85))) else 0)) then bool(truth(bool(s.fine_front_started > 0))) else 0)) then bool(truth(bool((now - s.fine_front_started) <= 90))) else 0))
                        bflick = trunc((if truth((if truth((if truth((if truth((if truth((if truth(bflick) then bool(truth(bool(bm >= .85))) else 0)) then bool(truth(bool(s.fine_exit_stage == 2))) else 0)) then bool(truth(fpop)) else 0)) then bool(truth(bool(fm >= .85))) else 0)) then bool(truth(bool(s.fine_back_started > 0))) else 0)) then bool(truth(bool((now - s.fine_back_started) <= 90))) else 0))
                        if truth((if truth(scoop_foot) then bool(truth(bool(s.fine_exit_stage ~= (scoop_foot + 1)))) else 0)) then
                            do
                                scoop_foot = trunc(0)
                                s.rail_scoop_exit = trunc(0)
                            end
                        end
                        if truth((if truth(s.rail_scoop_candidate) then bool(truth(bool(s.fine_exit_stage ~= (s.rail_scoop_candidate + 1)))) else 0)) then
                            s.rail_scoop_candidate = trunc(0)
                        end
                        if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then
                            s.fine_exit_stage = trunc(1)
                        else
                            if truth((if truth((if truth((if truth(bool(s.fine_exit_stage == 1)) then bool(truth(bpop)) else 0)) then bool(truth(bool(bm >= .85))) else 0)) then bool(truth(bool(fm < off))) else 0)) then
                                s.fine_exit_stage = trunc(3)
                            else
                                if truth((if truth((if truth((if truth(bool(s.fine_exit_stage == 1)) then bool(truth(fpop)) else 0)) then bool(truth(bool(fm >= .85))) else 0)) then bool(truth(bool(bm < off))) else 0)) then
                                    s.fine_exit_stage = trunc(2)
                                else
                                    if truth((if truth((if truth(bool(s.fine_exit_stage == 3)) then bool(truth((if truth(bool(not truth(bpop))) then 1 else bool(truth((if truth((if truth(bool(fm >= off)) then bool(truth(bool(s.fine_front_started > 0))) else 0)) then bool(truth(bool((now - s.fine_front_started) > 90))) else 0)))))) else 0)) then 1 else bool(truth((if truth(bool(s.fine_exit_stage == 2)) then bool(truth((if truth(bool(not truth(fpop))) then 1 else bool(truth((if truth((if truth(bool(bm >= off)) then bool(truth(bool(s.fine_back_started > 0))) else 0)) then bool(truth(bool((now - s.fine_back_started) > 90))) else 0)))))) else 0))))) then
                                        s.fine_exit_stage = trunc(0)
                                    end
                                end
                            end
                        end
                    end
                else
                    do
                        s.fine_exit_stage = trunc(0)
                        s.fine_back_started = 0
                        s.fine_front_started = s.fine_back_started
                    end
                end
                if truth(bool(not truth(s.grind))) then
                    do
                        s.grind_pop = trunc(0)
                        s.rail_scoop_exit = trunc(0)
                        s.grind_bpop_t = 0
                        s.grind_fpop_t = s.grind_bpop_t
                    end
                else
                    do
                        if truth((if truth(fpop) then bool(truth((if truth(bool(not truth(s.grind_prev))) then 1 else bool(truth(bool(not truth(s.grind_fpop))))))) else 0)) then
                            s.grind_fpop_t = now
                        end
                        if truth((if truth(bpop) then bool(truth((if truth(bool(not truth(s.grind_prev))) then 1 else bool(truth(bool(not truth(s.grind_bpop))))))) else 0)) then
                            s.grind_bpop_t = now
                        end
                        if truth(bool(not truth(fpop))) then
                            s.grind_fpop_t = 0
                        end
                        if truth(bool(not truth(bpop))) then
                            s.grind_bpop_t = 0
                        end
                        release_back = trunc((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth((if truth(c.fine_grinds) then 1 else bool(truth(c.split_grinds)))))) then 1 else bool(truth(fine_release_back)))) then bool(truth(c.pop_on_release)) else 0)) then bool(truth(s.grind_prev)) else 0)) then bool(truth(s.grind_bpop)) else 0)) then bool(truth(s.d_back_full)) else 0)) then bool(truth(bool(bm < off))) else 0))
                        release_front = trunc((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth((if truth(c.fine_grinds) then 1 else bool(truth(c.split_grinds)))))) then 1 else bool(truth(fine_release_front)))) then bool(truth(c.pop_on_release)) else 0)) then bool(truth(s.grind_prev)) else 0)) then bool(truth(s.grind_fpop)) else 0)) then bool(truth(s.d_front_full)) else 0)) then bool(truth(bool(fm < off))) else 0))
                        if truth((if truth((if truth(bool(fm < off)) then bool(truth(bool(bm < off))) else 0)) then bool(truth(bool(not truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70))) else 0))))) else 0)) then
                            do
                                s.grind_pop = trunc(0)
                                if truth(bool(not truth((if truth((if truth((if truth((if truth(c.xl_layout) then bool(truth(c.goofy)) else 0)) then bool(truth(c.one_foot_flips)) else 0)) then bool(truth(bool(s.rail_scoop_exit == 2))) else 0)) then bool(truth(bool(((rail_bx * rail_bx) + (rail_by * rail_by)) > .0009))) else 0)))) then
                                    s.rail_scoop_exit = trunc(0)
                                end
                            end
                        end
                        local held_back = trunc((if truth((if truth((if truth((if truth((if truth(s.grind_prev) then bool(truth(bpop)) else 0)) then bool(truth(fflick)) else 0)) then bool(truth(bool(not truth(s.grind_fflick)))) else 0)) then bool(truth(bool(s.grind_bpop_t > 0))) else 0)) then bool(truth(bool((now - s.grind_bpop_t) >= 70.0))) else 0))
                        local held_front = trunc((if truth((if truth((if truth((if truth((if truth(s.grind_prev) then bool(truth(fpop)) else 0)) then bool(truth(bflick)) else 0)) then bool(truth(bool(not truth(s.grind_bflick)))) else 0)) then bool(truth(bool(s.grind_fpop_t > 0))) else 0)) then bool(truth(bool((now - s.grind_fpop_t) >= 70.0))) else 0))
                        if truth((if truth((if truth(s.grind_prev) then bool(truth(bool(not truth(s.grind_pop)))) else 0)) then bool(truth((if truth((if truth((if truth((if truth((if truth((if truth(scoop_foot) then 1 else bool(truth(held_back)))) then 1 else bool(truth(held_front)))) then 1 else bool(truth((if truth((if truth(bpop) then bool(truth(fflick)) else 0)) then bool(truth(bool(not truth(s.grind_fflick)))) else 0))))) then 1 else bool(truth((if truth((if truth(fpop) then bool(truth(bflick)) else 0)) then bool(truth(bool(not truth(s.grind_bflick)))) else 0))))) then 1 else bool(truth(release_back)))) then 1 else bool(truth(release_front))))) else 0)) then
                            do
                                s.grind_pop = trunc(1)
                                s.d_back_on = trunc(0)
                                s.d_front_on = trunc(s.d_back_on)
                                s.d_back_spent = trunc(0)
                                s.d_front_spent = trunc(s.d_back_spent)
                                s.d_back_ignored = trunc(0)
                                s.d_front_ignored = trunc(s.d_back_ignored)
                                s.d_back_carry = trunc(0)
                                s.d_front_carry = trunc(s.d_back_carry)
                                s.d_land_hold = trunc(s.d_front_carry)
                                s.d_windup = trunc(0)
                                if truth(scoop_foot) then
                                    do
                                        local k = trunc(bool(scoop_foot == 2))
                                        if truth(k) then
                                            do
                                                s.d_back_on = trunc(1)
                                                s.d_back_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                                                s.pf_t[1] = s.rail_scoop_turn
                                            end
                                        else
                                            do
                                                s.d_front_on = trunc(1)
                                                s.d_front_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                                                s.pf_t[0] = s.rail_scoop_turn
                                            end
                                        end
                                        s.lone_side[k] = trunc(0)
                                        sc_lone_start(s, k, s.rail_scoop_x, s.rail_scoop_y)
                                        s.last_name = (if truth(k) then "back-foot rail scoop" else "front-foot rail scoop")
                                        s.fired = trunc((s.fired + 1))
                                    end
                                else
                                    if truth((if truth(held_back) then 1 else bool(truth((if truth(bpop) then bool(truth(fflick)) else 0))))) then
                                        do
                                            s.d_back_on = trunc(1)
                                            s.d_back_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                                            s.pf_t[1] = s.grind_bpop_t
                                        end
                                    else
                                        if truth((if truth(held_front) then 1 else bool(truth((if truth(fpop) then bool(truth(bflick)) else 0))))) then
                                            do
                                                s.d_front_on = trunc(1)
                                                s.d_front_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                                                s.pf_t[0] = s.grind_fpop_t
                                            end
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
                if truth((if truth(release_back) then 1 else bool(truth(release_front)))) then
                    do
                        s.rp_t = now
                        s.rp_y = f32((if truth(release_back) then 1.0 else (-1.0)))
                        s.rp_two_foot = trunc(0)
                        s.d_front_full = trunc(0)
                        s.d_back_full = trunc(s.d_front_full)
                        s.last_name = (if truth(release_back) then "ollie (rail release)" else "nollie (rail release)")
                        s.fired = trunc((s.fired + 1))
                    end
                end
                s.grind_prev = trunc(s.grind)
                s.grind_fpop = trunc(fpop)
                s.grind_bpop = trunc(bpop)
                s.grind_fflick = trunc(fflick)
                s.grind_bflick = trunc(bflick)
                if truth((if truth((if truth((if truth((if truth((if truth(c.xl_layout) then bool(truth(c.goofy)) else 0)) then bool(truth(c.one_foot_flips)) else 0)) then bool(truth(s.grind)) else 0)) then bool(truth(bool(s.rail_scoop_exit == 2))) else 0)) then bool(truth(bool(fm < off))) else 0)) then
                    do
                        s.d_front_full = trunc(0)
                        s.d_back_full = trunc(s.d_front_full)
                        s.rp_t = 0
                        s.d_t = now
                        out_x.set(f32(rail_bx))
                        out_y.set(f32(rail_by))
                        return
                    end
                end
                if truth((if truth(s.grind) then bool(truth(bool(not truth(s.grind_pop)))) else 0)) then
                    do
                        local x = f32((fx - bx))
                        local y = f32((sc_abs(fy) - sc_abs(by)))
                        local gain = f32((if truth(bool(c.grind_sensitivity > 0)) then c.grind_sensitivity else 1.0))
                        x = f32((x * gain))
                        y = f32((y * gain))
                        if truth(bool(s.rail_scoop_candidate == 2)) then
                            do
                                local native_rear = trunc((if truth((if truth(c.xl_layout) then bool(truth(c.goofy)) else 0)) then bool(truth(c.one_foot_flips)) else 0))
                                x = f32((if truth(native_rear) then rail_bx else bx))
                                y = f32((if truth(native_rear) then rail_by else by))
                            end
                        else
                            if truth(c.split_grinds) then
                                do
                                    sc_split_rail_position(c, s, fx, fy, bx, by, gain, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                    s.split_grind_active = trunc(1)
                                end
                            else
                                if truth(c.fine_grinds) then
                                    sc_fine_board(fx, fy, bx, by, gain, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                end
                            end
                        end
                        local m = f32(sc_sqrt(((x * x) + (y * y))))
                        if truth((if truth(bool(m > 1)) then bool(truth(bool(not truth((if truth((if truth((if truth(c.xl_layout) then bool(truth(c.goofy)) else 0)) then bool(truth(c.one_foot_flips)) else 0)) then bool(truth(bool(s.rail_scoop_candidate == 2))) else 0))))) else 0)) then
                            do
                                x = f32((x / m))
                                y = f32((y / m))
                            end
                        end
                        s.d_front_full = trunc((if truth(fpop) then bool(truth(bool(fm >= .8))) else 0))
                        s.d_back_full = trunc((if truth(bpop) then bool(truth(bool(bm >= .8))) else 0))
                        s.d_back_on = trunc(0)
                        s.d_front_on = trunc(s.d_back_on)
                        s.d_back_spent = trunc(0)
                        s.d_front_spent = trunc(s.d_back_spent)
                        s.d_back_ignored = trunc(0)
                        s.d_front_ignored = trunc(s.d_back_ignored)
                        s.d_back_carry = trunc(0)
                        s.d_front_carry = trunc(s.d_back_carry)
                        s.d_land_hold = trunc(s.d_front_carry)
                        s.hold_foot = trunc(0)
                        s.d_took_off = trunc(s.hold_foot)
                        s.d_was_air = trunc(s.d_took_off)
                        s.rp_t = 0
                        s.d_windup = trunc(s.rp_t)
                        s.d_t = now
                        out_x.set(f32(x))
                        out_y.set(f32(y))
                        return
                    end
                end
            end
        end
        if truth(airborne) then
            do
                local pivot_threshold = f32((if truth((if truth(c.split_grinds) then bool(truth(c.air_controls)) else 0)) then 8.0 else 20.0))
                if truth(s.hold_foot) then
                    do
                        local hx = f32((if truth(bool(s.hold_foot == 1)) then fx else bx))
                        local hy = f32((if truth(bool(s.hold_foot == 1)) then fy else by))
                        local hm = f32(sc_sqrt(((hx * hx) + (hy * hy))))
                        local ta = f32((if truth(bool(s.hold_foot == 1)) then s.tk_fa else s.tk_ba))
                        local other_m = f32((if truth(bool(s.hold_foot == 1)) then bm else fm))
                        local other_a = f32((if truth(bool(s.hold_foot == 1)) then sc_angle(bx, by) else sc_angle(fx, fy)))
                        local other_ta = f32((if truth(bool(s.hold_foot == 1)) then s.tk_ba else s.tk_fa))
                        local other_on = trunc((if truth(bool(s.hold_foot == 1)) then s.d_back_on else s.d_front_on))
                        local pivot = trunc((if truth(bool(other_m >= on)) then bool(truth((if truth(bool(not truth(other_on))) then 1 else bool(truth(bool(sc_abs(sc_wrap((other_a - other_ta))) >= pivot_threshold)))))) else 0))
                        local other_x = f32((if truth(bool(s.hold_foot == 1)) then bx else fx))
                        if truth((if truth((if truth((if truth(c.split_grinds) then bool(truth(c.air_controls)) else 0)) then bool(truth(bool(other_m >= .65))) else 0)) then bool(truth(bool(sc_abs(other_x) >= (.65 * other_m)))) else 0)) then
                            do
                                pivot = trunc(1)
                                s.grind_setup_override = trunc(1)
                            end
                        end
                        if truth((if truth((if truth(bool(not truth(pivot))) then bool(truth(s.hold_scoop)) else 0)) then bool(truth((if truth(bool(s.hold_scoop == 2)) then 1 else bool(truth(bool(hm < .7)))))) else 0)) then
                            do
                                s.hold_scoop = trunc(2)
                                if truth(bool(hm < off)) then
                                    s.hold_scoop = trunc(0)
                                    s.hold_foot = trunc(s.hold_scoop)
                                end
                                s.d_was_air = trunc(1)
                                s.d_t = now
                                out_y.set(f32(0))
                                out_x.set(f32(out_y.get()))
                                return
                            end
                        end
                        if truth((if truth((if truth(bool(not truth(pivot))) then bool(truth(bool(hm >= 0.7))) else 0)) then bool(truth(bool(sc_abs(sc_wrap((sc_angle(hx, hy) - ta))) < pivot_threshold))) else 0)) then
                            do
                                s.d_was_air = trunc(1)
                                s.d_t = now
                                out_x.set(f32(s.hold_x))
                                out_y.set(f32(s.hold_y))
                                return
                            end
                        end
                        if truth((if truth(c.split_grinds) then bool(truth(c.air_controls)) else 0)) then
                            s.grind_setup_override = trunc(1)
                        end
                        s.hold_scoop = trunc(0)
                        s.hold_foot = trunc(s.hold_scoop)
                    end
                end
                if truth((if truth((if truth((if truth(s.d_front_on) then bool(truth(s.d_front_spent)) else 0)) then bool(truth(bool(fm >= off))) else 0)) then bool(truth(bool(sc_abs(sc_wrap((sc_angle(fx, fy) - s.tk_fa))) >= pivot_threshold))) else 0)) then
                    s.d_front_spent = trunc(0)
                end
                if truth((if truth((if truth((if truth(s.d_back_on) then bool(truth(s.d_back_spent)) else 0)) then bool(truth(bool(bm >= off))) else 0)) then bool(truth(bool(sc_abs(sc_wrap((sc_angle(bx, by) - s.tk_ba))) >= pivot_threshold))) else 0)) then
                    s.d_back_spent = trunc(0)
                end
                local fl = trunc((if truth(s.d_front_on) then bool(not truth(s.d_front_spent)) else bool(fm >= off)))
                local bl = trunc((if truth(s.d_back_on) then bool(not truth(s.d_back_spent)) else bool(bm >= off)))
                if truth((if truth((if truth(fl) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(bool(fm >= on))) else 0)) then
                    do
                        s.d_front_on = trunc(1)
                        s.d_front_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                    end
                end
                if truth((if truth((if truth(bl) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(bm >= on))) else 0)) then
                    do
                        s.d_back_on = trunc(1)
                        s.d_back_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                    end
                end
                local x = f32(0)
                local y = f32(0)
                if truth(c.air_controls) then
                    do
                        local fh = trunc((if truth((if truth((if truth((if truth(airborne) then bool(truth(bool(not truth(fl)))) else 0)) then bool(truth(s.d_front_on)) else 0)) then bool(truth(s.d_front_spent)) else 0)) then bool(truth(bool(fm >= off))) else 0))
                        local bh = trunc((if truth((if truth((if truth((if truth(airborne) then bool(truth(bool(not truth(bl)))) else 0)) then bool(truth(s.d_back_on)) else 0)) then bool(truth(s.d_back_spent)) else 0)) then bool(truth(bool(bm >= off))) else 0))
                        local fcr = trunc((if truth((if truth(fh) then bool(truth(bool(fm >= 0.85))) else 0)) then bool(truth(bool(sc_abs(fx) < (0.5 * fm)))) else 0))
                        local bcr = trunc((if truth((if truth(bh) then bool(truth(bool(bm >= 0.85))) else 0)) then bool(truth(bool(sc_abs(bx) < (0.5 * bm)))) else 0))
                        if truth(fl) then
                            do
                                x = f32((x + fx))
                                y = f32((y + sc_abs(fy)))
                            end
                        else
                            if truth(fh) then
                                do
                                    x = f32((x + fx))
                                    if truth(bool(not truth(fcr))) then
                                        y = f32((y + sc_abs(fy)))
                                    end
                                end
                            end
                        end
                        if truth(bl) then
                            do
                                x = f32((x - bx))
                                y = f32((y - sc_abs(by)))
                            end
                        else
                            if truth(bh) then
                                do
                                    x = f32((x - bx))
                                    if truth(bool(not truth(bcr))) then
                                        y = f32((y - sc_abs(by)))
                                    end
                                end
                            end
                        end
                    end
                else
                    do
                        if truth((if truth((if truth(fl) then bool(truth(bool(fy > 0))) else 0)) then bool(truth((if truth(bool(not truth(bl))) then 1 else bool(truth(bool(fm >= bm)))))) else 0)) then
                            y = f32(fy)
                        else
                            if truth((if truth(bl) then bool(truth(bool(by < 0))) else 0)) then
                                y = f32(by)
                            end
                        end
                    end
                end
                if truth(c.air_controls) then
                    do
                        local gain = f32((if truth(bool(c.grind_sensitivity > 0)) then c.grind_sensitivity else 1.0))
                        if truth(c.split_grinds) then
                            do
                                sc_split_position(c, fx, fy, bx, by, gain, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                s.split_grind_active = trunc(1)
                            end
                        else
                            if truth(c.fine_grinds) then
                                sc_fine_board(fx, fy, bx, by, gain, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                            else
                                x = f32((x * gain))
                            end
                        end
                    end
                end
                local m = f32(sc_sqrt(((x * x) + (y * y))))
                if truth(bool(m > 1.0)) then
                    do
                        x = f32((x / m))
                        y = f32((y / m))
                    end
                end
                local rear_load = f32((if truth(bool(by < 0)) then (-by) else 0))
                local front_load = f32((if truth(bool(fy > 0)) then fy else 0))
                local cooperative_front = trunc((if truth((if truth(bool((fx * bx) < 0)) then bool(truth(bool(sc_abs(fx) >= sc_abs(fy)))) else 0)) then bool(truth(bool(front_load <= rear_load))) else 0))
                if truth((if truth((if truth((if truth((if truth((if truth((if truth(c.split_grinds) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then bool(truth(c.air_controls)) else 0)) then bool(truth((if truth(bool(fm < off)) then 1 else bool(truth(cooperative_front))))) else 0)) then bool(truth(bool(bm >= on))) else 0)) then bool(truth(bool(sc_abs(bx) >= .5))) else 0)) then bool(truth(bool(sc_abs(bx) >= sc_abs(by)))) else 0)) then
                    do
                        s.entry_back_active = trunc(1)
                        s.entry_side_back = trunc((if truth(bool(fm < off)) then bool(truth(bool(sc_abs(by) <= (.35 * sc_abs(bx))))) else 0))
                        s.entry_back_x = f32(bx)
                        s.entry_back_y = f32((if truth(bool(by < 0)) then by else 0))
                        if truth((if truth((if truth(c.native_grind_entry) then bool(truth(s.entry_rail_target)) else 0)) then bool(truth(bool(entry_lateral > .5))) else 0)) then
                            do
                                local lateral = f32((entry_lateral / .95))
                                if truth(bool(lateral > 1)) then
                                    lateral = f32(1)
                                end
                                local turn = f32(((lateral - .5) * 2))
                                if truth(bool(turn > 1)) then
                                    turn = f32(1)
                                end
                                y = f32((y * (1 - turn)))
                                s.entry_rotation_active = trunc(1)
                            end
                        end
                    end
                end
                if truth((if truth((if truth(c.xl_layout) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(s.board_frame_mirror)) else 0)) then
                    x = f32((-x))
                end
                if truth((if truth(c.split_grinds) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then
                    do
                        if truth((if truth((if truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then bool(truth(bool(sc_abs(fx) >= (.25 * fm)))) else 0)) then bool(truth(bool(bm < off))) else 0)) then
                            s.split_air_setup = trunc(1)
                        else
                            if truth((if truth((if truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then bool(truth(bool(sc_abs(bx) >= (.25 * bm)))) else 0)) then bool(truth(bool(fm < off))) else 0)) then
                                s.split_air_setup = trunc(2)
                            end
                        end
                    end
                end
                s.d_was_air = trunc(1)
                s.d_t = now
                out_x.set(f32(x))
                out_y.set(f32(y))
                return
            end
        end
        if truth((if truth(s.d_was_air) then bool(truth(c.air_controls)) else 0)) then
            do
                local front_setup = trunc((if truth((if truth((if truth((if truth((if truth(bool(previous_split_setup == 1)) then bool(truth(c.split_grinds)) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then bool(truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then bool(truth(bool(sc_abs(fx) >= (.25 * fm)))) else 0)) then bool(truth(bool(bm < off))) else 0))
                local back_setup = trunc((if truth((if truth((if truth((if truth((if truth(bool(previous_split_setup == 2)) then bool(truth(c.split_grinds)) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then bool(truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then bool(truth(bool(sc_abs(bx) >= (.25 * bm)))) else 0)) then bool(truth(bool(fm < off))) else 0))
                s.split_landing_setup = trunc((if truth(front_setup) then 1 else bool(truth(back_setup))))
                s.d_front_carry = trunc((if truth(bool(fm >= off)) then bool(truth((if truth(bool(fm < (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then 1 else bool(truth(front_setup))))) else 0))
                s.d_back_carry = trunc((if truth(bool(bm >= off)) then bool(truth((if truth(bool(bm < (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85))) then 1 else bool(truth(back_setup))))) else 0))
                s.d_land_hold = trunc((if truth(s.d_front_carry) then 1 else bool(truth(s.d_back_carry))))
            end
        end
        s.d_was_air = trunc(0)
        if truth(s.d_land_hold) then
            do
                if truth((if truth(bool(fm < off)) then 1 else bool(truth((if truth((if truth(front_pop_dir) then bool(truth(bool(fm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then bool(truth(bool(not truth((if truth(s.split_landing_setup) then bool(truth(bool(sc_abs(fx) >= (.25 * fm)))) else 0))))) else 0))))) then
                    s.d_front_carry = trunc(0)
                end
                if truth((if truth(bool(bm < off)) then 1 else bool(truth((if truth((if truth(back_pop_dir) then bool(truth(bool(bm >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then bool(truth(bool(not truth((if truth(s.split_landing_setup) then bool(truth(bool(sc_abs(bx) >= (.25 * bm)))) else 0))))) else 0))))) then
                    s.d_back_carry = trunc(0)
                end
                local new_foot = trunc((if truth((if truth(bool(not truth(s.d_front_carry))) then bool(truth(bool(fm >= on))) else 0)) then 1 else bool(truth((if truth(bool(not truth(s.d_back_carry))) then bool(truth(bool(bm >= on))) else 0)))))
                if truth((if truth(bool(not truth(new_foot))) then bool(truth((if truth(s.d_front_carry) then 1 else bool(truth(s.d_back_carry))))) else 0)) then
                    do
                        local x = f32(((if truth(s.d_front_carry) then fx else 0) - (if truth(s.d_back_carry) then bx else 0)))
                        local y = f32(((if truth(s.d_front_carry) then sc_abs(fy) else 0) - (if truth(s.d_back_carry) then sc_abs(by) else 0)))
                        local m = f32(sc_sqrt(((x * x) + (y * y))))
                        if truth(bool(m > 1)) then
                            do
                                x = f32((x / m))
                                y = f32((y / m))
                            end
                        end
                        if truth(s.split_landing_setup) then
                            do
                                local gain = f32((if truth(bool(c.grind_sensitivity > 0)) then c.grind_sensitivity else 1.0))
                                sc_split_position(c, fx, fy, bx, by, gain, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                if truth((if truth(c.xl_layout) then bool(truth(s.board_frame_mirror)) else 0)) then
                                    x = f32((-x))
                                end
                                s.split_grind_active = trunc(1)
                            end
                        end
                        s.d_t = now
                        out_x.set(f32(x))
                        out_y.set(f32(y))
                        return
                    end
                end
                s.d_land_hold = trunc(0)
                s.split_landing_setup = trunc(0)
            end
        end
        if truth((if truth((if truth((if truth((if truth((if truth(s.grind) then bool(truth(c.air_controls)) else 0)) then bool(truth(bool(not truth(s.grind_pop)))) else 0)) then bool(truth(bool(s.phase == 0))) else 0)) then bool(truth(bool(not truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0))))) else 0)) then bool(truth(bool(not truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0))))) else 0)) then
            do
                local fpop = trunc((if truth((if truth(bool(fy > 0)) then bool(truth(bool(fm >= 0.6))) else 0)) then bool(truth(bool(sc_abs(fx) < (0.7 * fm)))) else 0))
                local bpop = trunc((if truth((if truth(bool(by < 0)) then bool(truth(bool(bm >= 0.6))) else 0)) then bool(truth(bool(sc_abs(bx) < (0.7 * bm)))) else 0))
                if truth((if truth(bool(not truth(fpop))) then bool(truth(bool(not truth(bpop)))) else 0)) then
                    do
                        local x = f32(0)
                        local y = f32(0)
                        if truth(bool(fm >= off)) then
                            do
                                x = f32((x + fx))
                                y = f32((y + sc_abs(fy)))
                            end
                        end
                        if truth(bool(bm >= off)) then
                            do
                                x = f32((x - bx))
                                y = f32((y - sc_abs(by)))
                            end
                        end
                        local m = f32(sc_sqrt(((x * x) + (y * y))))
                        if truth(bool(m > 1.0)) then
                            do
                                x = f32((x / m))
                                y = f32((y / m))
                            end
                        end
                        s.d_t = now
                        out_x.set(f32(x))
                        out_y.set(f32(y))
                        return
                    end
                end
            end
        end
        if truth((if truth((if truth((if truth(bool(s.split_pop_t > 0)) then bool(truth(s.game_known)) else 0)) then bool(truth(bool(s.game_crouch == 1))) else 0)) then bool(truth((if truth((if truth((if truth(bool(fm < off)) then bool(truth(bool(bm >= off))) else 0)) then bool(truth(bool(sc_abs(by) <= (.35 * sc_abs(bx))))) else 0)) then 1 else bool(truth((if truth((if truth(bool(bm < off)) then bool(truth(bool(fm >= off))) else 0)) then bool(truth(bool(sc_abs(fy) <= (.35 * sc_abs(fx))))) else 0)))))) else 0)) then
            do
                s.d_windup = trunc(0)
                s.d_t = now
                out_x.set(f32(0))
                out_y.set(f32((if truth((if truth((if truth(c.pop_on_release) then bool(truth(bool(s.rp_t > 0))) else 0)) then bool(truth(bool((now - s.rp_t) < 70))) else 0)) then s.rp_y else 0)))
                return
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(fm >= on))) else 0)) then bool(truth(bool(not truth(s.d_front_ignored)))) else 0)) then
            do
                if truth((if truth((if truth((if truth(s.d_back_on) then 1 else bool(truth(s.d_front_flick)))) then 1 else bool(truth(front_pop_dir)))) then 1 else bool(truth(bool(f_off <= side_lim))))) then
                    do
                        s.d_front_on = trunc(1)
                        s.d_front_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                        s.pf_t[0] = now
                        s.lone_side[0] = trunc((if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(f_off >= 55.0))) else 0)) then (if truth(bool(fx > 0)) then 1 else (-1)) else 0))
                        sc_lone_start(s, 0, fx, fy)
                        if truth((if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(not truth(s.d_front_flick)))) else 0)) then bool(truth(bool(not truth(front_pop_dir)))) else 0)) then
                            do
                                s.d_windup = trunc(1)
                                s.d_windup_t = now
                                s.d_log = trunc(1)
                                s.d_log_a = f32(fa)
                                s.d_log_b = f32(0)
                            end
                        end
                    end
                else
                    s.d_front_ignored = trunc(1)
                end
            end
        end
        if truth((if truth((if truth(bool(not truth(s.d_back_on))) then bool(truth(bool(bm >= on))) else 0)) then bool(truth(bool(not truth(s.d_back_ignored)))) else 0)) then
            do
                if truth((if truth((if truth((if truth(s.d_front_on) then 1 else bool(truth(s.d_back_flick)))) then 1 else bool(truth(back_pop_dir)))) then 1 else bool(truth(bool(b_off <= side_lim))))) then
                    do
                        s.d_back_on = trunc(1)
                        s.d_back_seq = trunc((function() local old = s.d_seq; s.d_seq = trunc((s.d_seq + 1)); return s.d_seq end)())
                        s.pf_t[1] = now
                        s.lone_side[1] = trunc((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(b_off >= 55.0))) else 0)) then (if truth(bool(bx > 0)) then 1 else (-1)) else 0))
                        sc_lone_start(s, 1, bx, by)
                        if truth((if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_flick)))) else 0)) then bool(truth(bool(not truth(back_pop_dir)))) else 0)) then
                            do
                                s.d_windup = trunc(1)
                                s.d_windup_t = now
                                s.d_log = trunc(1)
                                s.d_log_a = f32(ba)
                                s.d_log_b = f32(1)
                            end
                        end
                    end
                else
                    s.d_back_ignored = trunc(1)
                end
            end
        end
        local fl = trunc((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0))
        local bl = trunc((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0))
        s.pf_hold = trunc(0)
        if truth((if truth(fl) then bool(truth(bl)) else 0)) then
            do
                local front_first = trunc(bool(s.d_front_seq < s.d_back_seq))
                local t0 = s.pf_t[(if truth(front_first) then 0 else 1)]
                if truth((if truth((if truth(bool(not truth(s.d_windup))) then bool(truth(bool(t0 > 0))) else 0)) then bool(truth(bool((now - t0) < 70.0))) else 0)) then
                    s.pf_hold = trunc((if truth(front_first) then 1 else (-1)))
                end
            end
        end
        if truth((if truth((if truth(fl) then bool(truth(bl)) else 0)) then bool(truth(bool(not truth(s.pf_hold)))) else 0)) then
            do
                if truth((if truth((if truth(c.split_grinds) then bool(truth(c.air_controls)) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then
                    s.split_pop_t = now
                end
                if truth(bool(s.d_front_seq > s.d_back_seq)) then
                    do
                        s.d_back_spent = trunc(1)
                        s.d_front_flick = trunc(1)
                        bl = trunc(0)
                    end
                else
                    do
                        s.d_front_spent = trunc(1)
                        s.d_back_flick = trunc(1)
                        fl = trunc(0)
                    end
                end
                s.d_log = trunc(2)
                s.d_log_a = f32((if truth(fl) then fa else ba))
                s.d_log_b = f32(0)
            end
        end
        if truth((if truth((if truth(c.pop_on_release) then bool(truth(fl)) else 0)) then bool(truth(bl)) else 0)) then
            s.rp_two_foot = trunc(1)
        end
        if truth((if truth((if truth(c.pop_on_release) then bool(truth(bool(not truth(s.rp_two_foot)))) else 0)) then bool(truth(bool(not truth(s.rail_scoop_exit)))) else 0)) then
            do
                if truth((if truth((if truth(s.d_windup) then 1 else bool(truth(s.d_front_on)))) then 1 else bool(truth(s.d_back_spent)))) then
                    do
                        s.d_back_full = trunc(0)
                        s.d_back_peak = f32(0)
                    end
                else
                    if truth((if truth((if truth(bl) then bool(truth(bool(bm >= 0.8))) else 0)) then bool(truth(back_pop_dir)) else 0)) then
                        do
                            s.d_back_full = trunc(1)
                            if truth(bool(bm > s.d_back_peak)) then
                                s.d_back_peak = f32(bm)
                            end
                        end
                    end
                end
                if truth((if truth((if truth(s.d_windup) then 1 else bool(truth(s.d_back_on)))) then 1 else bool(truth(s.d_front_spent)))) then
                    do
                        s.d_front_full = trunc(0)
                        s.d_front_peak = f32(0)
                    end
                else
                    if truth((if truth((if truth(fl) then bool(truth(bool(fm >= 0.8))) else 0)) then bool(truth(front_pop_dir)) else 0)) then
                        do
                            s.d_front_full = trunc(1)
                            if truth(bool(fm > s.d_front_peak)) then
                                s.d_front_peak = f32(fm)
                            end
                        end
                    end
                end
                local rel = trunc(0)
                if truth((if truth((if truth((if truth(s.d_back_full) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(bool(bm < (s.d_back_peak - 0.12)))) else 0)) then bool(truth((if truth(bool(by > ((-0.85) * s.d_back_peak))) then 1 else bool(truth(bool(bm < 0.75)))))) else 0)) then
                    rel = trunc(1)
                else
                    if truth((if truth((if truth((if truth(s.d_front_full) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(fm < (s.d_front_peak - 0.12)))) else 0)) then bool(truth((if truth(bool(fy < (0.85 * s.d_front_peak))) then 1 else bool(truth(bool(fm < 0.75)))))) else 0)) then
                        rel = trunc(2)
                    end
                end
                if truth(rel) then
                    do
                        if truth(bool(rel == 1)) then
                            do
                                if truth(s.d_back_on) then
                                    s.d_back_spent = trunc(1)
                                end
                                s.d_back_full = trunc(0)
                                s.d_back_peak = f32(0)
                            end
                        else
                            do
                                if truth(s.d_front_on) then
                                    s.d_front_spent = trunc(1)
                                end
                                s.d_front_full = trunc(0)
                                s.d_front_peak = f32(0)
                            end
                        end
                        s.rp_t = now
                        s.rp_y = f32((if truth(bool(rel == 1)) then 1.0 else (-1.0)))
                        if truth((if truth((if truth(c.split_grinds) then bool(truth(c.air_controls)) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then
                            s.split_pop_t = now
                        end
                        s.last_name = (if truth(bool(rel == 1)) then "ollie (let go)" else "nollie (let go)")
                        s.fired = trunc((s.fired + 1))
                    end
                end
                if truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70.0))) else 0)) then
                    do
                        s.d_t = now
                        out_x.set(f32(0))
                        out_y.set(f32(s.rp_y))
                        return
                    end
                end
                s.rp_t = 0
            end
        end
        do
            local k = trunc(0)
            while truth(bool(k < 2)) do
                do
                    local live = trunc((if truth(k) then (if truth((if truth(bl) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(bool(not truth(s.d_back_flick)))) else 0) else (if truth((if truth(fl) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(not truth(s.d_front_flick)))) else 0)))
                    local px = f32((if truth(k) then bx else fx))
                    local py = f32((if truth(k) then by else fy))
                    local pm = f32((if truth(k) then bm else fm))
                    local own = f32((if truth(k) then (-py) else py))
                    if truth((if truth(c.one_foot_flips) then 1 else bool(truth(bool(not truth(live)))))) then
                        do
                            s.lp_blk[k] = trunc(0)
                            s.lp_dip[k] = trunc(0)
                            s.lp_mag[k] = f32(0)
                            k = trunc((k + 1))
                            continue
                        end
                    end
                    local a = f32(sc_angle(px, py))
                    if truth((if truth(bool(own > 0.3)) then bool(truth(bool(pm >= 0.5))) else 0)) then
                        do
                            s.lp_dip[k] = trunc(0)
                            s.lp_blk[k] = trunc(0)
                        end
                    else
                        if truth(bool(pm < 0.5)) then
                            s.lp_dip[k] = trunc(1)
                        else
                            if truth((if truth(bool(s.lp_mag[k] >= 0.5)) then bool(truth(bool(sc_abs(sc_wrap((a - s.lp_ang[k]))) > 100.0))) else 0)) then
                                s.lp_dip[k] = trunc(1)
                            end
                        end
                    end
                    if truth((if truth(bool(own < (-0.15))) then bool(truth(s.lp_dip[k])) else 0)) then
                        s.lp_blk[k] = trunc(1)
                    end
                    s.lp_ang[k] = f32(a)
                    s.lp_mag[k] = f32(pm)
                end
                k = trunc((k + 1))
            end
        end
        local x = f32(0)
        local y = f32(0)
        if truth(s.pf_hold) then
            do
                x = f32(0)
                y = f32((if truth(bool(s.pf_hold > 0)) then 1.0 else (-1.0)))
            end
        else
            if truth((if truth(fl) then bool(truth(bool(not truth(s.lp_blk[0])))) else 0)) then
                do
                    x = f32(fx)
                    y = f32(fy)
                    if truth((if truth((if truth(bool(not truth(c.one_foot_flips))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(not truth(s.d_front_flick)))) else 0)) then
                        sc_lone_clamp(s, 0, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                    end
                end
            else
                if truth((if truth(bl) then bool(truth(bool(not truth(s.lp_blk[1])))) else 0)) then
                    do
                        x = f32(bx)
                        y = f32(by)
                        if truth((if truth((if truth(bool(not truth(c.one_foot_flips))) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(bool(not truth(s.d_back_flick)))) else 0)) then
                            sc_lone_clamp(s, 1, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                        end
                    end
                else
                    if truth((if truth(fl) then 1 else bool(truth(bl)))) then
                        do
                            x = f32(0)
                            y = f32(0)
                        end
                    else
                        if truth((if truth(bool(not truth(s.d_front_on))) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then
                            do
                                local front_lean = trunc((if truth(bool(fy > 0)) then bool(truth(bool(not truth(s.d_front_ignored)))) else 0))
                                local back_lean = trunc((if truth(bool(by < 0)) then bool(truth(bool(not truth(s.d_back_ignored)))) else 0))
                                if truth((if truth(front_lean) then bool(truth((if truth(bool(not truth(back_lean))) then 1 else bool(truth(bool(fm >= bm)))))) else 0)) then
                                    do
                                        x = f32(fx)
                                        y = f32(fy)
                                    end
                                else
                                    if truth(back_lean) then
                                        do
                                            x = f32(bx)
                                            y = f32(by)
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
        if truth((if truth(bool(not truth(s.pf_hold))) then bool(truth((if truth((if truth(fl) then bool(truth((if truth(s.d_back_spent) then 1 else bool(truth(s.d_front_flick))))) else 0)) then 1 else bool(truth((if truth(bl) then bool(truth((if truth(s.d_front_spent) then 1 else bool(truth(s.d_back_flick))))) else 0)))))) else 0)) then
            do
                local sgn = f32((if truth(fl) then 1.0 else (-1.0)))
                local own = f32((y * sgn))
                local ax = f32(sc_abs(x))
                local m = f32(sc_sqrt(((x * x) + (y * y))))
                if truth((if truth((if truth(bool(m >= on)) then bool(truth(bool(ax > 0.2))) else 0)) then bool(truth(bool(own < (0.7 * ax)))) else 0)) then
                    do
                        x = f32(((if truth(bool(x > 0)) then 0.707 else (-0.707)) * m))
                        y = f32(((sgn * 0.707) * m))
                    end
                end
            end
        end
        if truth((if truth((if truth((if truth(s.d_windup) then bool(truth(bool(not truth((if truth(s.d_front_on) then bool(truth(s.d_back_on)) else 0))))) else 0)) then bool(truth(c.powerslide)) else 0)) then bool(truth(bool((now - s.d_windup_t) < 40.0))) else 0)) then
            do
                x = f32(0)
                y = f32(0)
            end
        end
        local m = f32(sc_sqrt(((x * x) + (y * y))))
        if truth(bool(m > 1.0)) then
            do
                x = f32((x / m))
                y = f32((y / m))
            end
        end
        s.d_t = now
        out_x.set(f32(x))
        out_y.set(f32(y))
    end
end
local function sc_pump_output(s, out, now)
    out = copy(out)
    do
        if truth(s.pump_r3) then
            do
                local u = f32(f32(((now - s.pump_press_t) / 45.0)))
                if truth(bool(u > 1)) then
                    u = f32(1)
                end
                if truth(bool(u < 0)) then
                    u = f32(0)
                end
                s.pump_level = f32((0.05 + (0.95 * u)))
                local pump_rt = trunc(trunc((255 * s.pump_level)))
                if truth(bool(pump_rt > out.rt)) then
                    out.rt = trunc(pump_rt)
                end
                out.ry = trunc(0)
                out.rx = trunc(out.ry)
                out.buttons = trunc(bit32.band(out.buttons, trunc(bit32.bnot(0x0080))))
            end
        else
            if truth(bool(s.pump_release_t > 0)) then
                do
                    local u = f32(f32((((now + 1) - s.pump_release_t) / 50.0)))
                    if truth(bool(u >= 1)) then
                        do
                            s.pump_release_t = 0
                            s.pump_level = f32(0)
                        end
                    else
                        do
                            s.pump_level = f32((s.pump_release_level * (1 - u)))
                            local pump_rt = trunc(trunc((255 * s.pump_level)))
                            if truth(bool(pump_rt > out.rt)) then
                                out.rt = trunc(pump_rt)
                            end
                        end
                    end
                end
            end
        end
        return copy(out)
    end
end
local function sc_translate(c, s, _c_in, now)
    _c_in = copy(_c_in)
    do
        if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(s.grind))) then 1 else bool(truth(s.grind_pop)))) then 1 else bool(truth(bool(not truth(c.enabled)))))) then 1 else bool(truth(bool(not truth(c.pure)))))) then 1 else bool(truth(bool(not truth(c.air_controls)))))) then 1 else bool(truth(bool(not truth(c.split_grinds)))))) then 1 else bool(truth(c.one_foot_flips)))) then
            do
                s.split_rail_back_load = f32(0)
                s.split_rail_front_load = f32(s.split_rail_back_load)
                s.split_rear_turn = trunc(0)
            end
        end
        if truth((if truth((if truth((if truth((if truth(bool(not truth(c.enabled))) then 1 else bool(truth(bool(not truth(c.pure)))))) then 1 else bool(truth(bool(not truth(c.air_controls)))))) then 1 else bool(truth(bool(not truth(c.split_grinds)))))) then 1 else bool(truth(c.one_foot_flips)))) then
            s.split_landing_setup = trunc(0)
            s.split_air_setup = trunc(s.split_landing_setup)
        end
        if truth(bool(not truth(s.d_was_air))) then
            s.split_air_setup = trunc(0)
        end
        if truth((if truth((if truth((if truth(bool(not truth(s.d_land_hold))) then 1 else bool(truth(s.air)))) then 1 else bool(truth(bool(s.takeoff > 0))))) then 1 else bool(truth(s.grind)))) then
            s.split_landing_setup = trunc(0)
        end
        s.entry_side_back = trunc(0)
        s.entry_back_active = trunc(s.entry_side_back)
        s.entry_rotation_active = trunc(0)
        s.split_grind_active = trunc(0)
        s.split_grind_lean = f32(0)
        s.grind_setup_override = trunc(0)
        s.radial_passthrough = trunc(0)
        if truth(bool(not truth(c.enabled))) then
            do
                s.split_pop_t = 0
                s.board_frame_valid = trunc(0)
                s.fine_exit_stage = trunc(0)
                s.fine_back_started = 0
                s.fine_front_started = s.fine_back_started
                return copy(_c_in)
            end
        end
        if truth(bit32.band(_c_in.buttons, 0x000F)) then
            s.dp_t = (now + 1)
        end
        if truth((if truth(bool(s.dp_t > 0)) then bool(truth(bool(((now + 1) - s.dp_t) < 250.0))) else 0)) then
            do
                s.split_rail_back_load = f32(0)
                s.split_rail_front_load = f32(s.split_rail_back_load)
                s.split_rear_turn = trunc(0)
                s.split_landing_setup = trunc(0)
                s.split_air_setup = trunc(s.split_landing_setup)
                s.radial_passthrough = trunc(1)
                s.split_pop_t = 0
                s.pump_r3 = trunc(0)
                s.pump_release_t = 0
                s.pump_level = f32(0)
                s.d_back_on = trunc(0)
                s.d_front_on = trunc(s.d_back_on)
                s.d_back_spent = trunc(0)
                s.d_front_spent = trunc(s.d_back_spent)
                s.phase = trunc(0)
                s.d_back_ignored = trunc(1)
                s.d_front_ignored = trunc(s.d_back_ignored)
                return copy(_c_in)
            end
        end
        local stock_airborne = trunc((if truth((if truth(bool(s.takeoff == 1)) then 1 else bool(truth(s.air)))) then bool(truth(bool(not truth(s.grind)))) else 0))
        if truth(bool(not truth(stock_airborne))) then
            do
                if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(s.stock_ramp_was_air))) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(s.takeoff == 0))) else 0)) then bool(truth(s.stock_ramp_departing)) else 0)) then bool(truth(s.stock_ramp_eligible)) else 0)) then bool(truth(bool(((now + 1) - s.stock_ramp_last_ground) <= 180.0))) else 0)) then
                    do
                    end
                else
                    if truth((if truth((if truth((if truth(s.stock_ramp_was_air) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(bool(s.takeoff ~= 0))))) then 1 else bool(truth(bool(not truth(s.stock_ramp_valid)))))) then
                        do
                            s.stock_ramp_since = 0
                            s.stock_ramp_eligible = trunc(0)
                            s.stock_ramp_curve = trunc(0)
                            s.stock_ramp_history_next = trunc(s.stock_ramp_curve)
                            s.stock_ramp_history_n = trunc(s.stock_ramp_history_next)
                            s.stock_ramp_evidence = 0
                        end
                    else
                        do
                            s.stock_ramp_last_ground = (now + 1)
                            local tilt = f32(s.stock_ramp_vertical)
                            if truth(bool(sc_abs(tilt) >= 0.5)) then
                                do
                                    if truth(bool(not truth(s.stock_ramp_since))) then
                                        s.stock_ramp_since = (now + 1)
                                    end
                                    if truth(bool(((now + 1) - s.stock_ramp_since) >= 80.0)) then
                                        s.stock_ramp_evidence = (now + 1)
                                    end
                                end
                            else
                                s.stock_ramp_since = 0
                            end
                            local curve = trunc(0)
                            do
                                local i = trunc(0)
                                while truth(bool(i < s.stock_ramp_history_n)) do
                                    do
                                        local age = ((now + 1) - s.stock_ramp_history_t[i])
                                        if truth((if truth((if truth((if truth(bool(age >= 100.0)) then bool(truth(bool(age <= 250.0))) else 0)) then bool(truth(bool(sc_abs(tilt) >= 0.17))) else 0)) then bool(truth(bool(sc_abs((tilt - s.stock_ramp_history_y[i])) >= 0.12))) else 0)) then
                                            curve = trunc(1)
                                        end
                                    end
                                    i = trunc((i + 1))
                                end
                            end
                            if truth(curve) then
                                do
                                    s.stock_ramp_evidence = (now + 1)
                                    s.stock_ramp_curve = trunc(1)
                                end
                            end
                            local prev = trunc(((s.stock_ramp_history_next + 7) % 8))
                            if truth((if truth(bool(not truth(s.stock_ramp_history_n))) then 1 else bool(truth(bool(((now + 1) - s.stock_ramp_history_t[prev]) >= 30.0))))) then
                                do
                                    local i = trunc(s.stock_ramp_history_next)
                                    s.stock_ramp_history_y[i] = f32(tilt)
                                    s.stock_ramp_history_t[i] = (now + 1)
                                    s.stock_ramp_history_next = trunc(((i + 1) % 8))
                                    if truth(bool(s.stock_ramp_history_n < 8)) then
                                        s.stock_ramp_history_n = trunc((s.stock_ramp_history_n + 1))
                                    end
                                end
                            end
                            s.stock_ramp_eligible = trunc((if truth(bool(s.stock_ramp_evidence > 0)) then bool(truth(bool(((now + 1) - s.stock_ramp_evidence) <= 450.0))) else 0))
                            if truth(bool(not truth(s.stock_ramp_eligible))) then
                                s.stock_ramp_curve = trunc(0)
                            end
                        end
                    end
                end
            end
        end
        s.stock_ramp_was_air = trunc(stock_airborne)
        local early_ground = trunc((if truth((if truth((if truth((if truth(c.stock_transition_ground) then bool(truth(s.stock_ramp_eligible)) else 0)) then bool(truth(bool(s.takeoff == 0))) else 0)) then bool(truth(bool(not truth(stock_airborne)))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0))
        if truth((if truth(s.stock_air_active) then bool(truth((if truth((if truth(bool(not truth(stock_airborne))) then bool(truth(bool(not truth(early_ground)))) else 0)) then 1 else bool(truth(bool(not truth(c.stock_grab_air))))))) else 0)) then
            do
                s.stock_air_active = trunc(0)
                s.stock_early_active = trunc(0)
                s.d_back_spent = trunc(0)
                s.d_front_spent = trunc(s.d_back_spent)
                s.d_back_on = trunc(s.d_front_spent)
                s.d_front_on = trunc(s.d_back_on)
                s.d_back_ignored = trunc(1)
                s.d_front_ignored = trunc(s.d_back_ignored)
                s.d_back_carry = trunc(0)
                s.d_front_carry = trunc(s.d_back_carry)
                s.d_land_hold = trunc(s.d_front_carry)
                s.d_windup = trunc(0)
                s.hold_foot = trunc(s.d_windup)
                s.d_took_off = trunc(s.hold_foot)
                s.d_was_air = trunc(s.d_took_off)
                s.rp_t = 0
                s.d_front_full = trunc(0)
                s.d_back_full = trunc(s.d_front_full)
                s.phase = trunc(0)
                s.rv_start = 0
                s.tr_rel[1] = 0
                s.tr_rel[0] = s.tr_rel[1]
                s.tr_down[0] = trunc(bool(_c_in.lt >= 100))
                s.tr_down[1] = trunc(bool(_c_in.rt >= 100))
                s.tr_press[1] = now
                s.tr_press[0] = s.tr_press[1]
                s.rv_hold = trunc(0)
                s.bf_arm = trunc(0)
                s.bf_side = trunc(s.bf_arm)
                s.bf_air_t = 0
                do
                    local i = trunc(0)
                    while truth(bool(i < 2)) do
                        do
                            s.g_down[i] = trunc(bool(bit32.band(_c_in.buttons, (if truth(i) then 0x0200 else 0x0100)) ~= 0))
                            s.g_grab[i] = trunc(0)
                            s.g_pulse[i] = 0
                        end
                        i = trunc((i + 1))
                    end
                end
            end
        end
        if truth((if truth((if truth(c.stock_grab_air) then bool(truth(s.stock_ramp_eligible)) else 0)) then bool(truth((if truth(early_ground) then 1 else bool(truth((if truth(stock_airborne) then bool(truth((if truth((if truth(s.stock_air_active) then 1 else bool(truth(bool(_c_in.lt >= 30))))) then 1 else bool(truth(bool(_c_in.rt >= 30)))))) else 0)))))) else 0)) then
            do
                local lx = f32(sc_axis(_c_in.lx))
                local ly = f32(sc_axis(_c_in.ly))
                local rx = f32(sc_axis(_c_in.rx))
                local ry = f32(sc_axis(_c_in.ry))
                local lm = f32(sc_sqrt(((lx * lx) + (ly * ly))))
                local rm = f32(sc_sqrt(((rx * rx) + (ry * ry))))
                if truth(bool(not truth(s.stock_air_active))) then
                    do
                        s.stock_air_active = trunc(1)
                        s.stock_early_active = trunc(early_ground)
                        s.d_log = trunc((if truth(early_ground) then 11 else 10))
                        s.stock_air_block_l = trunc(bool(lm >= c.active))
                        s.stock_air_block_r = trunc(bool(rm >= c.active))
                        s.stock_air_langle = f32(sc_angle(lx, ly))
                        s.stock_air_rangle = f32(sc_angle(rx, ry))
                    end
                end
                if truth((if truth(bool(lm < (c.active * .6))) then 1 else bool(truth(bool(sc_abs(sc_wrap((sc_angle(lx, ly) - s.stock_air_langle))) >= 30.0))))) then
                    s.stock_air_block_l = trunc(0)
                end
                if truth((if truth(bool(rm < (c.active * .6))) then 1 else bool(truth(bool(sc_abs(sc_wrap((sc_angle(rx, ry) - s.stock_air_rangle))) >= 30.0))))) then
                    s.stock_air_block_r = trunc(0)
                end
                local stock = copy(_c_in)
                if truth(s.stock_air_block_l) then
                    stock.ly = trunc(0)
                end
                if truth(s.stock_air_block_r) then
                    stock.ry = trunc(0)
                    stock.rx = trunc(stock.ry)
                end
                s.ps_logged = trunc(0)
                s.ps_side = trunc(s.ps_logged)
                s.rv_start = 0
                s.bf_arm = trunc(0)
                s.bf_side = trunc(s.bf_arm)
                if truth(early_ground) then
                    do
                        local held = trunc(bool(bit32.band(_c_in.buttons, bit32.bor(0x0040, 0x0080)) == 0x0080))
                        local limit = f32((if truth(s.pump_r3) then c.active else c.deadzone))
                        local neutral = trunc((if truth(bool(lm <= limit)) then bool(truth(bool(rm <= limit))) else 0))
                        local was = trunc(s.pump_r3)
                        if truth((if truth(bool(not truth(held))) then 1 else bool(truth(bool(not truth(neutral)))))) then
                            s.pump_r3 = trunc(0)
                        else
                            if truth(bool(not truth(was))) then
                                do
                                    s.pump_r3 = trunc(1)
                                    s.pump_press_t = now
                                    s.pump_release_t = 0
                                end
                            end
                        end
                        if truth((if truth(was) then bool(truth(bool(not truth(s.pump_r3)))) else 0)) then
                            do
                                if truth((if truth(neutral) then bool(truth(bool(not truth(held)))) else 0)) then
                                    do
                                        s.pump_release_t = (now + 1)
                                        s.pump_release_level = f32(s.pump_level)
                                    end
                                else
                                    do
                                        s.pump_release_t = 0
                                        s.pump_level = f32(0)
                                    end
                                end
                            end
                        end
                        if truth((if truth(bool(not truth(neutral))) then 1 else bool(truth(bool(bit32.band(_c_in.buttons, bit32.bor(0x0040, 0x0080)) == bit32.bor(0x0040, 0x0080)))))) then
                            do
                                s.pump_release_t = 0
                                if truth(bool(not truth(s.pump_r3))) then
                                    s.pump_level = f32(0)
                                end
                            end
                        end
                        return copy(sc_pump_output(s, stock, now))
                    end
                end
                s.pump_r3 = trunc(0)
                s.pump_release_t = 0
                s.pump_level = f32(0)
                return copy(stock)
            end
        end
        local out = copy(_c_in)
        if truth(bool(c.walk_ms > 0)) then
            do
                local wx = f32(sc_axis(_c_in.lx))
                local wy = f32(sc_axis(_c_in.ly))
                sc_deadzone({get=function() return wx end, set=function(value) wx = f32(value) end}, {get=function() return wy end, set=function(value) wy = f32(value) end}, c.deadzone)
                local wm = f32(sc_sqrt(((wx * wx) + (wy * wy))))
                local wa = f32(sc_angle(wx, wy))
                local walking_shape = trunc((if truth((if truth(bool(wm >= 0.5)) then bool(truth(bool(wa > 75.0))) else 0)) then bool(truth(bool(wa < 285.0))) else 0))
                if truth((if truth(walking_shape) then bool(truth(bool(s.phase == 0))) else 0)) then
                    do
                        if truth(bool(s.walk_since <= 0)) then
                            s.walk_since = (now + 1)
                        end
                        if truth(bool(((now + 1) - s.walk_since) >= c.walk_ms)) then
                            do
                                s.walk_request = trunc(1)
                                s.walk_since = 0
                                return copy(_c_in)
                            end
                        end
                    end
                else
                    do
                        s.walk_since = 0
                    end
                end
            end
        end
        local left_is_front = trunc((if truth(c.front_back) then 1 else bool(not truth(c.goofy))))
        if truth(c.feet_swapped) then
            left_is_front = trunc(bool(not truth(left_is_front)))
        end
        if truth((if truth((if truth(bool(c.switch_flip == 3)) then bool(truth(s.switched)) else 0)) then bool(truth(bool(not truth(c.front_back)))) else 0)) then
            left_is_front = trunc(bool(not truth(left_is_front)))
        end
        local board_position = trunc((if truth((if truth((if truth(c.pure) then bool(truth(c.xl_layout)) else 0)) then bool(truth(c.air_controls)) else 0)) then bool(truth((if truth((if truth(s.air) then 1 else bool(truth(bool(s.takeoff > 0))))) then 1 else bool(truth(s.grind))))) else 0))
        if truth(board_position) then
            do
                if truth(bool(not truth(s.board_frame_valid))) then
                    do
                        s.board_frame_valid = trunc(1)
                        s.board_frame_left = trunc((if truth(s.feet_map_ok) then s.feet_map_left else left_is_front))
                        s.board_frame_mirror = trunc((if truth(s.game_mirror_known) then bool(truth(s.game_mirror)) else 0))
                    end
                end
                left_is_front = trunc(s.board_frame_left)
            end
        else
            s.board_frame_valid = trunc(0)
        end
        local landing_lean = trunc(0)
        if truth((if truth((if truth((if truth((if truth((if truth(c.pure) then bool(truth(c.air_controls)) else 0)) then bool(truth(s.d_was_air)) else 0)) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff <= 0))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0)) then
            do
                local lx = f32(sc_axis(_c_in.lx))
                local ly = f32(sc_axis(_c_in.ly))
                local rx = f32(sc_axis(_c_in.rx))
                local ry = f32(sc_axis(_c_in.ry))
                sc_deadzone({get=function() return lx end, set=function(value) lx = f32(value) end}, {get=function() return ly end, set=function(value) ly = f32(value) end}, c.deadzone)
                sc_deadzone({get=function() return rx end, set=function(value) rx = f32(value) end}, {get=function() return ry end, set=function(value) ry = f32(value) end}, c.deadzone)
                local lm = f32(((lx * lx) + (ly * ly)))
                local rm = f32(((rx * rx) + (ry * ry)))
                local limit = f32(((if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85) * (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))
                landing_lean = trunc((if truth((if truth((if truth((if truth(bool(lm >= (.3 * .3))) then bool(truth(bool(lm < limit))) else 0)) then bool(truth(bool(rm < (.1 * .1)))) else 0)) then bool(truth(bool(sc_abs(lx) <= (sc_abs(ly) * .6)))) else 0)) then 1 else bool(truth((if truth((if truth((if truth(bool(rm >= (.3 * .3))) then bool(truth(bool(rm < limit))) else 0)) then bool(truth(bool(lm < (.1 * .1)))) else 0)) then bool(truth(bool(sc_abs(rx) <= (sc_abs(ry) * .6)))) else 0)))))
                if truth((if truth((if truth(c.split_grinds) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then bool(truth(s.split_air_setup)) else 0)) then
                    do
                        local off = f32((c.active * .6))
                        off = f32((off * off))
                        local held_left = trunc(bool(s.split_air_setup == (if truth(s.feet_map_left) then 1 else 2)))
                        if truth((if truth((if truth((if truth((if truth(held_left) then bool(truth(bool(lm >= limit))) else 0)) then bool(truth(bool((lx * lx) >= (.0625 * lm)))) else 0)) then bool(truth(bool(rm < off))) else 0)) then 1 else bool(truth((if truth((if truth((if truth(bool(not truth(held_left))) then bool(truth(bool(rm >= limit))) else 0)) then bool(truth(bool((rx * rx) >= (.0625 * rm)))) else 0)) then bool(truth(bool(lm < off))) else 0))))) then
                            landing_lean = trunc(1)
                        end
                    end
                end
            end
        end
        if truth((if truth((if truth((if truth(s.feet_map_ok) then bool(truth((if truth(s.d_land_hold) then 1 else bool(truth(landing_lean))))) else 0)) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff <= 0))) else 0)) then
            left_is_front = trunc(s.feet_map_left)
        end
        if truth((if truth(s.feet_map_ok) then bool(truth(bool(left_is_front ~= s.feet_map_left))) else 0)) then
            sc_remap_foot_history(s)
        end
        s.feet_map_ok = trunc(1)
        s.feet_map_left = trunc(left_is_front)
        local lx = f32(sc_axis(_c_in.lx))
        local ly = f32(sc_axis(_c_in.ly))
        local rx = f32(sc_axis(_c_in.rx))
        local ry = f32(sc_axis(_c_in.ry))
        local fx = f32((if truth(left_is_front) then lx else rx))
        local fy = f32((if truth(left_is_front) then ly else ry))
        local bx = f32((if truth(left_is_front) then rx else lx))
        local by = f32((if truth(left_is_front) then ry else ly))
        local stick_turn = f32(0)
        if truth(c.stick_steer) then
            do
                local ground = trunc((if truth((if truth(bool(not truth(s.air))) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(s.takeoff <= 0))) else 0))
                local steering_left = trunc((if truth(bool(c.stick_steer == 2)) then 1 else left_is_front))
                local sx = f32((if truth(steering_left) then lx else rx))
                local sy = f32((if truth(steering_left) then ly else ry))
                local ox = f32((if truth(steering_left) then rx else lx))
                local oy = f32((if truth(steering_left) then ry else ly))
                local fax = f32(sc_abs(sx))
                local fmag = f32(sc_sqrt(((sx * sx) + (sy * sy))))
                local bmag = f32(sc_sqrt(((ox * ox) + (oy * oy))))
                local pop_pending = trunc((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(s.split_pop_t > 0)) then bool(truth(bool(now >= s.split_pop_t))) else 0)) then bool(truth(bool((now - s.split_pop_t) <= 350))) else 0)) then bool(truth(c.split_grinds)) else 0)) then bool(truth(c.air_controls)) else 0)) then bool(truth(bool(not truth(c.one_foot_flips)))) else 0)) then bool(truth(s.game_known)) else 0)) then bool(truth(bool(s.game_crouch == 1))) else 0))
                local foot_busy = trunc((if truth((if truth((if truth((if truth((if truth(pop_pending) then 1 else bool(truth(s.d_front_on)))) then 1 else bool(truth(s.d_back_on)))) then 1 else bool(truth(s.d_front_full)))) then 1 else bool(truth(s.d_back_full)))) then 1 else bool(truth((if truth(bool(s.rp_t > 0)) then bool(truth(bool((now - s.rp_t) < 70))) else 0)))))
                if truth(bool(not truth(s.st_latch))) then
                    do
                        local angle = trunc(sc_stick_steer_angle(c))
                        local ratio = f32((if truth(bool(angle == 34)) then 1.5 else (sc_sin_deg((90.0 - angle)) / sc_sin_deg(f32(angle)))))
                        if truth((if truth((if truth((if truth((if truth((if truth((if truth(ground) then bool(truth(bool(not truth(foot_busy)))) else 0)) then bool(truth(bool(s.phase == 0))) else 0)) then bool(truth(bool(not truth(s.ps_side)))) else 0)) then bool(truth(bool(bmag < 0.3))) else 0)) then bool(truth(bool(fax >= 0.15))) else 0)) then bool(truth(bool(fax > (ratio * sc_abs(sy))))) else 0)) then
                            s.st_latch = trunc(1)
                        end
                    end
                else
                    if truth((if truth(bool(fmag < 0.10)) then 1 else bool(truth(bool(not truth(ground)))))) then
                        s.st_latch = trunc(0)
                    end
                end
                if truth(s.st_latch) then
                    do
                        stick_turn = f32(sx)
                        if truth(bool(steering_left == left_is_front)) then
                            do
                                fx = f32(0)
                                fy = f32(0)
                            end
                        else
                            do
                                bx = f32(0)
                                by = f32(0)
                            end
                        end
                        if truth(steering_left) then
                            do
                                lx = f32(0)
                                ly = f32(0)
                            end
                        else
                            do
                                rx = f32(0)
                                ry = f32(0)
                            end
                        end
                    end
                end
            end
        end
        local foot_switch = trunc(s.switched)
        if truth((if truth((if truth(board_position) then bool(truth(bool(not truth(c.front_back)))) else 0)) then bool(truth(bool(c.switch_flip == 3))) else 0)) then
            foot_switch = trunc(bool(left_is_front ~= bool(bool(not truth(c.goofy)) ~= bool(not truth(bool(not truth(c.feet_swapped)))))))
        end
        if truth((if truth((if truth((if truth(c.xl_layout) then bool(truth(bool(not truth(c.front_back)))) else 0)) then bool(truth(bool(c.switch_flip == 3))) else 0)) then bool(truth(foot_switch)) else 0)) then
            do
                fy = f32((-fy))
                by = f32((-by))
            end
        end
        if truth(foot_switch) then
            do
                if truth(c.switch_invert_x) then
                    do
                        fx = f32((-fx))
                        bx = f32((-bx))
                    end
                end
                if truth(c.switch_invert_y) then
                    do
                        fy = f32((-fy))
                        by = f32((-by))
                    end
                end
            end
        end
        local ps_active = trunc(0)
        local ps_pending = trunc(0)
        local psx = f32(0)
        local psy = f32(0)
        if truth(bool(c.revert_ms > 0)) then
            do
                local tv = {[0]=trunc(_c_in.lt), [1]=trunc(_c_in.rt)}
                do
                    local i = trunc(0)
                    while truth(bool(i < 2)) do
                        do
                            local down = trunc(bool(tv[i] >= 100))
                            if truth((if truth(down) then bool(truth(bool(not truth(s.tr_down[i])))) else 0)) then
                                do
                                    if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(c.revert_input == 1)) then bool(truth(bool(s.tr_rel[i] > 0))) else 0)) then bool(truth(bool((now - s.tr_rel[i]) <= c.revert_ms))) else 0)) then bool(truth(bool((now - s.tr_rel[i]) >= c.revert_min_gap_ms))) else 0)) then bool(truth(bool((s.tr_rel[i] - s.tr_press[i]) <= c.revert_ms))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff ~= 1))) else 0)) then bool(truth(bool(s.game_crouch ~= 1))) else 0)) then bool(truth(bool(s.phase == 0))) else 0)) then bool(truth(bool(not truth((if truth(s.d_front_on) then bool(truth(bool(sc_sqrt(((fx * fx) + (fy * fy))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0))))) else 0)) then bool(truth(bool(not truth((if truth(s.d_back_on) then bool(truth(bool(sc_sqrt(((bx * bx) + (by * by))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0))))) else 0)) then
                                        do
                                            local gap = (now - s.tr_rel[i])
                                            local start = (s.tr_rel[i] + c.revert_gap_ms)
                                            s.rv_start = (if truth(bool(start > now)) then start else now)
                                            s.rv_side = trunc((if truth(i) then 1 else (-1)))
                                            s.tr_rel[i] = 0
                                            s.rv_block = trunc(1)
                                            s.rv_hold = trunc((i + 1))
                                            s.d_log = trunc(4)
                                            s.d_log_a = f32(f32(s.rv_side))
                                            s.d_log_b = f32(f32(gap))
                                        end
                                    end
                                    s.tr_press[i] = now
                                end
                            else
                                if truth((if truth(bool(not truth(down))) then bool(truth(s.tr_down[i])) else 0)) then
                                    do
                                        s.tr_rel[i] = (if truth((if truth(bool(s.rv_start > 0)) then bool(truth(bool((now - s.rv_start) < 400.0))) else 0)) then 0 else now)
                                    end
                                end
                            end
                            s.tr_down[i] = trunc(down)
                        end
                        i = trunc((i + 1))
                    end
                end
                if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(c.revert_input == 2)) then bool(truth(s.tr_down[0])) else 0)) then bool(truth(s.tr_down[1])) else 0)) then bool(truth(bool(not truth(s.rv_both)))) else 0)) then bool(truth(bool(sc_abs((s.tr_press[0] - s.tr_press[1])) <= 120.0))) else 0)) then bool(truth((if truth(bool(s.rv_start <= 0)) then 1 else bool(truth(bool((now - s.rv_start) > 600.0)))))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff ~= 1))) else 0)) then bool(truth(bool(s.game_crouch ~= 1))) else 0)) then bool(truth(bool(not truth((if truth(s.d_front_on) then bool(truth(bool(sc_sqrt(((fx * fx) + (fy * fy))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0))))) else 0)) then bool(truth(bool(not truth((if truth(s.d_back_on) then bool(truth(bool(sc_sqrt(((bx * bx) + (by * by))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0))))) else 0)) then
                    do
                        local first_r = trunc(bool(s.tr_press[1] < s.tr_press[0]))
                        s.rv_start = now
                        s.rv_side = trunc((if truth(first_r) then 1 else (-1)))
                        s.rv_block = trunc(1)
                        s.rv_hold = trunc(3)
                        s.rv_both = trunc(1)
                        s.tr_rel[1] = 0
                        s.tr_rel[0] = s.tr_rel[1]
                        s.d_log = trunc(4)
                        s.d_log_a = f32(f32(s.rv_side))
                        s.d_log_b = f32(0)
                    end
                end
                if truth((if truth(bool(not truth(s.tr_down[0]))) then bool(truth(bool(not truth(s.tr_down[1])))) else 0)) then
                    s.rv_both = trunc(0)
                end
            end
        end
        local rv_active = trunc(0)
        local rvx = f32(0)
        local rvy = f32(0)
        if truth((if truth(bool(s.rv_start > 0)) then bool(truth((if truth((if truth((if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(sc_sqrt(((fx * fx) + (fy * fy))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then 1 else bool(truth((if truth(s.d_back_on) then bool(truth(bool(sc_sqrt(((bx * bx) + (by * by))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0))))) then 1 else bool(truth(s.air)))) then 1 else bool(truth(bool(s.takeoff == 1))))) then 1 else bool(truth(bool(s.game_crouch == 1)))))) else 0)) then
            do
                s.rv_start = 0
                s.d_log = trunc(8)
            end
        end
        if truth(bool(s.rv_start > 0)) then
            do
                local spin = (if truth(bool(c.revert_sweep_ms > 30)) then c.revert_sweep_ms else 160.0)
                local lead = 0.0
                local t = (now - s.rv_start)
                local dur = (if truth(bool(c.revert_shape == 5)) then ((spin * 580.0) / 300.0) else ((lead + spin) + 20.0))
                local quiet = (dur + 100.0)
                local cw = trunc(bool(((s.rv_side * (if truth(c.revert_flip) then (-1) else 1)) * (if truth(c.goofy) then (-1) else 1)) > 0))
                if truth(bool(t < 0)) then
                    do
                        rv_active = trunc(2)
                    end
                else
                    if truth(bool(t < dur)) then
                        do
                            local u = f32((if truth(bool(t > spin)) then 1.0 else f32((t / spin))))
                            local th = 0
                            do
                                local switch_value = c.revert_shape
                                if switch_value == 2 then
                                    th = f32((if truth(cw) then (180.0 + (180.0 * u)) else (180.0 - (180.0 * u))))
                                elseif switch_value == 3 then
                                    th = f32((if truth(cw) then (360.0 - (180.0 * u)) else (180.0 * u)))
                                elseif switch_value == 4 then
                                    th = f32((if truth(cw) then (90.0 + (180.0 * u)) else (270.0 - (180.0 * u))))
                                elseif switch_value == 1 then
                                    th = f32((if truth(cw) then (360.0 * u) else ((-360.0) * u)))
                                else
                                    th = f32(0)
                                end
                            end
                            rvx = f32((sc_sin_deg(th) * 0.95))
                            rvy = f32((sc_sin_deg((90.0 - th)) * 0.95))
                            if truth(bool(c.revert_shape == 5)) then
                                do
                                    local sd = f32((if truth(bool((s.rv_side * (if truth(c.revert_flip) then (-1) else 1)) > 0)) then 1.0 else (-1.0)))
                                    local k = (spin / 300.0)
                                    local t1 = (40.0 * k)
                                    local t2 = (t1 + (160.0 * k))
                                    local t3 = (t2 + (110.0 * k))
                                    local a = 0
                                    if truth(bool(t < t1)) then
                                        do
                                            rvx = f32((sd * f32((t / t1))))
                                            rvy = f32(0)
                                            a = f32((-1))
                                        end
                                    else
                                        if truth(bool(t < t2)) then
                                            a = f32((90.0 + (90.0 * f32(((t - t1) / (t2 - t1))))))
                                        else
                                            if truth(bool(t < t3)) then
                                                a = f32((180.0 + (105.0 * f32(((t - t2) / (t3 - t2))))))
                                            else
                                                a = f32(285.0)
                                            end
                                        end
                                    end
                                    if truth(bool(a >= 0)) then
                                        do
                                            local ar = f32((if truth(bool(sd > 0)) then a else (360.0 - a)))
                                            rvx = f32(sc_sin_deg(ar))
                                            rvy = f32(sc_sin_deg((90.0 - ar)))
                                        end
                                    end
                                end
                            else
                                if truth((if truth(bool(c.revert_shape < 1)) then 1 else bool(truth(bool(c.revert_shape > 4))))) then
                                    do
                                        local side = f32((if truth(cw) then 1.0 else (-1.0)))
                                        local arc = f32((if truth(bool(c.revert_shape == 6)) then 135.0 else 180.0))
                                        if truth(bool(u < 0.3)) then
                                            do
                                                rvx = f32(((side * 0.95) * (u / 0.3)))
                                                rvy = f32(0)
                                            end
                                        else
                                            do
                                                local v = f32(((u - 0.3) / 0.7))
                                                local a = f32((if truth(cw) then (90.0 + (arc * v)) else (270.0 - (arc * v))))
                                                rvx = f32((sc_sin_deg(a) * 0.95))
                                                rvy = f32((sc_sin_deg((90.0 - a)) * 0.95))
                                            end
                                        end
                                    end
                                end
                            end
                            rv_active = trunc(1)
                        end
                    else
                        if truth(bool(t < quiet)) then
                            do
                                rv_active = trunc(2)
                            end
                        else
                            s.rv_start = 0
                        end
                    end
                end
            end
        end
        if truth((if truth((if truth((if truth(bool(not truth(c.powerslide))) then 1 else bool(truth(s.air)))) then 1 else bool(truth(bool(s.takeoff == 1))))) then 1 else bool(truth(s.grind)))) then
            do
                s.ps_logged = trunc(0)
                s.ps_side = trunc(s.ps_logged)
                s.ps_rt = 0
                s.ps_lt = s.ps_rt
                s.ps_pop_t = 0
            end
        end
        if truth((if truth((if truth((if truth((if truth(c.powerslide) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff ~= 1))) else 0)) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(s.phase ~= 2))) else 0)) then
            do
                local alx = f32(sc_abs(lx))
                local arx = f32(sc_abs(rx))
                local side = trunc(0)
                local slide_on = f32((if truth(s.ps_side) then 0.35 else 0.4))
                local slide_angle = f32((if truth(s.ps_side) then 0.9 else 1.2))
                local lside = trunc((if truth(bool(alx >= slide_on)) then bool(truth(bool(alx > (slide_angle * sc_abs(ly))))) else 0))
                local rside = trunc((if truth(bool(arx >= slide_on)) then bool(truth(bool(arx > (slide_angle * sc_abs(ry))))) else 0))
                if truth(bool(not truth(lside))) then
                    s.ps_lt = 0
                else
                    if truth(bool(s.ps_lt <= 0)) then
                        s.ps_lt = (now + 1)
                    end
                end
                if truth(bool(not truth(rside))) then
                    s.ps_rt = 0
                else
                    if truth(bool(s.ps_rt <= 0)) then
                        s.ps_rt = (now + 1)
                    end
                end
                local recent_pop = trunc((if truth(bool(s.ps_pop_t > 0)) then bool(truth(bool(((now + 1) - s.ps_pop_t) < 300.0))) else 0))
                local foot_busy = trunc((if truth(c.pure) then bool(truth(recent_pop)) else 0))
                if truth((if truth((if truth(bool(fy >= .85)) then bool(truth(bool(sc_abs(fx) < .5))) else 0)) then 1 else bool(truth((if truth(bool(by <= (-.85))) then bool(truth(bool(sc_abs(bx) < .5))) else 0))))) then
                    s.ps_pop_t = (now + 1)
                end
                ps_pending = trunc((if truth((if truth((if truth((if truth((if truth(bool(not truth(foot_busy))) then bool(truth(bool(alx >= .25))) else 0)) then bool(truth(bool(arx >= .25))) else 0)) then bool(truth(bool(alx > (1.2 * sc_abs(ly))))) else 0)) then bool(truth(bool(arx > (1.2 * sc_abs(ry))))) else 0)) then bool(truth(bool(bool(lx > 0) ~= bool(rx > 0)))) else 0))
                if truth((if truth((if truth((if truth(lside) then bool(truth(rside)) else 0)) then bool(truth(bool(bool(lx > 0) ~= bool(rx > 0)))) else 0)) then bool(truth((if truth(s.ps_side) then 1 else bool(truth((if truth(bool(not truth(foot_busy))) then bool(truth((if truth(bool(sc_abs((s.ps_lt - s.ps_rt)) <= 220.0)) then 1 else bool(truth((if truth(bool(alx >= .65)) then bool(truth(bool(arx >= .65))) else 0)))))) else 0)))))) else 0)) then
                    side = trunc((if truth(bool(lx > 0)) then 1 else (-1)))
                end
                if truth((if truth(side) then bool(truth(bool(side ~= s.ps_side))) else 0)) then
                    do
                        s.ps_start = now
                        s.ps_side = trunc(side)
                    end
                else
                    if truth((if truth(bool(not truth(side))) then bool(truth(s.ps_side)) else 0)) then
                        do
                            s.ps_side = trunc(0)
                            s.ps_logged = trunc(0)
                        end
                    end
                end
                if truth((if truth(bool(not truth(ps_active))) then bool(truth(s.ps_side)) else 0)) then
                    do
                        local dir = trunc(((s.ps_side * (if truth(c.ps_flip) then (-1) else 1)) * (if truth(c.goofy) then (-1) else 1)))
                        psx = f32((if truth(bool(dir > 0)) then 0.75 else (-0.75)))
                        psy = f32(((-0.75) + ((0.5 * (ly + ry)) * 0.5)))
                        if truth(bool(psy < (-1.0))) then
                            psy = f32((-1.0))
                        end
                        if truth(bool(psy > (-0.35))) then
                            psy = f32((-0.35))
                        end
                        ps_active = trunc(1)
                        if truth(bool(not truth(s.ps_logged))) then
                            do
                                s.ps_logged = trunc(1)
                                s.d_log = trunc(5)
                                s.d_log_a = f32(f32(s.ps_side))
                            end
                        end
                    end
                end
                if truth(s.ps_side) then
                    do
                        s.d_back_ignored = trunc(1)
                        s.d_front_ignored = trunc(s.d_back_ignored)
                        s.d_back_on = trunc(0)
                        s.d_front_on = trunc(s.d_back_on)
                        s.d_pop = trunc(0)
                        s.d_pending = trunc(0)
                        s.d_windup = trunc(0)
                        s.d_latch = trunc(0)
                        if truth((if truth(bool(s.phase == 3)) then 1 else bool(truth(bool(s.phase == 1))))) then
                            s.phase = trunc(0)
                        end
                    end
                end
            end
        end
        local vx = 0
        local vy = 0
        if truth((if truth((if truth(s.ps_side) then 1 else bool(truth(bool(ps_active == 1))))) then 1 else bool(truth(ps_pending)))) then
            do
                vx = f32(0)
                vy = f32(0)
            end
        else
            if truth(c.pure) then
                sc_pure(c, s, now, fx, fy, bx, by, {get=function() return vx end, set=function(value) vx = f32(value) end}, {get=function() return vy end, set=function(value) vy = f32(value) end})
            else
                if truth(c.direct) then
                    sc_direct(c, s, now, fx, fy, bx, by, {get=function() return vx end, set=function(value) vx = f32(value) end}, {get=function() return vy end, set=function(value) vy = f32(value) end})
                else
                    sc_feet_to_flick(c, s, now, fx, fy, bx, by, {get=function() return vx end, set=function(value) vx = f32(value) end}, {get=function() return vy end, set=function(value) vy = f32(value) end})
                end
            end
        end
        if truth((if truth((if truth((if truth((if truth(c.pure) then bool(truth(c.xl_layout)) else 0)) then bool(truth(s.game_mirror_known)) else 0)) then bool(truth(s.game_mirror)) else 0)) then bool(truth((if truth((if truth(bool(not truth(board_position))) then bool(truth(bool(not truth(s.split_landing_setup)))) else 0)) then 1 else bool(truth((if truth((if truth(s.grind) then bool(truth(c.goofy)) else 0)) then bool(truth((if truth(bool(s.rail_scoop_candidate == 2)) then 1 else bool(truth(bool(s.rail_scoop_exit == 2)))))) else 0)))))) else 0)) then
            vx = f32((-vx))
        end
        s.ox = f32(vx)
        s.oy = f32(vy)
        do
            local want = trunc((if truth((if truth((if truth((if truth((if truth(s.game_switch_known) then s.game_switch else s.switched)) then bool(truth(c.switch_flip)) else 0)) then bool(truth((if truth((if truth(bool(c.switch_flip ~= 3)) then 1 else bool(truth(c.front_back)))) then 1 else bool(truth(c.xl_layout))))) else 0)) then bool(truth((if truth((if truth(bool(not truth(c.goofy))) then 1 else bool(truth(c.switch_goofy_flip)))) then 1 else bool(truth(c.xl_layout))))) else 0)) then 1 else 0))
            local held = trunc((if truth(c.pure) then (if truth(bool(vx ~= 0)) then 1 else bool(truth(bool(vy ~= 0)))) else bool(((vx * vx) + (vy * vy)) >= (0.3 * 0.3))))
            if truth((if truth(bool(not truth(s.sw_lat_ok))) then 1 else bool(truth(bool(not truth(held)))))) then
                do
                    s.sw_lat = trunc(want)
                    s.sw_lat_ok = trunc(1)
                end
            else
                if truth((if truth(bool(want ~= s.sw_lat)) then bool(truth((if truth(s.air) then 1 else bool(truth(bool(s.takeoff == 1)))))) else 0)) then
                    do
                        s.sw_lat = trunc(want)
                    end
                end
            end
            if truth(s.sw_lat) then
                do
                    vy = f32((-vy))
                    if truth(bool(c.switch_flip == 2)) then
                        vx = f32((-vx))
                    end
                end
            end
        end
        if truth(bool(s.switched ~= s.sw_prev)) then
            do
                s.sw_prev = trunc(s.switched)
                if truth(c.switch_flip) then
                    s.sw_blend_until = (now + 250.0)
                end
            end
        end
        if truth((if truth((if truth(0) then bool(truth(bool(now < s.sw_blend_until))) else 0)) then bool(truth(bool(not truth(rv_active)))) else 0)) then
            do
                local dt = (if truth(bool(s.sw_t > 0)) then (now - s.sw_t) else 0)
                local dx = f32((vx - s.sw_x))
                local dy = f32((vy - s.sw_y))
                local d = f32(sc_sqrt(((dx * dx) + (dy * dy))))
                local step = f32(f32((dt / 120.0)))
                if truth((if truth(bool(d > step)) then bool(truth(bool(d > 0))) else 0)) then
                    do
                        vx = f32((s.sw_x + ((dx / d) * step)))
                        vy = f32((s.sw_y + ((dy / d) * step)))
                    end
                end
            end
        end
        s.sw_t = now
        s.sw_x = f32(vx)
        s.sw_y = f32(vy)
        if truth((if truth(rv_active) then bool(truth(bool(sc_sqrt(((vx * vx) + (vy * vy))) >= (if truth((if truth(bool(c.manual_max > 0.2)) then bool(truth(bool(c.manual_max < 0.95))) else 0)) then c.manual_max else 0.85)))) else 0)) then
            do
                vx = f32(0)
                vy = f32(0)
            end
        end
        out.rx = trunc(sc_short(vx))
        out.ry = trunc(sc_short(vy))
        if truth((if truth(bool(s.rv_hold == 3)) then (if truth(bool(not truth(s.tr_down[0]))) then bool(truth(bool(not truth(s.tr_down[1])))) else 0) else (if truth(s.rv_hold) then bool(truth(bool(not truth(s.tr_down[(s.rv_hold - 1)])))) else 0))) then
            s.rv_hold = trunc(0)
        end
        local steer_l = f32((if truth((if truth(bool(s.rv_hold == 1)) then 1 else bool(truth(bool(s.rv_hold == 3))))) then 0.0 else f32(_c_in.lt)))
        local steer_r = f32((if truth((if truth(bool(s.rv_hold == 2)) then 1 else bool(truth(bool(s.rv_hold == 3))))) then 0.0 else f32(_c_in.rt)))
        local steer = f32(((steer_r - steer_l) / 255.0))
        s.rv_block = trunc(0)
        local steer_out = f32((steer + stick_turn))
        if truth(bool(steer_out > 1)) then
            steer_out = f32(1)
        end
        if truth(bool(steer_out < (-1))) then
            steer_out = f32((-1))
        end
        local mag = f32((sc_pow(sc_abs(steer_out), c.steer_curve) * c.steer_scale))
        out.lx = trunc(sc_short((if truth(bool(steer_out < 0)) then (-mag) else mag)))
        out.ly = trunc(0)
        if truth((if truth(c.prewind) then bool(truth((if truth(c.direct) then 1 else bool(truth(c.pure))))) else 0)) then
            do
                local back_crouch = trunc((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(bool(not truth(s.d_front_on)))) else 0))
                local front_crouch = trunc((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(bool(not truth(s.d_back_on)))) else 0))
                local pm = f32((if truth(back_crouch) then sc_sqrt(((bx * bx) + (by * by))) else sc_sqrt(((fx * fx) + (fy * fy)))))
                local real = trunc((if truth(bool(pm >= 0.85)) then 1 else bool(truth(bool(s.game_crouch > 0)))))
                local crouched = trunc((if truth((if truth((if truth((if truth(bool(not truth(s.air))) then bool(truth(bool(not truth(s.d_windup)))) else 0)) then bool(truth((if truth(back_crouch) then 1 else bool(truth(front_crouch))))) else 0)) then bool(truth(bool(s.phase == 0))) else 0)) then bool(truth(real)) else 0))
                local wind_threshold = f32(0.3)
                if truth((if truth((if truth(c.prewind_steer) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff ~= 1))) else 0)) then
                    do
                        local percent = trunc(c.prewind_percent)
                        if truth((if truth(bool(percent < 50)) then 1 else bool(truth(bool(percent > 90))))) then
                            percent = trunc(70)
                        end
                        wind_threshold = f32((percent / 100.0))
                    end
                end
                local want = trunc((if truth(bool(steer >= wind_threshold)) then 1 else (if truth(bool(steer <= (-wind_threshold))) then (-1) else 0)))
                local airborne = trunc((if truth(s.air) then 1 else bool(truth(bool(s.takeoff == 1)))))
                if truth((if truth((if truth(s.pw_side) then bool(truth(s.pw_was_air)) else 0)) then bool(truth(bool(not truth(airborne)))) else 0)) then
                    do
                        s.pw_side = trunc(0)
                        s.pw_logged = trunc(0)
                        s.pw_block = trunc(want)
                    end
                end
                if truth(s.pw_side) then
                    s.pw_was_air = trunc(airborne)
                end
                if truth((if truth(crouched) then 1 else bool(truth(airborne)))) then
                    s.pw_crouch_t = now
                end
                if truth((if truth((if truth((if truth(s.pw_side) then bool(truth(bool(not truth(airborne)))) else 0)) then bool(truth(bool(not truth(crouched)))) else 0)) then bool(truth(bool((now - s.pw_crouch_t) > 250.0))) else 0)) then
                    do
                        s.pw_side = trunc(0)
                        s.pw_logged = trunc(0)
                        s.pw_block = trunc(want)
                    end
                end
                if truth((if truth(bool(not truth(want))) then 1 else bool(truth(bool(want ~= s.pw_block))))) then
                    s.pw_block = trunc(0)
                end
                if truth(bool(not truth(want))) then
                    s.pw_side = trunc(0)
                else
                    if truth((if truth((if truth(bool(want ~= s.pw_side)) then bool(truth(crouched)) else 0)) then bool(truth(bool(not truth(s.pw_block)))) else 0)) then
                        do
                            s.pw_side = trunc(want)
                            s.pw_t = now
                            s.pw_nollie = trunc(front_crouch)
                            s.pw_was_air = trunc(0)
                        end
                    end
                end
                if truth((if truth(s.pw_side) then bool(truth(bool(want == s.pw_side))) else 0)) then
                    do
                        local sx = f32(f32(s.pw_side))
                        local hx = 0
                        local hy = 0
                        local ax = 0
                        local ay = 0
                        local game_nollie = trunc(bool(s.pw_nollie ~= (if truth((if truth((if truth((if truth(s.game_switch_known) then s.game_switch else s.switched)) then bool(truth(c.switch_flip)) else 0)) then bool(truth((if truth((if truth(bool(c.switch_flip ~= 3)) then 1 else bool(truth(c.front_back)))) then 1 else bool(truth(c.xl_layout))))) else 0)) then bool(truth((if truth((if truth(bool(not truth(c.goofy))) then 1 else bool(truth(c.switch_goofy_flip)))) then 1 else bool(truth(c.xl_layout))))) else 0)))
                        local from_up = trunc((if truth((if truth(bool(c.prewind == 4)) then 1 else bool(truth((if truth(bool(c.prewind == 1)) then bool(truth(game_nollie)) else 0))))) then 1 else bool(truth((if truth(bool(c.prewind == 5)) then bool(truth(bool(not truth(game_nollie)))) else 0)))))
                        if truth(bool(c.prewind == 3)) then
                            do
                                ax = f32(0)
                                ay = f32(0)
                                hx = f32(sx)
                                hy = f32(0)
                            end
                        else
                            if truth(from_up) then
                                do
                                    ax = f32(0)
                                    ay = f32(1.0)
                                    hx = f32((0.8 * sx))
                                    hy = f32(0.6)
                                end
                            else
                                do
                                    ax = f32(0)
                                    ay = f32((-1.0))
                                    hx = f32((0.8 * sx))
                                    hy = f32((-0.6))
                                end
                            end
                        end
                        local ms = (if truth(bool(c.prewind_ms > 0)) then c.prewind_ms else 60.0)
                        local t = (now - s.pw_t)
                        local u = f32((if truth(bool(t >= ms)) then 1.0 else f32((t / ms))))
                        local px = 0
                        local py = 0
                        if truth(bool(u < 0.4)) then
                            do
                                local v = f32((u / 0.4))
                                px = f32((ax * v))
                                py = f32((ay * v))
                            end
                        else
                            do
                                local v = f32(((u - 0.4) / 0.6))
                                px = f32((ax + ((hx - ax) * v)))
                                py = f32((ay + ((hy - ay) * v)))
                                local m = f32(sc_sqrt(((px * px) + (py * py))))
                                if truth(bool(m > 0.01)) then
                                    do
                                        px = f32((px * (0.97 / m)))
                                        py = f32((py * (0.97 / m)))
                                    end
                                end
                            end
                        end
                        if truth((if truth(bool(c.prewind ~= 3)) then bool(truth(bool(t >= (ms + 30.0)))) else 0)) then
                            do
                                px = f32((0.97 * sx))
                                py = f32(0)
                            end
                        end
                        out.lx = trunc(sc_short(px))
                        out.ly = trunc(sc_short(py))
                        if truth(bool(not truth(s.pw_logged))) then
                            do
                                s.pw_logged = trunc(1)
                                s.d_log = trunc(6)
                                s.d_log_a = f32(sx)
                            end
                        end
                    end
                else
                    s.pw_logged = trunc(0)
                end
            end
        end
        do
            local th = trunc(trunc(bit32.band(_c_in.buttons, bit32.bor(0x0040, 0x0080))))
            local down = trunc(trunc(bit32.band(th, bit32.bnot(s.bump_prev))))
            local pump_was_held = trunc(s.pump_r3)
            local pump_ground = trunc((if truth((if truth(bool(not truth(s.grind))) then bool(truth(bool(not truth(s.air)))) else 0)) then bool(truth(bool(s.takeoff ~= 1))) else 0))
            local pump_limit = f32((if truth(s.pump_r3) then c.active else c.deadzone))
            local pump_neutral = trunc((if truth(bool(((lx * lx) + (ly * ly)) <= (pump_limit * pump_limit))) then bool(truth(bool(((rx * rx) + (ry * ry)) <= (pump_limit * pump_limit)))) else 0))
            if truth((if truth((if truth(bool(not truth(pump_ground))) then 1 else bool(truth(bool(not truth(pump_neutral)))))) then 1 else bool(truth(bool(th ~= 0x0080))))) then
                s.pump_r3 = trunc(0)
            else
                if truth((if truth((if truth((if truth(bool(s.phase == 0)) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then bool(truth(bool(s.game_crouch ~= 1))) else 0)) then
                    do
                        if truth(bool(not truth(s.pump_r3))) then
                            do
                                s.pump_press_t = now
                                s.pump_release_t = 0
                            end
                        end
                        s.pump_r3 = trunc(1)
                        s.bf_side = trunc(0)
                        s.bf_arm = trunc(s.bf_side)
                    end
                end
            end
            if truth((if truth(pump_was_held) then bool(truth(bool(not truth(s.pump_r3)))) else 0)) then
                do
                    if truth((if truth((if truth(pump_neutral) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(not truth(bit32.band(th, 0x0080))))) else 0)) then
                        do
                            s.pump_release_t = (now + 1)
                            s.pump_release_level = f32(s.pump_level)
                        end
                    else
                        do
                            s.pump_release_t = 0
                            s.pump_level = f32(0)
                        end
                    end
                end
            end
            if truth((if truth((if truth(bool(not truth(pump_neutral))) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(bool(th == bit32.bor(0x0040, 0x0080)))))) then
                do
                    s.pump_release_t = 0
                    if truth(bool(not truth(s.pump_r3))) then
                        s.pump_level = f32(0)
                    end
                end
            end
            if truth((if truth((if truth((if truth(s.grind) then bool(truth(bool(c.bump_ms > 0))) else 0)) then bool(truth(down)) else 0)) then bool(truth(bool(th ~= bit32.bor(0x0040, 0x0080)))) else 0)) then
                do
                    local cx = f32(sc_axis((if truth(bit32.band(down, 0x0080)) then _c_in.rx else _c_in.lx)))
                    local cy = f32(sc_axis((if truth(bit32.band(down, 0x0080)) then _c_in.ry else _c_in.ly)))
                    s.bump_y = trunc((if truth((if truth(bool(sc_abs(cy) >= 0.5)) then bool(truth(bool(sc_abs(cy) > sc_abs(cx)))) else 0)) then (if truth(bool(cy > 0)) then 1 else (-1)) else 0))
                    s.bump_side = trunc((if truth(s.bump_y) then 0 else (if truth(bit32.band(down, 0x0080)) then 1 else (-1))))
                    local held_direction = trunc((if truth(s.bump_y) then bool((sc_axis(out.ly) * s.bump_y) > .5) else bool((sc_axis(out.lx) * s.bump_side) > .5)))
                    if truth((if truth(bool((now - s.bump_output_time) <= 50)) then bool(truth(bool(s.bump_output_time > 0))) else 0)) then
                        held_direction = trunc(bit32.bor(held_direction, (if truth(s.bump_y) then bool((s.bump_output_y * s.bump_y) > .5) else bool((s.bump_output_x * s.bump_side) > .5))))
                    end
                    s.bump_neutral_until = (if truth(held_direction) then (now + 20) else 0)
                    s.bump_until = ((now + c.bump_ms) + (if truth(held_direction) then 20 else 0))
                    s.d_log = trunc(7)
                    s.d_log_a = f32(f32(s.bump_side))
                    s.d_log_b = f32(f32(s.bump_y))
                end
            end
            local airborne = trunc((if truth(s.air) then 1 else bool(truth(bool(s.takeoff == 1)))))
            if truth((if truth((if truth((if truth((if truth(c.body_flips) then bool(truth(bool(not truth(s.grind)))) else 0)) then bool(truth(bool(not truth(s.pump_r3)))) else 0)) then bool(truth(down)) else 0)) then bool(truth(bool(th ~= bit32.bor(0x0040, 0x0080)))) else 0)) then
                do
                    local front_click = trunc((if truth(bit32.band(down, 0x0040)) then left_is_front else bool(not truth(left_is_front))))
                    local dir = trunc((if truth(front_click) then 1 else (-1)))
                    local crouched = trunc((if truth((if truth(s.d_front_on) then 1 else bool(truth(s.d_back_on)))) then 1 else bool(truth(bool(s.game_crouch == 1)))))
                    if truth(airborne) then
                        do
                            s.bf_side = trunc(dir)
                            s.bf_t = now
                            s.bf_arm = trunc(0)
                            s.d_log = trunc(9)
                            s.d_log_a = f32(f32(dir))
                        end
                    else
                        if truth(crouched) then
                            do
                                s.bf_arm = trunc(dir)
                                s.bf_arm_t = now
                                s.d_log = trunc(9)
                                s.d_log_a = f32(f32((dir * 2)))
                            end
                        end
                    end
                end
            end
            if truth((if truth(s.bf_arm) then bool(truth((if truth((if truth(airborne) then 1 else bool(truth(bool(s.d_log == 2))))) then 1 else bool(truth(bool(s.rp_t == now)))))) else 0)) then
                do
                    s.bf_side = trunc(s.bf_arm)
                    s.bf_t = now
                    s.bf_arm = trunc(0)
                end
            end
            if truth((if truth((if truth(s.bf_arm) then bool(truth(bool(not truth(airborne)))) else 0)) then bool(truth(bool((now - s.bf_arm_t) > 700.0))) else 0)) then
                s.bf_arm = trunc(0)
            end
            if truth((if truth((if truth(s.bf_side) then bool(truth(s.bf_was_air)) else 0)) then bool(truth(bool(not truth(airborne)))) else 0)) then
                s.bf_side = trunc(0)
            end
            if truth((if truth((if truth((if truth(s.bf_side) then bool(truth(bool(not truth(airborne)))) else 0)) then bool(truth(bool(not truth(s.bf_was_air)))) else 0)) then bool(truth(bool((now - s.bf_t) > 400.0))) else 0)) then
                s.bf_side = trunc(0)
            end
            if truth((if truth((if truth(s.bf_side) then bool(truth(airborne)) else 0)) then bool(truth(bool(not truth(s.bf_air_t)))) else 0)) then
                s.bf_air_t = now
            end
            if truth(bool(not truth(s.bf_side))) then
                s.bf_air_t = 0
            end
            s.bf_was_air = trunc(airborne)
            s.bump_prev = trunc(th)
            if truth((if truth((if truth(s.grind) then 1 else bool(truth((if truth(c.body_flips) then bool(truth((if truth((if truth((if truth((if truth(airborne) then 1 else bool(truth(s.bf_arm)))) then 1 else bool(truth(s.d_front_on)))) then 1 else bool(truth(s.d_back_on)))) then 1 else bool(truth(bool(s.game_crouch == 1)))))) else 0))))) then bool(truth(bool(th ~= bit32.bor(0x0040, 0x0080)))) else 0)) then
                out.buttons = trunc(bit32.band(out.buttons, trunc(bit32.bnot(bit32.bor(0x0040, 0x0080)))))
            end
        end
        if truth(ps_active) then
            do
                out.lx = trunc(sc_short(psx))
                out.ly = trunc(sc_short(psy))
            end
        end
        if truth(bool(rv_active == 1)) then
            do
                out.lx = trunc(sc_short(rvx))
                out.ly = trunc(sc_short(rvy))
            end
        end
        if truth(s.bf_side) then
            do
                local y = f32((if truth(bool((now - s.bf_t) < 20.0)) then ((-0.15) * s.bf_side) else (0.97 * s.bf_side)))
                local sx = f32(sc_axis(out.lx))
                if truth(bool(sx > 0.6)) then
                    sx = f32(0.6)
                end
                if truth(bool(sx < (-0.6))) then
                    sx = f32((-0.6))
                end
                out.lx = trunc(sc_short(sx))
                out.ly = trunc(sc_short(y))
            end
        end
        if truth(bool(now < s.bump_until)) then
            do
                local neutral = trunc(bool(now < s.bump_neutral_until))
                out.lx = trunc((if truth(neutral) then 0 else sc_short(f32(s.bump_side))))
                out.ly = trunc((if truth(neutral) then 0 else sc_short(f32(s.bump_y))))
                out.ry = trunc(0)
                out.rx = trunc(out.ry)
            end
        else
            if truth(bool(rv_active == 2)) then
                do
                    out.lx = trunc(0)
                    out.ly = trunc(0)
                end
            end
        end
        local glt = trunc(0)
        local grt = trunc(0)
        if truth((if truth(bool(c.grab_mode >= 1)) then bool(truth(bool(c.grab_double_ms > 0))) else 0)) then
            do
                local bit = {[0]=trunc(0x0100), [1]=trunc(0x0200)}
                local hold = (if truth(bool(c.grab_hold_ms > 0)) then c.grab_hold_ms else 100.0)
                do
                    local i = trunc(0)
                    while truth(bool(i < 2)) do
                        do
                            local down = trunc(bool(bit32.band(_c_in.buttons, bit[i]) ~= 0))
                            out.buttons = trunc(bit32.band(out.buttons, trunc(bit32.bnot(bit[i]))))
                            if truth((if truth(down) then bool(truth(bool(not truth(s.g_down[i])))) else 0)) then
                                do
                                    local dbl = trunc((if truth((if truth(bool(s.g_rel[i] > 0)) then bool(truth(bool((now - s.g_rel[i]) <= c.grab_double_ms))) else 0)) then bool(truth(bool((s.g_rel[i] - s.g_press[i]) <= c.grab_double_ms))) else 0))
                                    local ground = trunc((if truth(bool(not truth(s.air))) then bool(truth(bool(s.takeoff ~= 1))) else 0))
                                    s.g_grab[i] = trunc((if truth((if truth(ground) then 1 else bool(truth(dbl)))) then 1 else 3))
                                    s.g_press[i] = now
                                end
                            else
                                if truth((if truth(bool(not truth(down))) then bool(truth(s.g_down[i])) else 0)) then
                                    do
                                        if truth((if truth(bool(s.g_grab[i] == 3)) then bool(truth(s.air)) else 0)) then
                                            s.g_pulse[i] = (now + 80.0)
                                        end
                                        s.g_rel[i] = (if truth((if truth(bool(s.g_grab[i] == 1)) then 1 else bool(truth(bool(s.g_grab[i] == 2))))) then 0 else now)
                                        s.g_grab[i] = trunc(0)
                                    end
                                end
                            end
                            s.g_down[i] = trunc(down)
                            if truth((if truth((if truth(down) then bool(truth(bool(s.g_grab[i] == 3))) else 0)) then bool(truth(bool((now - s.g_press[i]) >= hold))) else 0)) then
                                s.g_grab[i] = trunc(2)
                            end
                            if truth((if truth(down) then bool(truth(bool(s.g_grab[i] == 1))) else 0)) then
                                out.buttons = trunc(bit32.bor(out.buttons, bit[i]))
                            end
                            if truth((if truth(down) then bool(truth(bool(s.g_grab[i] == 2))) else 0)) then
                                do
                                    if truth(bool(i == 0)) then
                                        glt = trunc(255)
                                    else
                                        grt = trunc(255)
                                    end
                                end
                            end
                            if truth(bool(now < s.g_pulse[i])) then
                                out.buttons = trunc(bit32.bor(out.buttons, bit[i]))
                            end
                        end
                        i = trunc((i + 1))
                    end
                end
            end
        else
            if truth(bool(c.grab_mode >= 1)) then
                do
                    if truth(bit32.band(_c_in.buttons, 0x0100)) then
                        glt = trunc(255)
                    end
                    if truth(bit32.band(_c_in.buttons, 0x0200)) then
                        grt = trunc(255)
                    end
                    out.buttons = trunc(bit32.band(out.buttons, trunc(bit32.bnot(bit32.bor(0x0100, 0x0200)))))
                end
            end
        end
        if truth(bool(c.grab_mode == 2)) then
            do
                if truth(bool(_c_in.lt >= c.grab_threshold)) then
                    glt = trunc(255)
                end
                if truth(bool(_c_in.rt >= c.grab_threshold)) then
                    grt = trunc(255)
                end
            end
        end
        if truth((if truth((if truth(s.bf_side) then bool(truth(bool(s.bf_air_t > 0))) else 0)) then bool(truth(bool((now - s.bf_air_t) < c.bf_grab_ms))) else 0)) then
            do
                if truth(bool(s.bf_side > 0)) then
                    grt = trunc(255)
                else
                    glt = trunc(255)
                end
            end
        end
        out.lt = trunc(glt)
        out.rt = trunc(grt)
        out = copy(sc_pump_output(s, out, now))
        s.bump_output_x = f32(sc_axis(out.lx))
        s.bump_output_y = f32(sc_axis(out.ly))
        s.bump_output_time = now
        return copy(out)
    end
end
local function sc_mc_replay_record(m, x, y, now)
    do
        if truth((if truth(m.replay_count) then bool(truth(bool((now - m.replay_sample_at) < 8))) else 0)) then
            return
        end
        if truth(bool((now - m.replay_sample_at) > 80)) then
            m.replay_count = trunc(0)
            m.replay_head = trunc(m.replay_count)
        end
        m.replay_x[m.replay_head] = trunc(sc_short(x))
        m.replay_y[m.replay_head] = trunc(sc_short(y))
        m.replay_head = trunc(((m.replay_head + 1) % 32))
        if truth(bool(m.replay_count < 32)) then
            m.replay_count = trunc((m.replay_count + 1))
        end
        m.replay_sample_at = now
    end
end
local function sc_mc_cancel(m, buttons)
    do
        replace(m, {phase=trunc(0), seen=0, front_left=0, requested=0, was_air=0, candidate=0, recentred=0, candidate_kind=0, scoop_track=0, hold_window_id=0, observed_seen=0, observed_id=0, observed_doing=0, emitted_seen=0, id_seen=0, id_previous=0, held_recentred=0, previous=0, x=0, y=0, scoop_last=0, scoop_sweep=0, emitted_x=0, emitted_y=0, candidate_at=0, release_until=0, started=0, scoop_time=0, emitted_at=0, replay_x={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0, [25]=0, [26]=0, [27]=0, [28]=0, [29]=0, [30]=0, [31]=0}, replay_y={[0]=0, [1]=0, [2]=0, [3]=0, [4]=0, [5]=0, [6]=0, [7]=0, [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0, [16]=0, [17]=0, [18]=0, [19]=0, [20]=0, [21]=0, [22]=0, [23]=0, [24]=0, [25]=0, [26]=0, [27]=0, [28]=0, [29]=0, [30]=0, [31]=0}, replay_head=0, replay_count=0, replay_running=0, replay_sample_at=0, replay_started=0})
        m.seen = trunc(1)
        m.previous = trunc(bit32.band(buttons, bit32.bor(0x0040, 0x0080)))
    end
end
local function sc_mc_frame(c, s, x, y)
    do
        if truth((if truth((if truth(c.xl_layout) then bool(truth(s.game_mirror_known)) else 0)) then bool(truth(s.game_mirror)) else 0)) then
            x.set(f32((-x.get())))
        end
        if truth(s.sw_lat) then
            do
                y.set(f32((-y.get())))
                if truth(bool(c.switch_flip == 2)) then
                    x.set(f32((-x.get())))
                end
            end
        end
    end
end
local function sc_manual_catch_translate_observed(c, s, m, raw, now, state, trick_id, doing_trick)
    raw = copy(raw)
    do
        local air = trunc((if truth((if truth(s.air) then 1 else bool(truth(bool(s.takeoff > 0))))) then bool(truth(bool(not truth(s.grind)))) else 0))
        local valid = trunc((if truth((if truth((if truth((if truth(c.manual_catch) then bool(truth(c.enabled)) else 0)) then bool(truth(c.pure)) else 0)) then bool(truth(bool(state >= 100))) else 0)) then bool(truth(bool(state < 400))) else 0))
        local clicks = trunc(bit32.band(raw.buttons, bit32.bor(0x0040, 0x0080)))
        local edge = trunc((if truth(m.seen) then bit32.band(clicks, bit32.bnot(m.previous)) else 0))
        m.seen = trunc(1)
        m.previous = trunc(clicks)
        if truth((if truth((if truth((if truth(bool(not truth(valid))) then 1 else bool(truth(s.stock_air_active)))) then 1 else bool(truth(s.stock_early_active)))) then 1 else bool(truth(s.radial_passthrough)))) then
            do
                sc_mc_cancel(m, raw.buttons)
                return copy(sc_translate(c, s, raw, now))
            end
        end
        if truth((if truth(m.was_air) then bool(truth(bool(not truth(air)))) else 0)) then
            do
                sc_mc_cancel(m, raw.buttons)
            end
        end
        if truth((if truth(s.grind) then bool(truth(m.phase)) else 0)) then
            sc_mc_cancel(m, raw.buttons)
        end
        local native_event = trunc(0)
        local native_id_event = trunc(0)
        if truth(bool(trick_id >= 0)) then
            do
                native_id_event = trunc((if truth((if truth(m.id_seen) then bool(truth(bool(trick_id > 0))) else 0)) then bool(truth(bool(trick_id ~= m.id_previous))) else 0))
                m.id_seen = trunc(1)
                m.id_previous = trunc(trick_id)
            end
        else
            m.id_seen = trunc(0)
        end
        if truth((if truth(bool(trick_id >= 0)) then bool(truth((if truth(bool(doing_trick == 0)) then 1 else bool(truth(bool(doing_trick == 1)))))) else 0)) then
            do
                native_event = trunc((if truth((if truth((if truth(m.observed_seen) then bool(truth(doing_trick)) else 0)) then bool(truth(bool(trick_id > 0))) else 0)) then bool(truth((if truth(bool(trick_id ~= m.observed_id)) then 1 else bool(truth(bool(not truth(m.observed_doing))))))) else 0))
                m.observed_seen = trunc(1)
                m.observed_id = trunc(trick_id)
                m.observed_doing = trunc(doing_trick)
            end
        else
            m.observed_seen = trunc(0)
        end
        if truth((if truth((if truth(bool(m.phase == 2)) then bool(truth(bool(not truth(air)))) else 0)) then bool(truth(bool((now - m.started) > 250))) else 0)) then
            sc_mc_cancel(m, raw.buttons)
        end
        local input = copy(raw)
        local ready = trunc((if truth(bool(m.phase == 2)) then 1 else bool(truth((if truth((if truth((if truth(bool(m.phase == 0)) then bool(truth(m.candidate)) else 0)) then bool(truth(bool((now - m.candidate_at) <= 160))) else 0)) then bool(truth(air)) else 0)))))
        if truth((if truth(air) then bool(truth(ready)) else 0)) then
            do
                input.buttons = trunc(bit32.band(input.buttons, trunc(bit32.bnot(bit32.bor(0x0040, 0x0080)))))
                if truth(edge) then
                    do
                        if truth(bool(m.phase ~= 3)) then
                            m.requested = trunc(0)
                        end
                        if truth(bit32.band(edge, 0x0040)) then
                            m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 1 else 2)))
                        end
                        if truth(bit32.band(edge, 0x0080)) then
                            m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 2 else 1)))
                        end
                        m.phase = trunc(3)
                        m.release_until = (now + 60)
                        m.recentred = trunc(0)
                        m.emitted_seen = trunc(0)
                        s.hold_foot = trunc(0)
                        s.bf_arm = trunc(0)
                    end
                end
            end
        else
            if truth((if truth(air) then bool(truth(bool(m.phase == 3))) else 0)) then
                do
                    input.buttons = trunc(bit32.band(input.buttons, trunc(bit32.bnot(bit32.bor(0x0040, 0x0080)))))
                    if truth(bit32.band(edge, 0x0040)) then
                        m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 1 else 2)))
                    end
                    if truth(bit32.band(edge, 0x0080)) then
                        m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 2 else 1)))
                    end
                end
            end
        end
        local out = copy(sc_translate(c, s, input, now))
        if truth((if truth((if truth(s.stock_air_active) then 1 else bool(truth(s.stock_early_active)))) then 1 else bool(truth(s.radial_passthrough)))) then
            do
                sc_mc_cancel(m, raw.buttons)
                return copy(out)
            end
        end
        if truth((if truth(c.manual_catch_replay) then bool(truth((if truth((if truth(bool(not truth(air))) then bool(truth(bool(m.phase ~= 3))) else 0)) then 1 else bool(truth((if truth(air) then bool(truth((if truth(bool(m.phase == 0)) then 1 else bool(truth((if truth((if truth(bool(m.phase == 3)) then bool(truth(m.recentred)) else 0)) then bool(truth(bool(now >= m.release_until))) else 0)))))) else 0)))))) else 0)) then
            do
                local x = f32(sc_axis(out.rx))
                local y = f32(sc_axis(out.ry))
                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                sc_mc_replay_record(m, x, y, now)
            end
        end
        if truth((if truth(air) then bool(truth((if truth(bool(m.phase == 0)) then 1 else bool(truth((if truth((if truth(bool(m.phase == 3)) then bool(truth(m.recentred)) else 0)) then bool(truth(bool(now >= m.release_until))) else 0)))))) else 0)) then
            do
                local x = f32(sc_axis(out.rx))
                local y = f32(sc_axis(out.ry))
                local mag = f32(((x * x) + (y * y)))
                if truth(bool(mag >= (.75 * .75))) then
                    do
                        sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                        local scale = f32(sc_sqrt(mag))
                        m.emitted_x = f32((x / scale))
                        m.emitted_y = f32((y / scale))
                        m.emitted_at = now
                        m.emitted_seen = trunc(1)
                    end
                end
                if truth((if truth((if truth(native_event) then bool(truth(m.emitted_seen)) else 0)) then bool(truth(bool((now - m.emitted_at) <= 160))) else 0)) then
                    do
                        m.x = f32(m.emitted_x)
                        m.y = f32(m.emitted_y)
                        m.phase = trunc(2)
                        m.requested = trunc(0)
                        m.candidate = trunc(1)
                        m.candidate_kind = trunc(3)
                        m.front_left = trunc(s.feet_map_left)
                        m.started = now
                        m.candidate_at = m.started
                        m.emitted_seen = trunc(0)
                        m.recentred = trunc(0)
                        m.held_recentred = trunc(0)
                        if truth(edge) then
                            do
                                if truth(bit32.band(edge, 0x0040)) then
                                    m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 1 else 2)))
                                end
                                if truth(bit32.band(edge, 0x0080)) then
                                    m.requested = trunc(bit32.bor(m.requested, (if truth(m.front_left) then 2 else 1)))
                                end
                                m.phase = trunc(3)
                                m.release_until = (now + 60)
                                s.bf_side = trunc(0)
                                s.bf_arm = trunc(s.bf_side)
                                s.hold_foot = trunc(s.bf_arm)
                                out.buttons = trunc(bit32.band(out.buttons, trunc(bit32.bnot(bit32.bor(0x0040, 0x0080)))))
                            end
                        end
                    end
                end
            end
        end
        if truth((if truth(bool(not truth(air))) then bool(truth(bool(m.phase == 0))) else 0)) then
            do
                local x = f32(sc_axis(out.rx))
                local y = f32(sc_axis(out.ry))
                local mag = f32(((x * x) + (y * y)))
                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                local foot = trunc((if truth((if truth((if truth(s.d_front_on) then bool(truth(bool(not truth(s.d_front_spent)))) else 0)) then bool(truth(bool(not truth(s.d_back_on)))) else 0)) then 1 else (if truth((if truth((if truth(s.d_back_on) then bool(truth(bool(not truth(s.d_back_spent)))) else 0)) then bool(truth(bool(not truth(s.d_front_on)))) else 0)) then 2 else 0)))
                local scoop = trunc(bool(s.rail_scoop_exit ~= 0))
                if truth((if truth((if truth((if truth((if truth((if truth(foot) then bool(truth(bool(mag >= (.5 * .5)))) else 0)) then bool(truth(bool(not truth(s.ps_side)))) else 0)) then bool(truth(bool(not truth(s.st_latch)))) else 0)) then bool(truth(bool(not truth(s.pump_r3)))) else 0)) then bool(truth((if truth(bool(not truth(s.grind))) then 1 else bool(truth(s.rail_scoop_exit))))) else 0)) then
                    do
                        local a = f32(sc_angle(x, y))
                        local dt = (now - m.scoop_time)
                        if truth((if truth((if truth(bool(m.scoop_track == foot)) then bool(truth(bool(dt > 0))) else 0)) then bool(truth(bool(dt <= 100))) else 0)) then
                            do
                                local delta = f32(sc_wrap((a - m.scoop_last)))
                                if truth(bool(sc_abs(delta) <= 110)) then
                                    m.scoop_sweep = f32((m.scoop_sweep + delta))
                                else
                                    m.scoop_sweep = f32(0)
                                end
                            end
                        else
                            m.scoop_sweep = f32(0)
                        end
                        m.scoop_track = trunc(foot)
                        m.scoop_last = f32(a)
                        m.scoop_time = now
                        scoop = trunc(bit32.bor(scoop, bool(sc_abs(m.scoop_sweep) >= 35)))
                    end
                else
                    do
                        m.scoop_track = trunc(0)
                        m.scoop_sweep = f32(0)
                    end
                end
                local flick = trunc((if truth((if truth((if truth((if truth(s.d_front_flick) then 1 else bool(truth(s.d_back_flick)))) then 1 else bool(truth(bool(s.d_log == 2))))) then 1 else bool(truth((if truth(c.one_foot_flips) then bool(truth(s.d_windup)) else 0))))) then 1 else bool(truth(s.rail_scoop_exit))))
                if truth((if truth((if truth((if truth((if truth((if truth(scoop) then 1 else bool(truth((if truth(flick) then bool(truth(bool(sc_abs(x) >= .2))) else 0))))) then bool(truth(bool(mag >= (.75 * .75)))) else 0)) then bool(truth(bool(not truth(s.ps_side)))) else 0)) then bool(truth(bool(not truth(s.st_latch)))) else 0)) then bool(truth(bool(not truth(s.pump_r3)))) else 0)) then
                    do
                        local scale = f32(sc_sqrt(mag))
                        m.x = f32((x / scale))
                        m.y = f32((y / scale))
                        m.candidate = trunc(1)
                        m.candidate_kind = trunc((if truth(scoop) then 2 else 1))
                        m.candidate_at = now
                        m.front_left = trunc(s.feet_map_left)
                    end
                end
                if truth((if truth(m.candidate) then bool(truth(bool((now - m.candidate_at) > 160))) else 0)) then
                    m.candidate = trunc(0)
                end
            end
        end
        if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(s.grind))) then bool(truth(bool(m.phase ~= 3))) else 0)) then bool(truth(bool(m.phase ~= 2))) else 0)) then bool(truth(m.candidate)) else 0)) then bool(truth((if truth((if truth(bool(state == 103)) then 1 else bool(truth(air)))) then 1 else bool(truth((if truth((if truth(native_id_event) then 1 else bool(truth(native_event)))) then bool(truth((if truth(bool(state == 100)) then 1 else bool(truth(bool(state == 101)))))) else 0)))))) else 0)) then bool(truth(bool((now - m.candidate_at) <= 160))) else 0)) then
            do
                m.phase = trunc(2)
                m.requested = trunc(0)
                m.started = now
                m.held_recentred = trunc(0)
            end
        end
        if truth(bool(m.phase == 2)) then
            do
                if truth((if truth(air) then bool(truth(bool(not truth(c.manual_catch_replay)))) else 0)) then
                    do
                        local lx = f32(sc_axis(raw.lx))
                        local ly = f32(sc_axis(raw.ly))
                        local rx = f32(sc_axis(raw.rx))
                        local ry = f32(sc_axis(raw.ry))
                        if truth((if truth(bool(((lx * lx) + (ly * ly)) < (.18 * .18))) then bool(truth(bool(((rx * rx) + (ry * ry)) < (.18 * .18)))) else 0)) then
                            m.held_recentred = trunc(1)
                        end
                        local x = f32(sc_axis(out.rx))
                        local y = f32(sc_axis(out.ry))
                        local mag = f32(((x * x) + (y * y)))
                        if truth((if truth(m.held_recentred) then bool(truth(bool(mag >= (.85 * .85)))) else 0)) then
                            do
                                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                local scale = f32(sc_sqrt(mag))
                                m.x = f32((x / scale))
                                m.y = f32((y / scale))
                            end
                        end
                    end
                end
                if truth(bool(not truth(air))) then
                    do
                        local x = f32(sc_axis(out.rx))
                        local y = f32(sc_axis(out.ry))
                        local mag = f32(((x * x) + (y * y)))
                        if truth(bool(mag >= (.85 * .85))) then
                            do
                                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                local scale = f32(sc_sqrt(mag))
                                m.x = f32((x / scale))
                                m.y = f32((y / scale))
                            end
                        end
                    end
                end
                local x = f32(m.x)
                local y = f32(m.y)
                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                m.hold_window_id = trunc(0)
                out.rx = trunc(sc_short(x))
                out.ry = trunc(sc_short(y))
                if truth((if truth(c.manual_catch_replay) then bool(truth(air)) else 0)) then
                    do
                        if truth(bool(not truth(m.replay_running))) then
                            do
                                if truth((if truth(bool(not truth(m.replay_count))) then 1 else bool(truth(bool((now - m.replay_sample_at) > 160))))) then
                                    do
                                        m.replay_count = trunc(0)
                                        m.replay_head = trunc(m.replay_count)
                                        sc_mc_replay_record(m, m.x, m.y, now)
                                    end
                                end
                                m.replay_running = trunc(1)
                                m.replay_started = now
                            end
                        end
                        local cycle = trunc((16 + (m.replay_count * 8)))
                        local step = trunc((trunc((now - m.replay_started)) % cycle))
                        if truth(bool(step < 16)) then
                            out.ry = trunc(0)
                            out.rx = trunc(out.ry)
                        else
                            do
                                local i = trunc(((((m.replay_head + 32) - m.replay_count) + trunc(((step - 16) / 8))) % 32))
                                x = f32(sc_axis(m.replay_x[i]))
                                y = f32(sc_axis(m.replay_y[i]))
                                sc_mc_frame(c, s, {get=function() return x end, set=function(value) x = f32(value) end}, {get=function() return y end, set=function(value) y = f32(value) end})
                                out.rx = trunc(sc_short(x))
                                out.ry = trunc(sc_short(y))
                            end
                        end
                    end
                else
                    m.replay_running = trunc(0)
                end
            end
        else
            if truth(bool(m.phase == 3)) then
                do
                    m.replay_running = trunc(0)
                    local lx = f32(sc_axis(raw.lx))
                    local ly = f32(sc_axis(raw.ly))
                    local rx = f32(sc_axis(raw.rx))
                    local ry = f32(sc_axis(raw.ry))
                    if truth((if truth(bool(((lx * lx) + (ly * ly)) < (.18 * .18))) then bool(truth(bool(((rx * rx) + (ry * ry)) < (.18 * .18)))) else 0)) then
                        m.recentred = trunc(1)
                    end
                    if truth((if truth(bool(now < m.release_until)) then 1 else bool(truth(bool(not truth(m.recentred)))))) then
                        out.ry = trunc(0)
                        out.rx = trunc(out.ry)
                    end
                end
            end
        end
        m.was_air = trunc(air)
        return copy(out)
    end
end
local function sc_manual_catch_translate_ex(c, s, m, raw, now, state, trick_id)
    raw = copy(raw)
    do
        return copy(sc_manual_catch_translate_observed(c, s, m, raw, now, state, trick_id, (-1)))
    end
end
local function sc_manual_catch_translate(c, s, m, raw, now, state)
    raw = copy(raw)
    do
        return copy(sc_manual_catch_translate_ex(c, s, m, raw, now, state, (-1)))
    end
end
local function sc_camera_dpad_suspend(s, buttons)
    do
        local zero = {previous=trunc(0), blocked=0, started={[0]=0, [1]=0}, active=0, held=0, cancelled=0, synced=0, selected=0, pending=0, selected_at=0, pulse_until=0, last_time=0, next_toggle=0, owner=0}
        replace(s, zero)
        s.previous = trunc(buttons)
        s.blocked = trunc(bit32.band(buttons, 12))
        s.synced = trunc(1)
    end
end
local function sc_camera_dpad_step(s, buttons, now, enabled, request)
    do
        request.set(trunc(0))
        if truth((if truth((if truth((if truth(bool(not truth(enabled))) then 1 else bool(truth(bool(not truth(s.synced)))))) then 1 else bool(truth(bool(now < s.last_time))))) then 1 else bool(truth((if truth(bool(s.last_time > 0)) then bool(truth(bool((now - s.last_time) > 500))) else 0))))) then
            do
                sc_camera_dpad_suspend(s, buttons)
                s.last_time = now
                return trunc(buttons)
            end
        end
        s.last_time = now
        s.blocked = trunc(bit32.band(s.blocked, buttons))
        local out = trunc(bit32.band(buttons, bit32.bnot(12)))
        do
            local i = trunc(0)
            while truth(bool(i < 2)) do
                do
                    local bit = trunc(bit32.lshift(4, i))
                    local mask = trunc(bit32.lshift(1, i))
                    if truth(bit32.band(buttons, bit)) then
                        do
                            if truth(bit32.band(s.blocked, bit)) then
                                do
                                    out = trunc(bit32.bor(out, bit))
                                    i = trunc((i + 1))
                                    continue
                                end
                            end
                            if truth(bool(not truth(bit32.band(s.previous, bit)))) then
                                do
                                    s.started[i] = now
                                    s.active = trunc(bit32.bor(s.active, mask))
                                end
                            end
                            if truth(bit32.band(buttons, bit32.bnot(bit))) then
                                s.cancelled = trunc(bit32.bor(s.cancelled, mask))
                            end
                            if truth((if truth(bit32.band(s.active, mask)) then bool(truth(bool((now - s.started[i]) >= 500.0))) else 0)) then
                                s.held = trunc(bit32.bor(s.held, mask))
                            end
                            if truth(bit32.band(s.held, mask)) then
                                out = trunc(bit32.bor(out, bit))
                            end
                        end
                    else
                        if truth(bit32.band(s.previous, bit)) then
                            do
                                if truth((if truth((if truth((if truth((if truth(bit32.band(s.active, mask)) then bool(truth(bool(not truth(bit32.band(s.held, mask))))) else 0)) then bool(truth(bool(not truth(bit32.band(s.cancelled, mask))))) else 0)) then bool(truth(bool(not truth(bit32.band(buttons, bit32.bnot(bit)))))) else 0)) then bool(truth(bool((now - s.started[i]) < 500.0))) else 0)) then
                                    request.set(trunc((if truth(i) then (-1) else 1)))
                                end
                                s.active = trunc(bit32.band(s.active, bit32.bnot(mask)))
                                s.held = trunc(bit32.band(s.held, bit32.bnot(mask)))
                                s.cancelled = trunc(bit32.band(s.cancelled, bit32.bnot(mask)))
                            end
                        end
                    end
                end
                i = trunc((i + 1))
            end
        end
        s.previous = trunc(buttons)
        return trunc(out)
    end
end
local function sc_camera_dpad_output_buttons(translated, remapped, camera_pulse)
    do
        if truth(remapped) then
            translated = trunc(bit32.band(translated, trunc(bit32.bnot(0x0040))))
        end
        if truth(camera_pulse) then
            translated = trunc(bit32.bor(translated, 0x0040))
        end
        return trunc(translated)
    end
end
local function sc_camera_dpad_toggle(s, request, observed, now)
    do
        if truth(bool(not truth(request))) then
            return trunc(0)
        end
        local current = trunc((if truth((if truth(s.selected) then bool(truth(bool((now - s.selected_at) < 1000))) else 0)) then s.selected else observed))
        if truth(bool(not truth(current))) then
            return trunc(0)
        end
        s.selected = trunc(request)
        s.selected_at = now
        return trunc(bool(current ~= request))
    end
end
local function sc_camera_side_geometry(position, forward, camera)
    do
        do
            local i = trunc(0)
            while truth(bool(i < 3)) do
                if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(finite(position[i])))) then 1 else bool(truth(bool(not truth(finite(forward[i]))))))) then 1 else bool(truth(bool(not truth(finite(camera[i]))))))) then 1 else bool(truth(bool(position[i] < (-1000000)))))) then 1 else bool(truth(bool(position[i] > 1000000))))) then 1 else bool(truth(bool(camera[i] < (-1000000)))))) then 1 else bool(truth(bool(camera[i] > 1000000))))) then
                    return trunc(0)
                end
                i = trunc((i + 1))
            end
        end
        local fx = f32(forward[0])
        local fz = f32(forward[2])
        local len = f32(((fx * fx) + (fz * fz)))
        local dx = f32((camera[0] - position[0]))
        local dz = f32((camera[2] - position[2]))
        local distance = f32(((dx * dx) + (dz * dz)))
        if truth((if truth((if truth((if truth(bool(len < .5)) then 1 else bool(truth(bool(len > 1.5))))) then 1 else bool(truth(bool(distance < .25))))) then 1 else bool(truth(bool(distance > 400))))) then
            return trunc(0)
        end
        local rear = f32(((dx * fx) + (dz * fz)))
        if truth(bool((rear * rear) < ((.25 * distance) * len))) then
            return trunc(0)
        end
        if truth(bool(rear > 0)) then
            do
                fx = f32((-fx))
                fz = f32((-fz))
            end
        end
        local side = f32(((dx * fz) - (dz * fx)))
        if truth(bool((side * side) < (.01 * len))) then
            return trunc(0)
        end
        return trunc((if truth(bool(side > 0)) then 1 else (-1)))
    end
end
local function sc_track_stance(t, state, raw, spawned, initial_stance, now)
    do
        local valid = trunc((if truth(bool(raw == 0)) then 1 else bool(truth(bool(raw == 1)))))
        t.late_slide_ms = (-1)
        t.late_turn_ms = t.late_slide_ms
        t.landing_turn_ms = t.late_turn_ms
        if truth((if truth(bool(not truth(t.initialized))) then 1 else bool(truth(spawned)))) then
            do
                t.initialized = trunc(1)
                t.stance = trunc((if truth(valid) then raw else bool(initial_stance > 0)))
                t.pending_reference = trunc((-1))
                t.takeoff_stance = trunc(t.pending_reference)
                t.reference = trunc(t.takeoff_stance)
                t.saw_air_edge = trunc(0)
                t.prev_raw = trunc((-1))
                t.turn_time = 0
            end
        else
            do
                local prev = trunc(t.prev_state)
                local takeoff = trunc((if truth((if truth((if truth(bool(prev >= 100)) then bool(truth(bool(prev < 200))) else 0)) then bool(truth(bool(prev ~= 101))) else 0)) then bool(truth((if truth((if truth(bool(state >= 200)) then bool(truth(bool(state < 500))) else 0)) then 1 else bool(truth(sc_is_grind_state(state)))))) else 0))
                local revert = trunc((if truth((if truth((if truth(bool(prev >= 100)) then bool(truth(bool(prev < 200))) else 0)) then bool(truth(bool(prev ~= 101))) else 0)) then bool(truth(bool(state == 101))) else 0))
                local landed = trunc((if truth((if truth((if truth((if truth(bool(prev >= 200)) then bool(truth(bool(prev < 500))) else 0)) then 1 else bool(truth(sc_is_grind_state(prev))))) then bool(truth(bool(state >= 100))) else 0)) then bool(truth(bool(state < 200))) else 0))
                local end_slide = trunc((if truth(bool(prev == 101)) then bool(truth(bool(state ~= 101))) else 0))
                if truth((if truth((if truth((if truth(landed) then 1 else bool(truth(end_slide)))) then bool(truth(bool(t.reference >= 0))) else 0)) then bool(truth(bool(t.takeoff_stance >= 0))) else 0)) then
                    do
                        local final_raw = trunc((if truth(valid) then raw else t.prev_raw))
                        if truth(bool(final_raw >= 0)) then
                            t.stance = trunc((if truth(bool(final_raw ~= t.reference)) then bool(not truth(t.takeoff_stance)) else t.takeoff_stance))
                        end
                        if truth((if truth((if truth((if truth(landed) then 1 else bool(truth(end_slide)))) then bool(truth((if truth((if truth(end_slide) then 1 else bool(truth(bool(not truth(t.saw_air_edge)))))) then 1 else bool(truth(t.settle_touchdown))))) else 0)) then bool(truth(bool(final_raw == t.reference))) else 0)) then
                            do
                                t.pending_reference = trunc(t.reference)
                                t.pending_stance = trunc(t.takeoff_stance)
                                t.pending_revert = trunc((if truth(end_slide) then bool(truth(bool(not truth(landed)))) else 0))
                                t.landed_at = now
                            end
                        else
                            t.pending_reference = trunc((-1))
                        end
                        t.takeoff_stance = trunc((-1))
                        t.reference = trunc(t.takeoff_stance)
                    end
                end
                if truth((if truth(landed) then bool(truth(bool(t.turn_time > 0))) else 0)) then
                    do
                        t.landing_turn_ms = (now - t.turn_time)
                        t.turn_time = 0
                    end
                end
                if truth((if truth((if truth(takeoff) then 1 else bool(truth(revert)))) then 1 else bool(truth((if truth(end_slide) then bool(truth((if truth((if truth(bool(state >= 200)) then bool(truth(bool(state < 500))) else 0)) then 1 else bool(truth(sc_is_grind_state(state)))))) else 0))))) then
                    do
                        t.reference = trunc((if truth(bool(t.prev_raw >= 0)) then t.prev_raw else (if truth(valid) then raw else (-1))))
                        t.saw_air_edge = trunc(0)
                        t.pending_reference = trunc((-1))
                        t.takeoff_stance = trunc(t.stance)
                        t.turn_time = 0
                    end
                end
                if truth((if truth((if truth((if truth((if truth(bool(t.reference >= 0)) then bool(truth(bool(t.takeoff_stance >= 0))) else 0)) then bool(truth(bool(state >= 200))) else 0)) then bool(truth(bool(state < 300))) else 0)) then bool(truth(valid)) else 0)) then
                    do
                        local wanted = trunc((if truth(bool(raw ~= t.reference)) then bool(not truth(t.takeoff_stance)) else t.takeoff_stance))
                        if truth(bool(raw ~= t.reference)) then
                            t.saw_air_edge = trunc(1)
                        end
                        if truth(bool(wanted ~= t.stance)) then
                            do
                                t.stance = trunc(wanted)
                                t.turn_time = now
                            end
                        end
                    end
                end
                if truth(bool(t.pending_reference >= 0)) then
                    do
                        if truth((if truth((if truth(bool(state < 100)) then 1 else bool(truth(bool(state >= 200))))) then 1 else bool(truth(bool((now - t.landed_at) > 500.0))))) then
                            t.pending_reference = trunc((-1))
                        else
                            if truth((if truth(valid) then bool(truth(bool(raw ~= t.pending_reference))) else 0)) then
                                do
                                    t.stance = trunc(bool(not truth(t.pending_stance)))
                                    if truth(t.pending_revert) then
                                        t.late_slide_ms = (now - t.landed_at)
                                    else
                                        t.late_turn_ms = (now - t.landed_at)
                                    end
                                    t.pending_reference = trunc((-1))
                                end
                            end
                        end
                    end
                end
            end
        end
        t.prev_state = trunc(state)
        if truth(valid) then
            t.prev_raw = trunc(raw)
        else
            if truth(spawned) then
                t.prev_raw = trunc((-1))
            end
        end
        return trunc(t.stance)
    end
end
local function sc_resolve_stance(t, source, goofy, mode, front_back, state, raw, spawned, initial, now)
    do
        if truth((if truth(bool(source == 4)) then bool(truth(bool(not truth((if truth(bool(mode == 3)) then bool(truth(bool(not truth(front_back)))) else 0))))) else 0)) then
            do
                t.late_slide_ms = (-1)
                t.late_turn_ms = t.late_slide_ms
                t.landing_turn_ms = t.late_turn_ms
                local valid = trunc((if truth(bool(raw == 0)) then 1 else bool(truth(bool(raw == 1)))))
                if truth((if truth((if truth((if truth(bool(not truth(t.direct_initialized))) then 1 else bool(truth(bool(state == 300))))) then 1 else bool(truth(bool(state == 504))))) then 1 else bool(truth(bool(state == 700))))) then
                    do
                        t.direct_initialized = trunc(1)
                        t.direct_stance = trunc((if truth(valid) then raw else (if truth(bool(initial >= 0)) then initial else 0)))
                        t.direct_candidate = trunc((-1))
                    end
                else
                    if truth((if truth(valid) then bool(truth(bool(raw ~= t.direct_stance))) else 0)) then
                        do
                            if truth(bool(state ~= 100)) then
                                do
                                    t.direct_stance = trunc(raw)
                                    t.direct_candidate = trunc((-1))
                                end
                            else
                                if truth(bool(t.direct_candidate ~= raw)) then
                                    do
                                        t.direct_candidate = trunc(raw)
                                        t.direct_since = now
                                    end
                                else
                                    if truth(bool((now - t.direct_since) >= (if truth(t.direct_neutral) then 20 else 75))) then
                                        do
                                            t.direct_stance = trunc(raw)
                                            t.direct_candidate = trunc((-1))
                                        end
                                    end
                                end
                            end
                        end
                    else
                        t.direct_candidate = trunc((-1))
                    end
                end
                return trunc(t.direct_stance)
            end
        end
        if truth((if truth((if truth(bool(source == 4)) then bool(truth(bool(mode == 3))) else 0)) then bool(truth(bool(not truth(front_back)))) else 0)) then
            do
                if truth((if truth(bool(state == 300)) then 1 else bool(truth(bool(state == 700))))) then
                    t.rode_before_walk = trunc(0)
                end
                if truth((if truth((if truth(bool(state == 504)) then bool(truth(t.rode_before_walk)) else 0)) then bool(truth(t.initialized)) else 0)) then
                    do
                        t.pending_reference = trunc((-1))
                        t.takeoff_stance = trunc(t.pending_reference)
                        t.reference = trunc(t.takeoff_stance)
                        t.air_zero_pending = trunc(0)
                        t.saw_air_edge = trunc(t.air_zero_pending)
                        t.turn_time = 0
                        t.prev_state = trunc(504)
                        t.prev_raw = trunc((-1))
                        t.late_slide_ms = (-1)
                        t.late_turn_ms = t.late_slide_ms
                        t.landing_turn_ms = t.late_turn_ms
                        return trunc(t.stance)
                    end
                end
                if truth((if truth((if truth((if truth(bool(state >= 100)) then bool(truth(bool(state < 500))) else 0)) then bool(truth(bool(state ~= 300))) else 0)) then 1 else bool(truth(sc_is_grind_state(state))))) then
                    t.rode_before_walk = trunc(1)
                end
            end
        end
        if truth(bool(source == 4)) then
            spawned = trunc((if truth((if truth(bool(state == 504)) then 1 else bool(truth(bool(state == 300))))) then 1 else bool(truth(bool(state == 700)))))
        end
        if truth(bool(source == 4)) then
            do
                if truth((if truth((if truth((if truth((if truth(bool(not truth(spawned))) then bool(truth(bool(state >= 200))) else 0)) then bool(truth(bool(state < 300))) else 0)) then bool(truth(bool(raw == 0))) else 0)) then bool(truth(bool(t.prev_raw == 1))) else 0)) then
                    do
                        if truth(bool(not truth(t.air_zero_pending))) then
                            do
                                t.air_zero_pending = trunc(1)
                                t.air_zero_since = now
                            end
                        end
                        if truth(bool((now - t.air_zero_since) < 12)) then
                            raw = trunc(1)
                        end
                    end
                else
                    t.air_zero_pending = trunc(0)
                end
            end
        end
        t.settle_touchdown = trunc(bool(source == 4))
        return trunc(sc_track_stance(t, state, raw, spawned, initial, now))
    end
end
local function nr_same(a, b)
    do
        return trunc((if truth((if truth((if truth(bool(a.selector == b.selector)) then bool(truth(bool(a.core == b.core))) else 0)) then bool(truth(bool(a.context == b.context))) else 0)) then bool(truth(bool(a.source == b.source))) else 0))
    end
end
local function sc_physical_foot_layout(front_back, mode)
    do
        return trunc((if truth(bool(not truth(front_back))) then bool(truth(bool(mode == 3))) else 0))
    end
end
local function sc_xl_agreement_missing(g)
    do
        g.baseline_pending = trunc(0)
        g.sampled = trunc(g.baseline_pending)
        g.candidate = trunc((-1))
    end
end
local function sc_xl_agreement_step(g, f, state, idle, profile, now)
    do
        if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(f))) then 1 else bool(truth(bool(profile < 0))))) then 1 else bool(truth(bool(profile > 1))))) then 1 else bool(truth(bool(not truth(g.profile_known)))))) then 1 else bool(truth(bool(g.profile ~= profile))))) then 1 else bool(truth((if truth(g.profile_known) then bool(truth(bool(not truth(nr_same(f, g.previous))))) else 0))))) then
            do
                local empty = {previous=copy(0), time=0, baseline_since=0, candidate_since=0, sampled=0, profile_known=0, profile=0, baseline_pending=0, baseline_valid=0, mirror=0, candidate=0}
                replace(g, empty)
                g.candidate = trunc((-1))
                if truth((if truth((if truth(bool(not truth(f))) then 1 else bool(truth(bool(profile < 0))))) then 1 else bool(truth(bool(profile > 1))))) then
                    return trunc((-1))
                end
                g.profile_known = trunc(1)
                g.profile = trunc(profile)
            end
        end
        local dt = (now - g.time)
        local dot = f32(0)
        local moving = trunc(0)
        if truth((if truth((if truth(g.sampled) then bool(truth(bool(dt >= 20))) else 0)) then bool(truth(bool(dt <= 200))) else 0)) then
            do
                local x = f32((f.position[0] - g.previous.position[0]))
                local z = f32((f.position[2] - g.previous.position[2]))
                local d = f32(((x * x) + (z * z)))
                local v = f32(((f.forward[0] * f.forward[0]) + (f.forward[2] * f.forward[2])))
                local speed = f32(((d * 1000000) / f32((dt * dt))))
                if truth((if truth((if truth((if truth((if truth(finite(speed)) then bool(truth(bool(speed >= 1))) else 0)) then bool(truth(bool(speed <= 225))) else 0)) then bool(truth(bool(v >= .25))) else 0)) then bool(truth(bool(v <= 1.5))) else 0)) then
                    do
                        dot = f32((((x * f.forward[0]) + (z * f.forward[2])) / math.sqrt((d * v))))
                        moving = trunc(1)
                    end
                end
            end
        end
        g.previous = copy(f)
        g.time = now
        g.sampled = trunc(1)
        local normalized = trunc((if truth((if truth(bool(f.basis <= 1)) then bool(truth(bool(f.native_frame <= 1))) else 0)) then bit32.bxor(f.basis, f.native_frame) else (-1)))
        local stable = trunc((if truth((if truth(bool(state == 100)) then bool(truth(moving)) else 0)) then bool(truth(bool(normalized >= 0))) else 0))
        if truth(bool(not truth(g.baseline_valid))) then
            do
                if truth((if truth((if truth((if truth(stable) then bool(truth(bool(f.basis == 0))) else 0)) then bool(truth(bool(dot > .92))) else 0)) then bool(truth(bool(f.native_frame == 0))) else 0)) then
                    do
                        if truth((if truth(bool(not truth(g.baseline_pending))) then 1 else bool(truth(bool(g.mirror ~= f.mirrored))))) then
                            do
                                g.baseline_pending = trunc(1)
                                g.mirror = trunc(f.mirrored)
                                g.baseline_since = now
                            end
                        else
                            if truth(bool((now - g.baseline_since) >= 500)) then
                                g.baseline_valid = trunc(1)
                            end
                        end
                    end
                else
                    g.baseline_pending = trunc(0)
                end
            end
        end
        local desired = trunc((if truth(bool(dot > .92)) then 0 else (if truth(bool(dot < (-.92))) then 1 else (-1))))
        if truth((if truth((if truth((if truth(bool(not truth(stable))) then 1 else bool(truth(bool(not truth(g.baseline_valid)))))) then 1 else bool(truth(bool(desired < 0))))) then 1 else bool(truth(bool(normalized ~= desired))))) then
            do
                g.candidate = trunc((-1))
                return trunc((-1))
            end
        end
        local physical = trunc(bit32.bxor(bit32.bxor(bit32.bxor(desired, f.basis), f.mirrored), g.mirror))
        if truth(bool(g.candidate ~= physical)) then
            do
                g.candidate = trunc(physical)
                g.candidate_since = now
                return trunc((-1))
            end
        end
        return trunc((if truth(bool((now - g.candidate_since) >= (if truth(idle) then 80 else 300))) then physical else (-1)))
    end
end
local function sc_xl_agreement_can_apply(hint, retained, state, feet_neutral)
    do
        return trunc((if truth((if truth((if truth((if truth(bool(hint >= 0)) then bool(truth(bool(hint <= 1))) else 0)) then bool(truth(bool(hint ~= retained))) else 0)) then bool(truth(bool(state == 100))) else 0)) then bool(truth(feet_neutral)) else 0))
    end
end
local function sc_xl_agreement_cached(now, seen, owner, current, profile, current_profile, hint, expected_native, native)
    do
        return trunc((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(owner) then bool(truth(bool(owner == current))) else 0)) then bool(truth(bool(now >= seen))) else 0)) then bool(truth(bool((now - seen) <= 200))) else 0)) then bool(truth(bool(profile == current_profile))) else 0)) then bool(truth(bool(expected_native == native))) else 0)) then bool(truth(bool(expected_native >= 0))) else 0)) then bool(truth(bool(expected_native <= 1))) else 0)) then bool(truth(bool(hint >= 0))) else 0)) then bool(truth(bool(hint <= 1))) else 0)) then hint else (-1)))
    end
end
local function sc_xl_agreement_frame_matches(hint, baseline_mirror, evidence, current)
    do
        return trunc((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(hint >= 0)) then bool(truth(bool(hint <= 1))) else 0)) then bool(truth(nr_same(evidence, current))) else 0)) then bool(truth(bool(evidence.basis == current.basis))) else 0)) then bool(truth(bool(evidence.native_frame == current.native_frame))) else 0)) then bool(truth(bool(evidence.mirrored == current.mirrored))) else 0)) then bool(truth(bool(current.basis <= 1))) else 0)) then bool(truth(bool(current.native_frame <= 1))) else 0)) then bool(truth(bool(baseline_mirror >= 0))) else 0)) then bool(truth(bool(baseline_mirror <= 1))) else 0)) then bool(truth(bool(hint == bit32.bxor(bit32.bxor(current.native_frame, current.mirrored), baseline_mirror)))) else 0))
    end
end
local function sc_xl_physical_raw(native, mirror, baseline_mirror)
    do
        return trunc((if truth((if truth((if truth((if truth((if truth((if truth(bool(native >= 0)) then bool(truth(bool(native <= 1))) else 0)) then bool(truth(bool(mirror >= 0))) else 0)) then bool(truth(bool(mirror <= 1))) else 0)) then bool(truth(bool(baseline_mirror >= 0))) else 0)) then bool(truth(bool(baseline_mirror <= 1))) else 0)) then bit32.bxor(bit32.bxor(native, mirror), baseline_mirror) else (-1)))
    end
end
local function sc_xl_output_frame(xl, stance, motion_bias, native)
    do
        return trunc((if truth(xl) then native else bit32.bxor(stance, motion_bias)))
    end
end
local function sc_xl_absolute_step(s, raw, fallback, now)
    do
        if truth((if truth(bool(raw < 0)) then 1 else bool(truth(bool(raw > 1))))) then
            do
                s.pending = trunc(0)
                return trunc((if truth(s.valid) then s.stance else fallback))
            end
        end
        if truth(bool(not truth(s.valid))) then
            do
                s.valid = trunc(1)
                s.stance = trunc(raw)
                s.pending = trunc(0)
            end
        else
            if truth(bool(raw == s.stance)) then
                s.pending = trunc(0)
            else
                if truth((if truth(bool(not truth(s.pending))) then 1 else bool(truth(bool(s.candidate ~= raw))))) then
                    do
                        s.pending = trunc(1)
                        s.candidate = trunc(raw)
                        s.since = now
                    end
                else
                    if truth(bool((now - s.since) >= 12)) then
                        do
                            s.stance = trunc(raw)
                            s.pending = trunc(0)
                        end
                    end
                end
            end
        end
        return trunc(s.stance)
    end
end
local function sc_motion_stance_step(s, state, valid, x, z, fx, fz, raw, now)
    do
        if truth((if truth(bool(state == 300)) then 1 else bool(truth(bool(state == 700))))) then
            do
                local zero = {time=0, x=0, z=0, candidate_since=0, sampled=0, candidate=0, candidate_valid=0, bias=0, bias_valid=0, seen_riding=0, remount_pending=0, remount_started=0}
                zero.bias_valid = trunc(1)
                replace(s, zero)
                return trunc((-1))
            end
        end
        if truth(bool(state == 504)) then
            do
                if truth(s.seen_riding) then
                    do
                        local zero = {time=0, x=0, z=0, candidate_since=0, sampled=0, candidate=0, candidate_valid=0, bias=0, bias_valid=0, seen_riding=0, remount_pending=0, remount_started=0}
                        zero.remount_pending = trunc(1)
                        replace(s, zero)
                    end
                end
                s.candidate_valid = trunc(0)
                s.sampled = trunc(s.candidate_valid)
                return trunc((-1))
            end
        end
        if truth((if truth(bool(not truth(s.bias_valid))) then bool(truth(bool(not truth(s.remount_pending)))) else 0)) then
            s.bias_valid = trunc(1)
        end
        if truth((if truth(bool(state >= 100)) then bool(truth(bool(state < 300))) else 0)) then
            s.seen_riding = trunc(1)
        end
        if truth(s.remount_pending) then
            do
                if truth((if truth((if truth(bool(not truth(s.remount_started))) then bool(truth(bool(state >= 100))) else 0)) then bool(truth(bool(state < 300))) else 0)) then
                    s.remount_started = now
                end
                if truth((if truth((if truth(bool(state >= 200)) then bool(truth(bool(state < 300))) else 0)) then 1 else bool(truth((if truth(s.remount_started) then bool(truth(bool((now - s.remount_started) > 2000))) else 0))))) then
                    do
                        s.remount_pending = trunc(0)
                        s.bias = trunc(0)
                        s.bias_valid = trunc(1)
                        s.candidate_valid = trunc(0)
                        s.sampled = trunc(s.candidate_valid)
                    end
                end
            end
        end
        if truth(s.bias_valid) then
            return trunc((if truth(bool(raw >= 0)) then bit32.bxor(raw, s.bias) else (-1)))
        end
        if truth((if truth((if truth(bool(state ~= 100)) then 1 else bool(truth(bool(not truth(valid)))))) then 1 else bool(truth(bool(not truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(x > (-1000000))) then bool(truth(bool(x < 1000000))) else 0)) then bool(truth(bool(z > (-1000000)))) else 0)) then bool(truth(bool(z < 1000000))) else 0)) then bool(truth(bool(fx >= (-1.01)))) else 0)) then bool(truth(bool(fx <= 1.01))) else 0)) then bool(truth(bool(fz >= (-1.01)))) else 0)) then bool(truth(bool(fz <= 1.01))) else 0))))))) then
            do
                s.candidate_valid = trunc(0)
                s.sampled = trunc(s.candidate_valid)
                return trunc((if truth((if truth(s.bias_valid) then bool(truth(bool(raw >= 0))) else 0)) then bit32.bxor(raw, s.bias) else (-1)))
            end
        end
        if truth((if truth(s.sampled) then bool(truth(bool((now - s.time) < 90))) else 0)) then
            return trunc((if truth((if truth(s.bias_valid) then bool(truth(bool(raw >= 0))) else 0)) then bit32.bxor(raw, s.bias) else (-1)))
        end
        local dt = (now - s.time)
        local dx = (x - s.x)
        local dz = (z - s.z)
        local previous = trunc(s.sampled)
        s.sampled = trunc(1)
        s.time = now
        s.x = x
        s.z = z
        local distance2 = ((dx * dx) + (dz * dz))
        local facing2 = ((fx * fx) + (fz * fz))
        local dot = ((dx * fx) + (dz * fz))
        if truth((if truth(bool(not truth(previous))) then 1 else bool(truth(bool(not truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(dt >= 90)) then bool(truth(bool(dt <= 250))) else 0)) then bool(truth(bool(raw >= 0))) else 0)) then bool(truth(bool(raw <= 1))) else 0)) then bool(truth(bool(facing2 >= .7))) else 0)) then bool(truth(bool(facing2 <= 1.05))) else 0)) then bool(truth(bool(distance2 >= ((dt * dt) / 1000000)))) else 0)) then bool(truth(bool(distance2 <= ((dt * dt) * .0025)))) else 0)) then bool(truth(bool((dot * dot) >= ((distance2 * facing2) * .81)))) else 0))))))) then
            do
                s.candidate_valid = trunc(0)
                return trunc((if truth((if truth(s.bias_valid) then bool(truth(bool(raw >= 0))) else 0)) then bit32.bxor(raw, s.bias) else (-1)))
            end
        end
        local bias = trunc(bit32.bxor(bool(dot < 0), raw))
        if truth((if truth(bool(not truth(s.candidate_valid))) then 1 else bool(truth(bool(bias ~= s.candidate))))) then
            do
                s.candidate = trunc(bias)
                s.candidate_valid = trunc(1)
                s.candidate_since = now
            end
        else
            if truth(bool((now - s.candidate_since) >= 200)) then
                do
                    s.bias = trunc(bias)
                    s.bias_valid = trunc(1)
                    s.remount_pending = trunc(0)
                end
            end
        end
        return trunc((if truth(s.bias_valid) then bit32.bxor(raw, s.bias) else (-1)))
    end
end
local function sc_fakie_vertical_hold(c, x, y)
    do
        local fx = f32(sc_axis(x))
        local fy = f32(sc_axis(y))
        local low = f32((if truth(bool(c.deadzone > .1)) then c.deadzone else .15))
        return trunc((if truth(bool(sc_abs(fy) >= low)) then bool(truth(bool(sc_abs(fx) <= (sc_abs(fy) * .6)))) else 0))
    end
end
local function sc_fakie_protect_input(c, s, raw, out)
    do
        if truth(bit32.band(raw.buttons, bit32.bor(0x0100, 0x0200))) then
            return trunc(1)
        end
        if truth(s.d_land_hold) then
            return trunc(1)
        end
        return trunc((if truth((if truth(sc_fakie_vertical_hold(c, raw.lx, raw.ly)) then 1 else bool(truth(sc_fakie_vertical_hold(c, raw.rx, raw.ry))))) then 1 else bool(truth(sc_fakie_vertical_hold(c, out.rx, out.ry)))))
    end
end
local function sc_back_entry_plan(c, s)
    do
        local p = {active=trunc(0), type=0, accepted_only=0, yaw=0, pitch=0}
        if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(c.enabled))) then 1 else bool(truth(bool(not truth(c.pure)))))) then 1 else bool(truth(bool(not truth(c.split_grinds)))))) then 1 else bool(truth(bool(not truth(c.native_grind_entry)))))) then 1 else bool(truth(c.one_foot_flips)))) then 1 else bool(truth(bool(not truth(c.air_controls)))))) then 1 else bool(truth(bool(not truth(s.air)))))) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(bool(not truth(s.entry_back_active)))))) then 1 else bool(truth(s.radial_passthrough)))) then
            return copy(p)
        end
        local gain = f32((if truth(bool(c.grind_sensitivity > 0)) then c.grind_sensitivity else 1))
        local x = f32(((-s.entry_back_x) * gain))
        if truth(bool(x > 1)) then
            x = f32(1)
        end
        if truth(bool(x < (-1))) then
            x = f32((-1))
        end
        p.type = trunc((if truth(s.sw_lat) then 1 else 2))
        if truth((if truth(s.sw_lat) then bool(truth(bool(c.switch_flip == 2))) else 0)) then
            x = f32((-x))
        end
        p.yaw = f32((x * 1.570796327))
        p.pitch = f32((s.entry_back_y * .35))
        if truth(s.sw_lat) then
            p.pitch = f32((-p.pitch))
        end
        p.active = trunc(1)
        return copy(p)
    end
end
local function sc_side_entry_plan(c, s)
    do
        local p = {active=trunc(0), type=0, accepted_only=0, yaw=0, pitch=0}
        if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth((if truth(bool(not truth(c.enabled))) then 1 else bool(truth(bool(not truth(c.pure)))))) then 1 else bool(truth(bool(not truth(c.split_grinds)))))) then 1 else bool(truth(c.native_grind_entry)))) then 1 else bool(truth(c.one_foot_flips)))) then 1 else bool(truth(bool(not truth(c.air_controls)))))) then 1 else bool(truth(bool(not truth(s.air)))))) then 1 else bool(truth(s.grind)))) then 1 else bool(truth(bool(not truth(s.entry_back_active)))))) then 1 else bool(truth(bool(not truth(s.entry_side_back)))))) then 1 else bool(truth(s.radial_passthrough)))) then 1 else bool(truth(bool(sc_abs(s.entry_back_x) < .75))))) then 1 else bool(truth(bool(sc_abs(s.entry_back_y) > (.35 * sc_abs(s.entry_back_x))))))) then
            return copy(p)
        end
        p.active = trunc(1)
        p.accepted_only = trunc(1)
        p.type = trunc((if truth(s.sw_lat) then 1 else 2))
        p.yaw = f32(((-s.entry_back_x) * 1.570796327))
        p.pitch = f32((s.entry_back_y * .35))
        if truth(s.sw_lat) then
            do
                p.pitch = f32((-p.pitch))
                if truth(bool(c.switch_flip == 2)) then
                    p.yaw = f32((-p.yaw))
                end
            end
        end
        return copy(p)
    end
end
local function sc_entry_fresh(now, seen, state)
    do
        return trunc((if truth((if truth((if truth(seen) then bool(truth(bool(now >= seen))) else 0)) then bool(truth(bool((now - seen) <= 60))) else 0)) then bool(truth(bool(state == 201))) else 0))
    end
end
M.new_state = new_sc_state; M.new_config = new_sc_config; M.new_pad = new_sc_pad
M.translate = sc_translate; M.stick_steer_angle = sc_stick_steer_angle
M.new_catch = new_sc_manual_catch; M.catch = sc_manual_catch_translate_observed; M.cancel_catch = sc_mc_cancel
M.new_camera = new_sc_camera_dpad; M.camera_step = sc_camera_dpad_step; M.camera_buttons = sc_camera_dpad_output_buttons
M.new_stance = new_sc_stance_track; M.resolve_stance = sc_resolve_stance
M.camera_side = sc_camera_side_geometry
M.camera_toggle = sc_camera_dpad_toggle; M.camera_suspend = sc_camera_dpad_suspend
M.new_motion = new_sc_motion_stance; M.motion_step = sc_motion_stance_step
M.new_agreement = new_sc_xl_agreement; M.agreement_step = sc_xl_agreement_step; M.agreement_missing = sc_xl_agreement_missing
M.new_absolute = new_sc_xl_absolute; M.absolute_step = sc_xl_absolute_step; M.physical_raw = sc_xl_physical_raw; M.output_frame = sc_xl_output_frame
M.new_frame = new_nr_frame; M.protect_fakie = sc_fakie_protect_input
M.entry_plan = sc_back_entry_plan; M.is_grind = sc_is_grind_state; M.physical_foot_layout = sc_physical_foot_layout
return M
