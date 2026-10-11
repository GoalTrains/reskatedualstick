Add an optional Dual Stick Controls page to Mod Options

Players currently need a separate hotkey to find the companion mod's settings.
This adds a controller-friendly Mod Options > Dual Stick page using the existing
session_controls.ini beside Skate.exe. The companion v5.71+ DLL remains required.
ReSkate does not bundle or install the translator.

Session-style and XL-style presets appear first, followed by five shared custom
slots and editable gameplay options. Session blocks one-stick flips; XL allows
them. Preset help explains that both map Skate controls across two sticks.
Occupied slots require confirmation before overwrite. Missing configurations
are explained without creating files; unrelated tuning and menu preferences
survive edits and custom loads. The translator's enabled state can only be set
for the next launch from this page; its existing menu handles immediate toggles.

Validation completed locally:
- C++20 settings regression executable compiled with warnings as errors and ran
  successfully, including with NDEBUG defined (assertions retained by test).
- Complete edited hub_page.cpp passed LLVM MinGW C++20 syntax checking using
  repository headers and vendored dependencies.
- Git diff whitespace check passed.

Still required before release:
- Full supported MSVC x64 build (Visual Studio compiler unavailable locally).
- In-game controller navigation, page layout and live INI refresh checks.
- Confirm that an external companion settings page is the integration desired.

Implementation and this description were prepared with AI assistance. The
companion control translator itself is unchanged by this patch.
