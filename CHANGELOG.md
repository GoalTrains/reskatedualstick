# Changelog

## v5.89 TEST - changes since v5.88

- Fix an opposite release pop being generated when the second foot of a
  two-foot flick returns to centre in Release pop mode. Keep the flick output,
  then return to neutral instead of treating the flick as a fresh crouch.
- Preserve gesture identity through overlapping inputs, setup release and
  flick in the same poll, and a short neutral gap within the existing release
  pulse. Both sticks returning to centre rearms normal one-foot release pops.
- Cover regular/goofy, normal/switch, both foot layouts and ollie/nollie inputs.
  No new activation delay or stance-mapping change.

Nineteen native regression executables passed. The reproduced sequence now
ends in neutral instead of an opposite pop. In-game confirmation remains pending;
Linux/Proton gameplay is untested. Keep existing settings/custom presets.

## v5.88 TEST - changes since v5.87

- Start the menu window thread when the mod loads, independently of controller
  detection. Restarting with an unavailable/disconnected pad no longer prevents
  keyboard menu startup.
- Process menu hotkey, keyboard navigation, settings, control-page scrolling
  and hotkey rebinding on the window thread, including when the game stops
  polling controllers. Controller input still navigates and is blocked while
  the mod menu is open; official ReSkate Insert ownership remains respected.
- Log window-thread startup failure and allow a retry on a later controller poll.

Eighteen native regression executables passed. Affected-device confirmation and
Linux/Proton gameplay remain pending. Keep existing settings/custom presets.

## v5.87 TEST - changes since v5.86

- Stop stale held/spent foot flags from indefinitely blocking stick powerslides.
  Keep a brief 300 ms guard after actual vertical setup inputs to protect tricks.
- Allow strong opposing-stick slides even when the thumbs arrive more than
  220 ms apart. Eligible slides still activate without an added delay.
- Recover R3 pumping while the click is held when the sticks return to centre
  or the skater lands. Clear queued body-flip state when pumping takes over.
  Airborne body flips and rail exits retain their existing controls.

Seventeen native regression executables passed, including reproduced stale-foot,
asynchronous-thumb and held-click misses. Gameplay confirmation remains pending.
Keep existing settings/custom presets. Linux/Proton gameplay remains untested.

## v5.86 TEST - changes since v5.85

- Restore trigger steering immediately after a stock-mode grab lands, including
  a trigger held through touchdown. Seed trigger edges to avoid a new revert.
- Let held front/back feet pivot sideways as soon as takeoff is confirmed,
  rather than waiting for the delayed air-control flag. Held trick preservation
  and vertical crouch/manual guards remain.
- Apply the selected grind sensitivity to sideways air board adjustments too,
  for consistent entry/rail response; vertical manual strength is unchanged.
- Filter transient ground turn-flag changes for 75 ms. Airborne turns and
  revert states are immediate. Stable genuine ground changes still register.
- Read stance probes once per input call, remove redundant VirtualQuery checks
  from checked ReadProcessMemory reads, and sample motion only during actual
  remount calibration. Invalid pointers still fail closed.
- Add setup/menu/stance/translation/tail timing columns to the precise trace,
  so remaining input cost can be identified rather than inferred from FPS.

Sixteen native regression executables and an actual stance-read cache fixture
passed. The changes reduce repeated work and fix demonstrated handoff blocks;
in-game latency improvement still requires measurement on the affected device.
Keep existing settings/custom presets. Linux/Proton gameplay remains untested.

## v5.85 TEST - changes since v5.84

- Fix DualSense report framing in both native mode (PlayStationPads=2) and
  direct mode (PlayStationPads=1). Long USB reads no longer select the short
  Bluetooth layout, which could invent D-pad Up and L2=8 and keep stock
  radial-menu passthrough active.
- Validate consecutive 64-byte USB reports before processing a batch, including
  the 384-byte size seen in the affected log. Also accept one USB report with
  zero padding. Unsupported lengths/framing are left untouched; native mode
  logs the first skipped report ID and byte count.
- Preserve true 10-byte Bluetooth simple, 78-byte full Bluetooth and DualShock
  layouts. Keep native sensor, touchpad, haptics and trigger data outside the
  translated control fields.
- Add framing and translator regression checks reproducing the false input.

This fixes a demonstrated decoder defect. The user's 384-byte payload was not
provided, so gameplay confirmation on the affected controller is still required.
No stance, menu, steering curve or preset defaults changed. Keep your existing INI.

DUAL STICK CONTROLS v5.84 TEST - changes since v5.82

- Remove the redundant Controls: first page row; keep right-hand diagrams
  and LB/RB paging. Adjust row counts and submenu back navigation.
- Prevent the regular-stance motion correction treating a fresh spawn on foot
  as a backwards remount. Only an actual dismount after riding arms it.
- Cancel unfinished remount calibration on takeoff or after two seconds, so
  motion after a trick cannot establish an inverted reference.
- Distinguish successful remount calibration from cancelled calibration in logs.

Regression coverage replays spawn-on-foot/mount/tricks/backwards-motion and
interrupted-remount sequences. Regular backwards remount correction still needs
steady ground movement before the first trick. It remains regular-only; goofy
retains its existing stance route. In-game confirmation remains required.
Existing saved settings and presets are preserved.

