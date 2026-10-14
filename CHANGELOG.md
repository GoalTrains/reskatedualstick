# v5.128 — changes since v5.108

- Added Skater XL as a third layout alongside Front/Back Foot and Left/Right Foot.
- XL keeps each stick's setup direction consistent while ollie/nollie jobs swap in switch.
- Regular: right DOWN = ollie; left UP = nollie. In switch: right DOWN = fakie/nollie; left UP = switch ollie.
- Goofy: left DOWN = ollie; right UP = nollie. In switch: left DOWN = fakie/nollie; right UP = switch ollie.
- The XL preset selects Release pop, physical left-stick steering and allowed single-stick flips.
- Corrected XL's outgoing switch inputs for regular and goofy.
- Added resynchronisation of stale XL foot jobs when movement and the game's stance agree. Evidence gathers while skating; a correction applies only with neutral foot inputs and does not respawn you.
- Updated control diagrams and custom-preset saving for the XL layout.
- Improved local-player tracking after respawning and cleared stale foot, pump and wind-up inputs.
- Gameplay hitches no longer temporarily switch to stock controls when menus are closed.
- The on/off shortcut is LB + RB + L3 + R3 held for three seconds.
- Experimental stuck-pose recovery is enabled. It may briefly stop speed and visibly reset the skater.
- Expanded input logging; large pose diagnostics remain off by default.
- Retains the replay-editor and native-menu controls fixes.

## Installation

Close Skate and replace `version.dll` beside `Skate.exe`. Keep the official `ReSkate.dll`.
A fresh `session_controls.ini` is recommended. Back up the old INI first: replacing it resets settings and custom presets.
Launch, press End and choose Session-style or Skater XL-style. The included INI starts with Session-style controls.
