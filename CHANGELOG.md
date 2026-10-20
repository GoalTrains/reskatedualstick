## Unreleased v6.4 - XL fakie air controls and native flip speed

- Corrects XL airborne board/leg adjustment direction in mirrored fakie animation frames. Retains the takeoff frame through a jump; ground tricks, rail controls and reverts keep their existing handling.
- Removes the v6.3 physical stick-speed override entirely. Skate controls trick rotation speed again; old GestureSpeed keys are ignored.
- Manual Catch now displays ON/OFF, without the TEST tag. Its existing behaviour and limitations remain unchanged.

## v6.3 - Manual Catch improvements and stick-speed rotation

Manual Catch
- Refines catch timing and the handoff from your flick or scoop into the held input, reducing unintended automatic catches, including on frontside and backside shuvs.
- Manual Catch is in its best working state for now, within Skate's current trick behaviour. Not every trick keeps flipping or spinning while held; some settle into an angled pose instead. A clean result still depends on catching at the right moment. A missed or mistimed catch can end in an awkward-looking automatic catch.
- Click L3, R3 or both while airborne to catch. Manual Catch remains optional and OFF by default; enable it under End > Custom > Manual catch.
- This uses Skate's shared catch behaviour. Independent front/back-foot catches, dark catches and forced bails for missed catches are not implemented.

Stick-speed rotation
- Adds rotation-speed adjustment based on how quickly you move the sticks: slower flicks and scoops request slower trick rotation, while faster movements request faster rotation.
- Works during normal gameplay as well as with Manual Catch enabled. Speed is measured from your physical stick movement, independently of the held catch input.
- Holding a completed flick or releasing the stick no longer drags its measured speed down.
- The effect depends on the trick and Skate's animation timing; the response can be less noticeable on some tricks, including scoops.
- Enabled by default. To disable it, set GestureSpeed=0 under [SessionControls] in session_controls.ini; GestureSpeed=1 enables it. Restart Skate after changing this setting.

## v6.2 - D-pad camera controls and Custom menu cleanup

- Tap D-pad left or right to select that camera side. Repeating the same direction keeps it there.
- Hold left/right for 0.5 seconds to use the original D-pad action.
- D-pad camera sides is ON by default. Change it under End > Custom; an explicitly saved OFF preference is respected.
- The option automatically enables Switch side controls and disables Dynamic side switching while Dual Stick is on, preserving your saved game preferences.
- Physical L3 no longer switches camera sides while this option is on. The mod still receives L3 for catch, body-flip, grind-exit and shortcut handling.
- Reorders Custom, removes diagnostic recording from the menu, and keeps legacy/switch compatibility options near the bottom.
- Anchors the explanation box at the bottom and displays up to 20 settings with a scrollbar and visible option range.
- Native game menus retain normal controller navigation.

## v6.1 - Early manual-catch attempt (stable release)

Changes since v6.0:

- Refines two-foot grind entry and on-rail handling, including frontside/backside approaches, tail/nose support, and grind exits. This is the latest tested grind work after v6.0.
- Improves XL stance and input-frame handling after no-complys, automatic reverts, remounts, and grounded stance corrections. Keeps the live game input frame separate from the physical-foot mapping.
- Adds an optional manual-catch experiment under **Custom**. It attempts to keep a flip or scoop input held after stick release; tap L3, R3 or both in the air to release the held input for a catch.
- Manual catch is OFF in fresh settings (`ManualCatch=0`). It is not part of the Session or XL presets; enable it only from Custom.
- Catch logging and native-state snapshots are bounded and read-only; diagnostics remain OFF by default.
- The optional updater treats the author's private v6.13-v6.15 test builds as test builds so they can update to this first public release since v6.0; existing settings are preserved.
- This is an early attempt, not a complete catch system. Many tricks, especially scoop and combo tricks, may still release instead of holding. Separate front/back catches, dark catches and forced bails are not implemented.

## v6.10 - Local catch takeoff correction and landing diagnostics (included in v6.1)

