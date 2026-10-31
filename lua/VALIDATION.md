# Validation - Lua 6.6 testing candidate

The plugin ports released Dual Stick 6.6 (source commit 2c2254a7ee09f1d09f54f0009bf78b31490a09e9).
The generic SDK dependency is [GoalTrains/ReSkate codex/luau-controller-layers](https://github.com/GoalTrains/ReSkate/tree/codex/luau-controller-layers), commit 9409dec2b3e677098f5e24bf5bf480522be3fba7, merged with upstream main 9506a39.

- All 44 converted Luau regression suites pass. They include the 36,864-combination pop/setup matrix, rear rail circles, state-701 continuity and brief-ground-contact setup preservation.
- Comparing 852,927 translated samples with freshly rebuilt C 6.6 references finds zero mismatches at the checked tolerances. Buttons, triggers and checked state flags match exactly; maximum raw stick difference is two units (comparison tolerance: three).
- Runtime lifecycle/menu/console/settings tests pass. Added integration cases verify calibrated native physical-foot evidence for both Left/Right and XL, exclusion for Front/Back, grind state 701, and the default-off native target experiment.
- ReSkate x64 Release runtime and plugin-test targets build with MSVC /W4 /WX after merging current upstream. The plugin CTest suite passes. Fifteen optional maintainer plugin fixtures are absent/skipped; this checkout also reports missing maintainer assets/emotes build dependencies. No compiler errors or compiler warnings remain.
- The generated plugin ZIP is audited for entry/module files, JSON manifests and 256x256 PNG artwork. It contains no companion/runtime DLL, executable, game binary or extracted game data.

These are automated source and API checks. The Lua port has not been gameplay-tested. It samples on plugin ticks, whereas the DLL translates controller polls. Very short gestures can be missed, and a slow tick can expire the host's 100 ms output lease. Hardware, menu/replay timing, native animation availability, grind contact outcomes, and Proton still need testing. The favorable DLL gameplay feedback is not claimed as Lua gameplay verification.

Regeneration inputs and commands are documented in DEVELOPMENT.md. Native expectations are rebuilt for each comparison. The pure source snapshot uses an empty gesture catalog and contains no native memory readers. ReSkate supplies game gestures; this mod is pure control translation.

Revalidated 2026-10-31 after resolving current-main conflicts: all 44 Luau suites, runtime integration and 852,927 C/Luau samples still pass (maximum raw stick difference: 2). The updated SDK runtime/plugin build and native plugin tests pass, including approval-summary and padded Bluetooth/menu-preview regressions. Independent upstream PlayStation report/CRC checks pass. Regeneration from the pinned 6.6 source exactly reproduces the tested translator. The repository's newer DLL 6.7 release is retained; this Lua PR remains scoped to 6.6.
