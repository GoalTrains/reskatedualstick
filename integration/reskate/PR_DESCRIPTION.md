Title: Add optional Dual Stick settings to the Insert and native Mod Options menus

Dual Stick players currently need a separate settings window. This adds a companion settings page directly below Trainer in Insert and a Dual Stick tab in native Mod Options, using the same session_controls.ini already read by the standalone mod.

- Provides current Session-style and Skater XL-style presets; Front/Back, Left/Right Foot and XL layouts; pop/steering/switch directions; reverts and wind-up; air/grind options; ramp mode and hold-fakie compatibility.
- Shares five custom preset slots, confirms overwrites and preserves installation-specific controller/menu/stance preferences when loading.
- Leaves the page informational when the companion INI is absent. No translator DLL is bundled and no configuration is created implicitly.
- Adds controller focus/navigation through the existing multi-device reader. A enters the right panel; B backs out through popups and nested panels to the sidebar. Insert closes the menu. Existing upstream pages remain present.
- Adds a versioned input-ownership export so compatible companions can suspend generated inputs while ReSkate owns them.

Validation is recorded in VALIDATION.md. Current-upstream in-game verification remains required for both Insert and native Mod Options, including native PlayStation and XInput controllers, with and without the companion installed.