- Record named native dark, board-upright, landing, landing-on-board, landing-on-edge, preland-angle and wipeout flags alongside catch clicks. Each field is validated independently; unavailable values remain -1.
- Fresh read-only samples on L3/R3/bumper edges and physics-state changes, otherwise limited to 50 Hz. Record sample age and sampled state.
- Preserve scoop holds through pop state 103: prevent recorded 40-70 ms neutral gaps before airborne, while allowing the active scoop to finish.
- Record validated native CurrentFlipTrick enum to separate which held-trick family Skate actually recognized.
- No automatic dark catch or forced bail yet; the flags need validation in gameplay. Trace remains bounded to 16 MiB plus one previous file.

## v6.9 - Local scoop-catch experiment (included in v6.1)

- Include single-stick shuv, 360-shuv and impossible scoop paths in manual-catch detection, including with single-stick flips blocked.
- Preserve the circular input through the pop state before holding its endpoint in the air.
- Added candidate kind and scoop angle to the bounded trace for gameplay verification.

## v6.8 - Local held-catch experiment (included in v6.1)

- Optional Custom-menu manual catch: automatically retain the emitted flip input until a fresh L3, R3 or both click in the air.
- Catch clicks take priority over body-flip clicks while a flip is held; landing, menus and disabled controls clear the hold.
- Uses shared native held-trick release; requested foot is recorded, but independent one-foot catch animations are not implemented.
- Added bounded trace fields for enable state, hold/catch phase and requested foot mask.

v6.0 - Stable release

New optional Windows updater
- Open DualStickLauncher.exe (or a shortcut named ReSkate + Dual Stick) to check stable GitHub releases before the official ReSkate launcher opens.
- Shows your installed version, the available version, and scrollable patch notes before asking whether to update.
- Update now installs the verified DLL and confirms completion. Skip this release keeps your current build until a newer release appears. Disable update checks stops checking; closing the prompt skips only this launch.
- Preserves existing INI/custom presets and saves the previous DLL for rollback.
- Checks zip and DLL hashes, skips running games and newer local builds, and keeps the installed mod if checking or downloading fails.
- Does not replace ReSkate.dll or ReSkateLauncher.exe, and does not change ReSkate update settings. No upstream PR is needed.

Existing controls included
- Includes the two-foot grind setup, frontside/backside rail-entry work, nose/tail pressure handling and grind exits from v5.158.
- Includes the XL no-comply/revert/remount input-frame corrections, switch-fakie mirror compensation, 360 continuity and bounded logging.

Manual installation
1. Close Skate and open the folder containing Skate.exe and ReSkateLauncher.exe.
2. Back up version.dll and session_controls.ini.
3. Copy version.dll from the Windows or Linux mod zip beside Skate.exe. Keep official ReSkate.dll.
4. Keep your INI from v5.133 or newer. For older installs, back up the old INI before using fresh settings; this resets settings and saved presets.
5. Fresh settings use Session-style and FOOT SETUP grinds. Press End to choose XL if wanted. With an existing INI, enable Custom > Grind setup > FOOT SETUP to use the two-foot grind work.
Linux/Steam Deck: follow the Linux zip's Proton instructions. The optional updater is Windows-only.

Optional updater installation (Windows, once)
1. Download Dual_Stick_Auto_Updater_Windows.zip in addition to the mod zip.
2. Copy DualStickLauncher.exe, dual_stick_updater.json and the updater folder beside ReSkateLauncher.exe. If already installed, retain your existing dual_stick_updater.json preferences.
3. Open DualStickLauncher.exe; optionally create a shortcut named ReSkate + Dual Stick.
4. Use this shortcut for future launches. Opening ReSkateLauncher.exe directly bypasses mod update checks.
The updater itself is a separate optional download; future mod releases do not need it reinstalled unless the updater changes. Set enabled to true in dual_stick_updater.json to re-enable checks after disabling them.


v5.158 - Stable release

Includes the two-foot grind work omitted from v5.157, plus the XL fixes.

Grinds
- Adds FOOT SETUP grind controls: front and back sticks position the board during the approach and on the rail.
- Includes the latest frontside/backside entry handling and tail/nose pressure corrections.
- Holding the tail supported no longer flattens the board just because the back-foot stick moves sideways. Front-foot input can lower the nose into a two-foot grind.
- Includes the native rail-entry correction, held trick/setup handoff, and split-control grind exits.
- FOOT SETUP is enabled in fresh settings. With an existing INI, choose Custom > Grind setup > FOOT SETUP (SplitGrindControls=1). Keep your existing preferences if already testing it. For dedicated two-foot setup, block single-stick flips.

