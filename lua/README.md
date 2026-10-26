# Dual Stick Controls — Lua testing build

Standalone Luau port of Dual Stick v6.6 for ReSkate. Includes Session-style, XL-style and custom controls, five preset slots, manual catch, D-pad camera sides, foot setup grinds, pumping and stance/fakie handling.

**This is a testing candidate. It requires the proposed ReSkate controller API update. The currently released settings-only Lua plugin cannot run this translator.** No separate Dual Stick DLL is included or required once those APIs are available in ReSkate.

## Install for testing

1. Close Skate. Install a compatible ReSkate prerelease containing the controller-layer API additions.
2. Disable the old Dual Stick companion by moving its version.dll out of the game folder. Keep the official ReSkate.dll. Running both translators will apply controls twice.
3. Extract this package's contents into a single folder such as Mods/dual-stick/.
4. Start the game and open ReSkate > PLUGINS > Dual Stick. Choose your controller, stance and preset. Release held controls after closing a menu.

The plugin creates its own session_controls.ini in Mods/.reskate_plugin_data/<mod-folder-name>/. To migrate, back up your old INI and copy it into that data folder while the game is closed. Existing keys, comments and custom slots are preserved. Renaming the mod folder changes its data folder.

The Thunderstore manifest is staging metadata: its dependency list must be pinned to the API-bearing ReSkate release before public upload. Nothing has been uploaded to Thunderstore by this work.

## Controls and settings

- Session-style: front/back foot sticks, flick pop and trigger steering. One-stick flips are blocked by default.
- XL-style: physical foot directions, release pop and left-stick steering. Foot jobs follow switch stance. One-stick flips are allowed by default.
- Hold LB + RB + L3 + R3 together for three seconds to toggle Dual Stick. A live toggle is also on the plugin page.
- R3 is pumping crouch with centred foot sticks. Hold and release with your pumping timing.
- Partial vertical foot input sets up manuals. Stick tilts/clicks handle rail exits; centred L3/R3 request left/right exits.
- Manual Catch is optional and OFF by default. Click L3, R3 or both in the air. Some tricks settle at an angle; timing still matters. Independent foot catches and forced missed-catch bails are not implemented.
- D-pad left/right taps request a camera side. Holds of half a second pass through the original action. Native L3 camera switching is filtered after the mod receives trick/catch clicks. Menus retain normal input.
- Disable hold-fakie preserves deliberate held manual/setup input while allowing native return when released. Fakie return can wait for neutral controls or also request return after landing. Saved game preferences stay intact.

These are dual-stick translations of Skate's controls; they do not reproduce another game's physics. The v6.5 fixes are retained alongside v6.6 pop-to-air handoff, release-pop/steering compatibility, rear-stick rail circles, held setup across brief ground contact and Left/Right Foot tracking. Native grind target/alignment requests are opt-in (NativeGrindEntry=0 by default). Skate controls rotation speed; old GestureSpeed settings remain ignored.

Console: dualstick status; dualstick on; dualstick off; dualstick preset session|xl; dualstick save 1; dualstick load 1; dualstick test. Status includes the longest observed tick gap for testing.

## Testing status and limits

44 ported regression suites pass, including the 36,864-combination pop/setup matrix and the new rail/contact/state-701 cases. Across 852,927 samples, buttons, triggers and checked state flags match native 6.6 exactly; maximum stick error is two raw units. Details are in VALIDATION.md. Lua lifecycle tests cover initialization, the real menu and console, presets, pause/resume, held-input handoff, disconnect/reconnect, rider changes, output conflicts and release. The general native API regression suite passes, with 15 optional maintainer-plugin fixtures absent.

**This has not been tested in-game.** The native additions compile, but actual XInput/PlayStation hardware, animation graph observations, camera sides, rail acceptance and stance/manual chains still need prerelease testing. Luau samples once per plugin tick, whereas the companion translates individual controller polls. Very short gestures can be missed between ticks; slow frames may expire the 100 ms native output lease. Successful pure-state comparisons do not establish equal latency or gameplay behavior.

Default gameplay controls are ported. Windows updater/overlay/hotkey diagnostics, native pose-reset experiments and the optional ManualCatchNativeRanges asset experiment are not provided by this Lua package. The asset-range flag is explicitly disabled. The ReSkate plugin page replaces the old companion's End menu.

ReSkate supports the pinned Skate build 20260929. Windows compilation is verified; Proton and macOS behavior of this port are unverified.

## Credits and license

GoalTrains — original Dual Stick Controls. zee_x64 — ReSkate Lua settings port and API guidance. ReSkate contributors — host/runtime. Prepared with an AI coding assistant. GPL-3.0; see LICENSE.

## Changes since 6.5

The Lua port includes the control fixes in the [6.6 release](https://github.com/GoalTrains/reskatedualstick/releases/tag/6.6). Pop handoff and rear-foot circle fixes apply with FOOT SETUP, air controls and BLOCKED single-stick flips. ALLOWED single-stick flips retain their existing paths. The SDK provides only generic input/rider operations; all foot/grind behavior is implemented by this plugin.
