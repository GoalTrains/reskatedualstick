[Generic ReSkate API PR #373](https://github.com/Dingo-Shenanigans/ReSkate/pull/373)

Consumer: [Dual Stick 6.6 Lua PR #7](https://github.com/GoalTrains/reskatedualstick/pull/7).

Luau plugins can read controllers but cannot publish translated controls to the game, so input mods still need a separate native companion. This adds general controller layers and local rider requests; control schemes, gesture recognition, foot roles and timing stay in the plugin.

- Separate `input.write` permission for complete, normalized controller replacements, physical per-controller reads, ownership/expiry, stop cleanup and native menu/focus invalidation. XInput and native PlayStation paths retain existing input-hiding filters; HID writes preserve unrelated report data and Bluetooth CRCs.
- Read-only, coherent local rider frame and occupied typed animation-value observations. Permission-checked numeric physics-output requests validate the local owner and property identity twice; scripts receive no native addresses.
- Short-lived native rail-target selection for any class 0–6, with original request/cache restoration and native candidate acceptance retained. Uses build-scoped offsets, a checked entry prefix and HookBatch/Detours.
- Temporary observed boolean gameplay preference holds, with original values in Settings, ownership/limits/expiry and no persisted changes.

These primitives are used by a separate Luau port of Dual Stick v6.6. No Dual Stick rules, plugin files, DLLs, game assets or extracted gesture data are included in this SDK patch. The API reference and example are in `contrib/controller-layer-api.md`.

Validation:
- MSVC x64 Release runtime and plugin tests build with /W4 /WX.
- Native plugin suite passes, including permission denial, argument validation, leases, original preference/UI behavior, typed occupied animation pages and PlayStation report preservation. Fifteen optional maintainer-plugin fixtures are absent and skipped.
- This fresh checkout lacks maintainer assets/emotes; CMake emits missing dependency warnings. No compiler errors or compiler warnings remain.
- The separate Luau port passes 44 regression suites and lifecycle/menu/storage/console integration tests. Comparing 852,927 controller samples with the original C produces exact buttons/triggers/checked flags and at most two raw stick units of difference.
- Diff/whitespace checks pass.

Kept as a draft for API review and in-game testing. XInput and native PlayStation hardware, graph availability, rail acceptance, replay/menu handoff and slow frames remain unverified. Lua samples on plugin ticks rather than every controller poll, so these comparisons do not establish equal latency or capture gestures between ticks.

Prepared with an AI coding assistant.