XL controls
- Keeps physical stance tracking active when the mod's Regular/Goofy setting differs from Skate's setting.
- Separates Skate's live input frame from physical foot jobs after no-complys, automatic landing reverts and remounts.
- Synchronises confirmed grounded stance corrections and restores switch-fakie mirror compensation in the affected setup.
- Retains continuous XL air/rail adjustments for 360s.

Other
- Precise input logging stays capped at 16 MiB plus one 16 MiB previous file. Heavy native grind diagnostics are off.
- The separate ReSkate menu PR is not included. End opens Dual Stick; Insert opens official ReSkate.

Installation
Close Skate. Replace version.dll beside Skate.exe. Keep official ReSkate.dll.
Keep a v5.133 or newer session_controls.ini and custom presets. To try FOOT SETUP with an older INI, enable it in Custom as described above. Fresh settings start in Session-style with FOOT SETUP enabled; choose XL from the menu if wanted.
Linux/Steam Deck instructions are included in the Linux zip.


v5.157 - Stable release

Changes since v5.136 preview
- Fixes an XL stance-reader restriction that disabled physical stance tracking when the mod's Regular/Goofy setting differed from Skate's setting. This affected the supplied goofy setup and left no-complys and automatic landing reverts using the older tracker.
- Keeps Skate's live input frame separate from the physical foot jobs after no-complys, reverts and remounts.
- Keeps the retained XL stance in sync after a confirmed grounded correction.
- Restores native mirror compensation in the affected setup, addressing reversed kickflip/heelflip directions on switch fakie tricks.

Included from v5.136 (since stable v5.134)
- Consistent XL leg-adjustment directions throughout continuous airs and grinds, including full 360s.
- Absolute physical stance tracking rather than counting an absolute flag as another turn.
- Separate native mirror correction for ground tricks and continuous board adjustments.
- Precise input logs capped at 16 MiB plus one 16 MiB previous file. Duplicate timing-only rows are suppressed.

Installation
Close Skate. Replace version.dll beside Skate.exe. Keep the official ReSkate.dll.
Keep your v5.133 or newer session_controls.ini and custom settings. Older installs should back up their INI before using the supplied fresh one.
Fresh settings start in Session-style; press End and choose Skater XL-style if wanted.
Linux/Steam Deck uses the same DLL through Proton; see INSTALL.txt for launch options.

This is the standalone release. Local experimental two-foot grind changes and the separate ReSkate menu PR are not included.


v5.136 preview - changes since v5.134

- XL now treats validated physical stance as an absolute stance, rather than counting it as another turn relative to takeoff.
- Keeps XL board adjustment directions consistent through continuous air and rail sequences. Fresh tricks after landing use the updated stance.
- Adds native animation-mirror compensation for ground trick inputs, separate from air and rail adjustments.
- Caps the precise input CSV at 16 MiB plus one 16 MiB previous file. An oversized existing CSV is reduced to complete recent rows on the next launch.
- Stops timing jitter from filling the CSV with otherwise identical input rows. Timing data remains on recorded rows, with a 100 ms heartbeat.
- Adds physical-stance and board-frame evidence to the CSV for clearer diagnosis.
- Includes the v5.134 rail shove direction fix.

Installation
Close Skate and replace version.dll beside Skate.exe. Keep official ReSkate.dll.
You can keep your v5.133 or newer INI; this update needs no new settings.
For older installs, back up the INI before replacing it with the fresh one, which resets settings and custom slots.
The fresh INI starts with Session-style controls; press End to select Skater XL-style if wanted.
This preview has passed automated and recorded-input checks; in-game confirmation is pending.

# v5.134 - changes since v5.133

- Fixed quick shove-outs using the reversed tail-pivot input frame while grinding.
- A quick scoop from the foot's pop direction now uses the same trick-input mapping as a flat-ground shove-it, for front and back feet.
- Held sideways positions and gradual circular movements remain grind adjustments. Quick scoops are distinguished by their movement; there is no extra button or activation timer.
- Covers regular/goofy, normal/switch, all three layouts and Flick/Release pop styles.
- Added precise grind-pop and scoop-mode logging to make future reports easier to diagnose.
- Keeps the v5.133 steering, bump-out, hold-fakie and replay/menu fixes. Automatic pose resets remain off by default.

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

