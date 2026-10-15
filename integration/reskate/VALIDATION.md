# Validation

Base: Dingo-Shenanigans/ReSkate 6df4f9a7bd4045aa74c853f088ff445011933175.

- Full dingosdk_runtime Release build succeeded with Visual Studio 2026 x64, producing ReSkate.dll in the isolated build folder.
- dual_stick_settings and gamepad_navigation passed (2/2 CTest checks).
- Patch applies cleanly to the base commit, checked against a separate base index.
- git diff --check passed.
- Standalone v5.133 passed all 58 C regression executables. Both release archives passed CRC checks and contain the same standalone DLL.
- No PR runtime was installed into C:/Games/Skate. Official ReSkate.dll SHA256 remains CA9C103F85167AC6957E15ED45B999F59AFC387D2722277114047F8E04FE6AAA.

The full build emitted upstream incremental-build warnings about absent optional assets/emotes directories in the clean worktree. They did not prevent compilation or linking.

Before merging upstream, verify the current build in-game: Insert and native Mod Options, A/B navigation, native PlayStation and XInput, companion absent, existing mods/maps/music pages, shared custom slots and menu ownership on return to skating. These have automated coverage but have not been played through in this refreshed build.

This is PR preparation, not an upstream submission or a standalone release dependency.
