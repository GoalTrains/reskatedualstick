<a href="https://ko-fi.com/X8K128ZWSN" target="_blank"><img height="36" style="border:0px;height:36px;" src="https://storage.ko-fi.com/cdn/kofi6.png?v=6" border="0" alt="Buy Me a Coffee at ko-fi.com" /></a>

# Dual Stick Controls for reSkate

Play Skate with two foot sticks using **Session-style**, **Skater XL-style**, or your own **Custom** settings.
The mod translates Skate's controls across two sticks; it does not replace the game's skating physics.

## Early manual-catch attempt (v6.1)

Custom > Manual catch (early attempt) tries to keep a flip or scoop input held after the physical stick is released. Tap L3, R3, or both while airborne to release it for a catch. This is an incomplete experiment: many tricks still do not hold reliably, and independent foot catches and dark catches are not implemented. It is OFF by default; enable it only if you want to try it.

## Stable download v6.1

**v6.1 is the first public update since v6.0.** It includes refined grind handling, XL stance/input-frame fixes, and an early manual-catch attempt. Manual catch is OFF by default and available under Custom. The optional updater can update the author's private test builds to this release.

Fresh settings enable **FOOT SETUP**. With an existing INI, enable it under **Custom > Grind setup > FOOT SETUP** (`SplitGrindControls=1`). For dedicated two-foot setup, block single-stick flips. Each stick contributes to board position; deliberate front-foot input can lower the nose while keeping tail support.

