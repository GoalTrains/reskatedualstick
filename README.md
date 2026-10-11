# Dual Stick Controls for reSkate

A two-stick controller mod for [reSkate](https://github.com/Dingo-Shenanigans/ReSkate).
It translates foot inputs into Skate's existing controls, with Session-style and
XL-style presets, editable settings, custom presets, and optional stock ramp controls.

Current version: **v5.84 TEST**. Ramp detection remains experimental.

## Install on Windows

1. Close the game and back up any existing `version.dll`.
2. Place the mod's built `version.dll` beside `Skate.exe`.
3. For a fresh install, copy `config/session_controls.example.ini` beside
   `Skate.exe` and rename it to `session_controls.ini`.
4. Launch through `ReSkateLauncher.exe`.

When updating, keep your existing INI to retain your settings and custom presets.
This source repository does not include compiled game files. Build the DLL below
or use a separately supplied release archive.

## Linux / Steam Deck with Proton

Use the same Windows DLL beside `Skate.exe`. Add this Steam launch option:

```text
WINEDLLOVERRIDES="version=n,b" %command%
```

This is a Windows DLL loaded through Proton, not a native Linux library.
Linux/Proton gameplay has not been independently tested with this build.

## Menu

The menu opens on startup once controller input begins. Press **End** to reopen
it by default. Navigate with the D-pad or arrow keys, select with A or Enter,
and go back with B. The main menu lets you change its hotkey, size, and startup
display. Controls stay visible on the right in the main menu, Advanced settings,
and Custom presets. Use **LB / RB** or **Page Up / Page Down** to browse four
pages of diagrams. D-pad/arrows and A continue to operate settings on the left.
The menu automatically shrinks to fit the game window.

The companion menu always uses a game-owned window, with no competing graphics
hooks even before ReSkate loads. `MenuInGame=1` in an older INI cannot re-enable
the hook. The menu is not desktop-topmost, hides while another app is in front,
and ignores menu hotkeys and navigation outside the game. It handles replacement
of the game's startup window. Exclusive fullscreen may still hide a separate
window; use borderless/windowed mode if necessary.

The companion yields controller input while the official ReSkate Insert menu
is visible. This standalone update does not add our proposed Dual Stick page
to stock ReSkate; that remains a separate upstream integration.

Five custom preset slots let you save and load your own settings. Saving over an
occupied slot requires a second press. Holding L3 and R3 together for three
seconds toggles Dual Stick controls.

## Presets

| Setting | Session-style | XL-style |
| --- | --- | --- |
| Foot layout | Front / back foot | Left / right foot |
| Switch behavior | Consistent front / back jobs | True Foot |
| Pop style | Flick | Release |
| Stick steering | Off | On |
| One-stick flips | Blocked | Allowed |

**These presets still map Skate controls across two sticks. Their inputs differ
from Session and Skater XL.** All listed settings remain editable.

Front/Back Foot always assigns the left stick to the front end and the right to
the back end, including goofy and switch. Left/Right Foot follows physical feet;
True Foot changes their front/back jobs when riding switch.

## Basic controls

- Ollie: back foot down, then front foot up. Nollie: front up, then back down.
  Release pop also allows holding and releasing the pop foot.
- Outside stock ramp mode, triggers steer and pre-wind; bumpers handle grabs.
- Move both sticks inward or outward to powerslide. Adjust their vertical angle
  while holding the slide to change the coping approach.
- On rails and stalls, sticks adjust the board ends. Pull front up or back down
  to prepare a trick out, then flick the other foot.
- Centred L3/R3 bump left/right. Tilt a stick up/down and click it for a lip exit.
- With sticks centred while riding, hold R3 for pumping crouch and release with
  your usual pumping timing.

## Stock ramp controls

Under Advanced, choose **Off**, **Air only**, or **Early on ramp**. Fresh installs
default to Off.

- **Air only:** a qualifying ramp approach and an airborne trigger grab activate
  stock handling until landing.
- **Early on ramp:** a qualifying grounded climb activates stock handling before
  takeoff for stock pop/pre-wind inputs. Landing, or an expired approach, restores
  foot controls.

In stock mode, the physical left stick controls steering/spins/body direction,
the right controls the board, and triggers handle airborne grabs. R3 keeps its
ground pump. A vertical gesture held at handoff must centre or change direction
before becoming a stock vertical gesture; horizontal rotation is immediate.

The detector uses skater orientation and rapid grounded tilt changes, not a
confirmed collision-surface normal. Other qualifying surfaces may activate it.

## Build the standalone mod

The standalone mod uses PowerShell and LLVM tools: `clang`, `lld-link` (or
`ld.lld`), and `llvm-dlltool`. The existing build uses LLVM MinGW on Windows.

```powershell
.\build.ps1 -LlvmBin 'C:\path\to\llvm-mingw\bin'
```

The output is `build/version.dll`. The build does not install it into the game.
The active entry point is `src/version_proxy.c`; `src/translate.h` implements
the input mapping. Other source files include controller support, diagnostics,
menus, and historical helpers.

Run the included control regression tests with:

```powershell
.\test.ps1 -LlvmBin 'C:\path\to\llvm-mingw\bin'
```

## Proposed reSkate menu integration

`integration/reskate/dual-stick-menu.patch` adds a companion settings page under
reSkate's Mod Options. It is a separate, unsubmitted source patch. It requires
the standalone mod; it does not bundle the translator. The test integration also
feeds raw controller navigation to the Insert overlay and exposes an input
ownership signal. Companion v5.72 respects that signal, stops its generated
gestures while reSkate owns input, and protects held feet on return. With an older
ReSkate DLL that lacks the signal, the companion keeps its previous behavior.
See its README and draft PR description for the base revision and validation.

Building the complete modified reSkate requires its supported Visual Studio C++
toolchain. The standalone LLVM build above does not build `ReSkate.dll`.

## Logs and troubleshooting

`session_controls.log` records mod events. Precise input logging can also write
`session_controls_input.csv`. Optional RAMP/FLAT diagnostic recording writes
`session_controls_ramp.bin`; labels are manual recording labels. Changing the
label resets the bounded recording allowance. Keep `SwitchSource=4` unless
diagnosing another source.

Share logs when reporting problems, with your version, controller, preset,
regular/goofy stance, and what happened. Do not commit personal logs or settings.

To uninstall, close the game and remove the mod's `version.dll` (or restore your
backup). Keep the INI if you plan to reinstall. Remove an added Proton override.

## License and development

GPLv3; see [LICENSE](LICENSE). ReSkate is a separate project by its contributors.
This mod's development and menu integration have used AI assistance, with local
regression checks and user gameplay testing. See [CHANGELOG.md](CHANGELOG.md)
for version history and validation notes.

## Remount correction

The experimental regular-stance correction separates riding direction from
Skate's native trick-input frame. It is enabled even when an older INI omits
the new key. v5.84 only arms calibration after an actual dismount following
riding; a fresh spawn on foot keeps the native reference.

The correction needs a short straight ground roll before the first trick after
remounting; stationary, sideways, airborne and teleport samples cannot establish
the reference. Calibration ends at takeoff or after two seconds, retaining the
native reference if no valid direction was established. The reference stays
fixed while riding. Earlier remount behavior was confirmed in game; the latest
v5.84 validation changes still require gameplay confirmation.
Goofy retains its existing reader; this correction is not enabled for goofy.
Set `RemountStanceMotion=0` to return to the previous reader. Diagnostic
snapshots are off by default (`RemountDiagnostics=0`).