## v5.82

- Keep controls visible beside settings across all menus, with LB/RB and Page Up/Page Down paging.
- Fit the companion menu to the game window, including smaller handheld displays.
- Add controller paging and window-size regression checks.

DUAL STICK CONTROLS v5.81 TEST - changes since v5.77

- Stop the experimental remount stance reference recalibrating during normal
  riding. Once established it remains fixed until dismount or respawn, while
  the native turn flag continues to drive turns. This addresses regular True
  Foot unexpectedly changing physical stick jobs without a remount.
- Retain the corrected riding direction/native input-frame split for remounts.
- Add Hardflip and 360 Hardflip cards; Controls now has four pages.
- Correct scoop diagrams: tre right-down-left; laser and 360 hardflip
  left-down-right. Keep the confirmed kick/heel arrows; hardflip flicks left.
- Correct varial/hardflip setup sides and show the full 360 hardflip sweep.
- Revert and grab descriptions now reflect the selected settings.

Regression coverage includes reference changes during ordinary riding and
regular True Foot's normal/switch physical-stick ollie inputs. Eighty diagram
sequences were checked through the actual translator across stances/layouts.
In-game confirmation of the stance correction is still required.
Existing saved settings and presets are preserved.

Thirteen native regression executables passed. The True Foot correction is
installed for testing but still needs in-game confirmation. Linux/Proton has
not been independently tested. No ReSkate.dll or custom PR runtime is included.
When updating, copy version.dll and keep your existing session_controls.ini.

DUAL STICK CONTROLS v5.77 TEST - changes since v5.76

- Disable the companion's DXGI graphics hooks completely, including when
  ReSkate has not loaded yet. Old MenuInGame=1 settings cannot re-enable them.
  This closes the remaining startup load-order risk to Insert/mods/maps.
- Attach the settings window to the game instead of making it an independent
  desktop-topmost window. Create it only when the game is in front.
- Hide the menu and ignore hotkeys/navigation while another application has
  focus. Keys held while returning to the game cannot trigger fresh actions.
- Recreate or reattach the menu when the startup/game window is replaced.
- Retain the v5.76 regular-stance remount fix and existing gameplay mappings.

Thirteen native regression executables passed, including actual Windows menu
ownership/focus/lifetime checks. Affected players still need to confirm Insert
and missing menu items; these changes address remaining interference risks,
not a verified diagnosis of every reported failure.

The menu remains on End (or your saved custom hotkey). In exclusive fullscreen,
use borderless/windowed mode if the separate menu is hidden. This release
contains no ReSkate.dll or custom PR runtime. Linux/Proton remains untested.
Keep your existing session_controls.ini and custom presets when updating.

DUAL STICK CONTROLS v5.76 TEST - changes since v5.73

- Fix regular-stance foot mapping after getting off the board and remounting
  switch. Use steady ground movement to recover the riding direction when
  the game's existing switch flag has reset.
- Separate riding direction from Skate's native trick-input frame. A remount
  no longer makes the same setup/flick order swap fakie and switch ollies.
- In the confirmed regular Left/Right Foot + Switch ON + Flick setup:
  left stick UP first, then right stick DOWN remains the fakie input after
  a 180 and after a switch remount.
- Enable the correction for fresh installs and existing INIs. Set
  RemountStanceMotion=0 to return to the previous stance reader.
- Add bounded remount diagnostics for troubleshooting; OFF by default.
- Retain the v5.73 ReSkate Insert-menu compatibility changes.

After remounting, roll straight briefly before testing: the correction needs
steady ground movement. It rejects stationary, sideways, airborne and
teleport samples. Regular stance was confirmed in game; goofy retains its
existing reader and this remount correction is not enabled for goofy.

Twelve native regression executables passed, plus recorded-snapshot replay.
Linux/Proton gameplay has not been independently tested with this build.
These are standalone mod zips: no ReSkate.dll or custom PR runtime included.
When updating, keep your existing session_controls.ini and custom presets.
DUAL STICK CONTROLS v5.73 TEST - changes since v5.71

- Prevent the companion welcome/settings menu from installing competing DXGI
  graphics hooks while ReSkate is loaded. This targets the startup conflict
  that can prevent Insert, the mod manager and map browser from appearing.
- Use the separate companion menu window with ReSkate, including old INIs
  containing MenuInGame=1. End/custom hotkey and startup opt-out still work.
- Yield companion controller input while the official ReSkate Insert menu is
  visible, using its existing status export. Poll on the menu worker to avoid
  locking the render thread from a controller callback.
- Close the companion menu when ReSkate takes ownership. Clear generated
  gestures and guard held foot inputs until release when controls return.
- Support the extended input-ownership export in the experimental ReSkate
  integration, when present. Stock ReSkate does not require that PR build.
- Gameplay mappings and Session/XL preset defaults are unchanged from v5.71.

The proposed native Dual Stick settings page remains a separate PR project;
it is not included in these standalone zips. No ReSkate.dll is distributed.
Build and nine native control regressions passed. In-game Insert/menu access
still needs confirmation; Linux/Proton gameplay has not been independently tested.