| Platform | Download |
| --- | --- |
| Windows | [Download Windows zip](https://github.com/GoalTrains/reskatedualstick/releases/download/6.1/Dual_Stick_Controls_v6.1.zip) |
| Linux / Steam Deck (Proton) | [Download Linux / Steam Deck zip](https://github.com/GoalTrains/reskatedualstick/releases/download/6.1/Dual_Stick_Controls_v6.1_Linux.zip) |

[Release notes](docs/RELEASE_6.1.txt) · [Full changelog](CHANGELOG.md) · [Windows instructions](docs/INSTALL_WINDOWS.md) · [Linux instructions](docs/INSTALL_LINUX.md)

Changes since v6.0: refined grind handling, XL stance/input-frame corrections, and an optional early manual-catch attempt with bounded, read-only diagnostics. See the release notes for known limitations.

## Automatic updates (Windows)

[Optional auto-updater download](https://github.com/GoalTrains/reskatedualstick/releases/download/6.1/Dual_Stick_Auto_Updater_Windows.zip). Copy its contents beside `ReSkateLauncher.exe`, then open **DualStickLauncher.exe** (or a shortcut to it) to check stable releases before the official launcher opens. Read the patch notes, then choose **Update now**, **Skip this release**, or **Disable update checks**.

It preserves your INI, verifies download hashes, keeps the previous DLL, skips running games/newer local builds, and opens ReSkate even if GitHub is unavailable. It leaves the official launcher and its update settings unchanged. Opening ReSkateLauncher.exe directly bypasses this optional updater. [Details and opt-out](updater/README.md).

## Install or update

1. Close Skate completely.
2. Open the game folder containing `Skate.exe`.
3. Back up your existing `version.dll` and `session_controls.ini`.
4. Copy the downloaded `version.dll` beside `Skate.exe`.
5. **Updating from v5.133 or newer:** keep your INI. For older installs, a fresh INI is recommended; back up before replacing it, because this resets settings and custom preset slots.
6. Launch through your normal reSkate setup. Press **End** and choose **Session-style** or **Skater XL-style**.

The fresh INI starts with Session-style controls. Select XL in the menu to use its new layout.
Keep the official `ReSkate.dll`; these downloads contain the standalone mod.

To retain your settings instead, keep your INI and reapply the XL preset if you want XL controls.
Hold-fakie compatibility is on by default. Automatic pose resets are off; see the compatibility notes below.

## Choose your controls

| Preset | Layout | Pop style | Stick steering | Single-stick flips |
| --- | --- | --- | --- | --- |
| Session-style | Front/Back Foot | Flick | Off; triggers turn | Blocked |
| Skater XL-style | Skater XL | Release | Physical left stick | Allowed |
| Custom | Your choice | Flick or Release | Your choice | Your choice |

**Front/Back Foot:** left always has the front job and right always has the back job.

**Left/Right Foot:** each stick represents its physical foot. Front/back jobs swap in switch, using front-up/back-down setup directions.

**Skater XL:** each physical stick keeps its setup direction while its ollie/nollie job swaps in switch.

| Skater | Input | Normal stance | Switch |
| --- | --- | --- | --- |
| Regular | Right down | Ollie | Nollie / XL's fakie ollie |
| Regular | Left up | Nollie | Switch ollie |
| Goofy | Left down | Ollie | Nollie / XL's fakie ollie |
| Goofy | Right up | Nollie | Switch ollie |

With Release pop, hold the setup then let go. Opposite-foot flicks also work.
The diagrams follow the selected layout and detected stance. These are inspired profiles; other Skate inputs still differ from Session and XL.

## Open and use the menu

- **End:** open Dual Stick settings; its opening hotkey can be changed under Menu settings.
- **Insert:** open the official ReSkate menu.
- D-pad / arrow keys: navigate. A / Enter: select. B: go back.
- LB / RB or Page Up / Page Down: browse control diagrams.
- Custom contains the gameplay options and five save/load preset slots.
- Menu settings lets you disable the startup menu and adjust its size.
- Hold **LB + RB + L3 + R3** together for three seconds to toggle Dual Stick.

R3 remains the pumping crouch. With the foot sticks centred, hold it to crouch and release with your pumping timing.


## Menu guide
The home page has three large choices: **Session-style**, **Skater XL-style**, and **Custom**. Applying either style changes the core control settings. Editing a core setting changes the active profile to Custom; it does not create a saved slot automatically.
The remaining home-page options are **Dual Stick controls**, **Stance**, **Stock ramp controls**, **Menu settings**, and **Close**. The control diagrams stay alongside the settings; use LB/RB to browse them. There is no separate Controls page to open.
### Custom: choose how the sticks behave
| Option | What it changes |
| --- | --- |
| Stick layout | FRONT/BACK keeps left = front and right = back. LEFT/RIGHT FOOT follows physical feet with front-up/back-down setup. SKATER XL keeps each physical stick's setup direction when riding switch. |
| Pop style | FLICK: hold the setup foot, then flick the other. RELEASE: hold a full setup, then release; opposite-foot flicks also work. |
| Stick steering | OFF, FRONT FOOT, or LEFT STICK. Steering uses the chosen stick while the other is centred. Triggers still turn. Centre the steering stick before starting a foot trick. |
| Stick steering angle | Steering starts within 5-60 degrees of straight sideways. Lower values leave more diagonal room for scoop setups; try 20 if goofy tre-flip setups steer by accident. Default 34 keeps the original zone. Centre to end steering. Saved in custom presets. |
| Allow single-stick flips | BLOCKED requires two-foot inputs for flip tricks. One-stick shove-its, 360 shove-its and impossibles remain available. ALLOWED also permits single-stick flips. |
| Switch flick / scoop directions | STANDARD or REVERSED for left/right foot gestures in switch. Does not swap foot ownership or reverse steering. |
| Switch up / down directions | STANDARD or REVERSED for vertical foot gestures in switch. Does not swap which foot each stick controls. |
**True Foot is the old name for physical-foot job swapping.** Choose **Left/Right Foot** or **Skater XL** for that behavior. The distinction is their setup directions, as explained above. Front/Back already keeps its jobs consistent through a 180.
### Custom: skating options
| Option | What it changes |
| --- | --- |
| Reverts | OFF, DOUBLE TAP on either trigger, or BOTH TRIGGERS. With both triggers, the first pressed chooses the direction. |
| Revert double-tap window | Time allowed between taps: 100-600 ms. A larger window is easier to activate; this changes tap recognition, not the speed of the revert. |
| Spin wind-up | Enables trigger pre-wind while crouched before takeoff. |
| Crouched trigger handling | NORMAL uses the existing wind-up handling. LIGHT STEERS lets weaker pulls turn and stronger pulls wind up; easing below the threshold cancels the wind-up. |
| Wind-up trigger strength | Threshold for LIGHT STEERS, from 50-90%. Does not change ordinary turning sensitivity. |
| Body flips | Front/back foot stick clicks request frontflips/backflips while foot-crouched or airborne. R3 pumping and grind exits are separate. |
| Air controls | Moves each board end with its foot stick in the air and on rails. To pop out of a rail or stall, prepare a foot and flick the opposite foot. Bump-outs release held board controls and re-arm a direction that was already held. |
| Powerslides | Both sticks inward for toe-side, outward for heel-side. No activation delay; add vertical movement to adjust the approach angle. |
| Rail shove-outs | Quickly scoop from a foot's pop direction to use the same trick-input direction as on flat ground. Held sideways positions and gradual circles remain rail adjustments. |
| Grind bump-out | Tilt a stick up/down and click it to exit a rail or lip. With centred sticks, L3 exits left and R3 exits right. |
| Legacy switch mode | Compatibility option shown for physical layouts. SWAP JOBS is the physical-foot behavior. KEEP JOBS or OFF changes the layout to LEGACY L/R; start with a named layout instead. |
| Ramp diagnostic recording | OFF, RAMP or FLAT. These labels describe the attempts you record; they do not select a control layout. Leave OFF for normal play. |
| Disable hold-fakie | Temporarily disables Skate's hold-fakie stance and push reads while Dual Stick is enabled; saved Skate preferences are preserved. |
| Saved custom presets | Opens five slots for saving and loading your settings. |
### Save your own preset
1. Apply the closest style, then adjust options in **Custom**.
2. Open **Saved custom presets** and choose **Custom slot** 1-5.
3. Select **Save current settings**. For an occupied slot, select Save again to confirm replacement.
4. To restore it later, select that slot and **Load saved settings**.
Settings are written to `session_controls.ini`. Loading a saved preset preserves this installation's controller connection, menu preferences and regular/goofy selection. Back up the INI before replacing it; a fresh INI clears the slots.
### Stance and ramp controls
**Stance** is your skater's base regular/goofy setting, not a manual normal/switch toggle. AUTO follows the game's configured stance; REGULAR or GOOFY overrides it.
**Stock ramp controls** is separate from the Session/XL preset:
| Mode | Behavior |
| --- | --- |
| OFF | Keep dual-stick foot controls. |
| AIR ONLY | On a qualifying ramp air, a trigger grab activates stock air handling until landing. |
| EARLY ON RAMP | Activate stock handling during a qualifying ramp climb, allowing stock setup/pre-wind, and retain it through the air until landing. A cancelled approach returns to foot controls. |
During stock ramp handling, triggers grab and the sticks use Skate's handling. R3 remains pumping. The transition detector uses an experimental tilt cue; it is intended to keep flat-ground tricks in dual-stick controls.
### Menu preferences
**Menu settings** contains **Show menu on startup**, **Menu hotkey**, and **Menu size** (75-250%). Select Menu hotkey and press your preferred keyboard key; Left restores End, and Escape/B cancels binding.
Use D-pad/arrow keys to move, A/Enter to select, and left/right to change values. B returns from a subpage; on the home page it closes the menu. End controls this standalone menu; Insert remains the official ReSkate menu.

## Stance compatibility
**Custom â†’ Disable hold-fakie** is ON by default. While Dual Stick is enabled, this makes Skate read **Hold Fakie Stance** and **Hold Fakie Push** as off. Your saved Skate preferences are preserved; turning the option or Dual Stick off restores the normal reads. This applies to regular and goofy.
The foot-job correction distinguishes riding direction from Skate's input frame and animation mirror. It preserves the physical-foot reference through menus and remounts.
Automatic pose resets are **OFF** by default so they do not interrupt skating or stop your speed. If keeping an older INI, set these under `[SessionControls]`:
```ini
DisableHoldFakie=1
NativeMenuRespawnRecovery=0
NativeGroundPoseRecovery=0
SteerCurvePercent=100
```
Large pose and flight diagnostics are off in the fresh INI. Turn diagnostic recording on only when investigating a problem.
## Controller setup

On Windows, `PlayStationPads=1` reads supported PlayStation pads and presents Xbox input.
For native DualSense support, use `PlayStationPads=2`, disable Steam Input for the game, and restart. USB is the simplest first setup.
If Steam Input, DS4Windows or DSX supplies Xbox input, use `PlayStationPads=0`.

The Linux package defaults to `PlayStationPads=0` for an external controller route such as Steam Input. Both downloads contain the same Windows DLL; Linux uses Proton.
Add this Steam launch option, preserving any other arguments:

```text
WINEDLLOVERRIDES="version=n,b" %command%
```

If the settings window is hidden in exclusive fullscreen, try borderless/windowed mode.

## Report a problem

[Open an issue](https://github.com/GoalTrains/reskatedualstick/issues/new) with your version, controller, preset, regular/goofy stance and what happened.
Attach `session_controls.log` and `session_controls_input.csv` from the affected run. If an automatic reset fired unexpectedly, mention that too.

<details>
<summary>Build the source</summary>

Use PowerShell and LLVM MinGW tools (`clang`, `lld-link` or `ld.lld`, and `llvm-dlltool`):

```powershell
.\build.ps1 -LlvmBin 'C:\path\to\llvm-mingw\bin'
.\test.ps1 -LlvmBin 'C:\path\to\llvm-mingw\bin'
```

The output is `build/version.dll`; the script does not install it. The active entry point is `src/version_proxy.c` and the mapping lives in `src/translate.h`.
The source in this repository corresponds to the included standalone DLL.

</details>

## License

GPLv3; see [LICENSE](LICENSE). ReSkate is a separate project by its contributors.

<a href="https://ko-fi.com/X8K128ZWSN" target="_blank"><img height="36" style="border:0px;height:36px;" src="https://storage.ko-fi.com/cdn/kofi6.png?v=6" border="0" alt="Buy Me a Coffee at ko-fi.com" /></a>
