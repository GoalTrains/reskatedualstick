# v5.133 — changes since v5.128

- Improved stale stance and foot-job correction for regular and goofy, including after turns, menus and remounts.
- Separated physical foot ownership from Skate's board/input frame and animation mirror to prevent incorrect stick swaps.
- Added Disable hold-fakie under Custom, enabled by default. Skate's Hold Fakie Stance and Hold Fakie Push read as off while Dual Stick is enabled. Your saved game preferences are preserved.
- Automatic pose resets are now off by default, avoiding the abrupt stop and visible recovery reset.
- Improved grind and stall bump-outs: held board controls release during the exit, and an already-held exit direction gets a brief neutral re-arm before the bump.
- Reduced stick-steering activation from 35% to 15% and release threshold from 25% to 10%.
- Fresh settings use a linear steering curve for more immediate turning. Foot-trick and scoop protections remain.
- Added optional rolling stance diagnostics; heavy diagnostics are off in the fresh settings.
- Keeps the replay editor, radial menu and native menu input fixes.
- Standalone release: End opens Dual Stick; Insert stays with official ReSkate. Proposed ReSkate menu integration is separate.

## Earlier changes since v5.108

### v5.128 — changes since v5.108

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
- Added experimental stuck-pose recovery (now disabled by default in v5.133).
- Expanded input logging; large pose diagnostics remain off by default.
- Retains the replay-editor and native-menu controls fixes.

