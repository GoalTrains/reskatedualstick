Add companion Dual Stick settings and controller navigation

Players currently need a separate hotkey to find the companion mod's settings.
This adds a controller-friendly Mod Options > Dual Stick page using the existing
session_controls.ini beside Skate.exe. The companion v5.71+ DLL remains required.
ReSkate does not bundle or install the translator.

The Insert overlay enables keyboard navigation but disables ImGui's slot-zero
gamepad backend without providing a gamepad feed. Feed ImGui from ReSkate's
existing raw XInput/native PlayStation reader while the overlay is visible and
foreground. Release navigation state when it closes. This adds no input hooks.
A versioned ReSkateOverlayOwnsInputV1 C export also lets companion v5.72 suspend
its generated gestures during UI/freecam/prompt ownership and guard held feet
when gameplay returns, without relying on pause-state guesses.

Session-style and XL-style presets appear first, followed by five shared custom
slots and editable gameplay options. Session blocks one-stick flips; XL allows
them. Preset help explains that both map Skate controls across two sticks.
Occupied slots require confirmation before overwrite. Missing configurations
are explained without creating files; unrelated tuning and menu preferences
survive edits and custom loads. The translator's enabled state can only be set
for the next launch from this page; its existing menu handles immediate toggles.

Validation completed locally:
- Settings and gamepad regression executables built with Visual Studio 2026
  MSVC x64 Release and passed. Tests retain assertions in Release.
- Gamepad checks cover actual D-pad/A widget navigation and activation, analog
  input, deadzones and release on loss of ownership.
- Eight companion control regression executables passed, including ownership
  suspension, held-foot return guards and existing foot/ramp/pump controls.
- Full ReSkate.dll MSVC x64 Release build passed with Visual Studio Build Tools
  2026 (19.51). The versioned ownership export was verified in the built PE.
- Complete edited hub_page.cpp passed LLVM MinGW C++20 syntax checking.
- Git diff whitespace check passed.

Still required before release:
- In-game controller navigation, page layout and live INI refresh checks.
- Confirm that an external companion settings page is the integration desired.

Implementation and this description were prepared with AI assistance. The
companion control translator itself is unchanged by this patch.
