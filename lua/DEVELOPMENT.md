# Source and rebuilding

plugin/ contains the complete distributable Lua implementation. translate.lua is generated from our original v6.6 pure C state machines; runtime.lua connects them to the proposed general ReSkate APIs. config.lua and controls.lua retain INI and preset compatibility.

The generator input tools/source.c contains our own control/stance/catch/camera/entry-plan logic and an empty gesture catalog stub. No game gesture paths or native readers are included. Pure mode is enforced. To regenerate, preprocess source.c with clang -E -P to tools/source.pp.c, then run tools/port_translate.py using Python with pycparser available. The current environment already supplied pycparser; it is not a plugin runtime dependency.

To rebuild converted regression fixtures, run tools/port_tests.py <source-root> <this-folder>/test. Run tools/native_compare.py <source-root> <this-folder>/native-test <clang-path>, then tools/compare_port.mjs. The Luau test runner uses the external luau-web 1.5.1 validation runtime already staged at ../dual-stick-lua-validation/package. It is a development tool and is never shipped in the mod. tools/test_runtime.mjs exercises both the actual entry module and the runtime lifecycle.

Only manifest.json, reskate-plugin.json, README.md, CHANGELOG.md, LICENSE, icon.png and plugin/*.lua belong in the install ZIP. tools/build_package.py builds and audits that archive. Do not add the built ReSkate.dll or the old version.dll to the Lua mod ZIP.

The SDK changes are in the separate ReSkate fork branch. That branch contains only reusable native primitives and their tests. Its API reference is contrib/controller-layer-api.md in the SDK checkout. The generic API PR targets Dingo-Shenanigans:main from GoalTrains:codex/luau-controller-layers; in-game validation and maintainer review are still required before release.

## Regeneration from this repository

Set CLANG to clang and LUAU_WEB_ROOT to an existing luau-web package directory. The validator is an external development dependency and is never included in the plugin ZIP. Run tools/build_source.py, tools/port_tests.py <repository-root> <this-folder>/test, tools/native_compare.py <repository-root> <this-folder>/native-test <clang>, then tools/run_port_tests.mjs, tools/compare_port.mjs and tools/test_runtime.mjs. build_source.py reads the repository src/ headers, replaces the gesture catalog with an empty stub and includes no native memory readers. Native expectations are regenerated each run. The test converter expands C macros and escapes Lua reserved identifiers.

## Source baseline after syncing main

This PR ports the released 6.6 controls. Main now contains DLL release 6.7; its new revert/flip behavior is outside this PR. To regenerate, use a separate checkout of tag 6.6 and set DUAL_STICK_SOURCE_ROOT to it. Pass that same checkout as the source-root argument to port_tests.py and native_compare.py. build_source.py refuses a different version marker rather than silently changing the Lua baseline. The Lua runtime and existing regression fixtures retain 6.6 behavior.
