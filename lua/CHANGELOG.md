# Changelog

## 0.2.0 - Lua port of Dual Stick 6.6

- Includes the released 6.6 pop-to-air handoff, release-pop and stick-steering compatibility, rear-foot rail circles and held setup through brief ground contact.
- Shares calibrated physical-foot tracking between Left/Right and XL layouts while keeping their output conventions distinct. State 701 is treated as grinding through landing/fakie handling too.
- Native target/alignment experiments are opt-in and OFF by default.
- Adds the expanded setup matrix, rail/contact regressions and runtime integration coverage. See VALIDATION.md for exact results and tick-sampling limits.
- Keeps the mod entirely Lua; generic native APIs are proposed separately in ReSkate.

## 0.1.0 — Lua testing candidate

- Ports the v6.5 control state machine and settings into Luau, including the latest grind-exit, held-manual frame and fakie-input protection changes.
- Adds physical controller selection, live enable/disable, automatic settings initialization and native preference leases.
- Moves the companion menu to ReSkate's Dual Stick plugin page.
- Requires the separately reviewed general ReSkate controller-layer APIs. No companion DLL is packaged.
- 37 regression suites and Lua lifecycle integration pass; 71,151 controller samples agree with the original within two stick units. Native API tests pass. In-game testing remains required.
- Optional native pose reset, asset-range experiments, updater and companion diagnostics are excluded; see README.
