# v5.72 menu integration test

The local test pair was built and installed on 11 October 2026, preserving the
existing settings and backing up both previous DLLs. In-game feedback is pending.

ReSkate base revision: `63c9e39c2aca0c250c602ac53b43b497da1e368d`.
The source patch contains the companion settings page, controller navigation
feed, and input ownership export. It has not been submitted upstream.

## Build validation

- Visual Studio Build Tools 2026, MSVC 19.51, C++20, x64 Release.
- Full `dingosdk_runtime` DLL build passed with the repository's `/W4 /WX` settings.
- `dual_stick_settings` and `gamepad_navigation` passed through CTest.
- Gamepad tests include actual D-pad/A navigation and widget activation, analog
  input, deadzones, and key release when ownership ends.
- All eight standalone companion regression executables passed, including the
  new overlay handoff guard and the existing feet, powerslide, stall, ramp and
  R3 pumping checks.
- `ReSkateOverlayOwnsInputV1` is present in the test DLL's PE export table.
- Installed Skate.exe matches the supported game hash. Installed test DLL hashes
  match their build outputs; the active INI is unchanged.

The first runtime link used a stale input object; the final incremental rebuild
compiled the corrected function and linked successfully. MSBuild also reported
missing `assets` / `assets/emotes` directory dependencies from the base checkout;
these were non-fatal build-system warnings.

## Test build hashes

```text
ReSkate.dll  EE28B7BD4C46147230A5861EDEA10CE746571A9C668CA156E9C769D88C45F9CB
version.dll  F17C9978D2AFD496CE762E88D393BCED40EBC050538B93B24255F0D41E2F2B23
```

## In-game checks still needed

1. Launch, open the Insert overlay, and navigate/select using D-pad and A.
   Check both an Xbox-style pad and a native PlayStation pad where available.
2. Verify the mouse/keyboard still work, and controller input does not skate the
   character behind the menu. Close/reopen several times.
3. Close the overlay while holding a stick; confirm no fresh pop occurs, then
   release it and confirm foot controls work again.
4. In the game's pause menu, open Mod Options > Dual Stick. Apply a preset,
   edit a setting, save/load a custom slot, and verify the mod refreshes it.
5. Check normal street controls, grinds, R3 pumping and optional ramp controls.

Both DLLs are required for the new menu ownership behavior. The standalone
companion preserves its previous fallback when used with an older ReSkate DLL.

Insert menu correction, 11 October 2026:
- Dual Stick now appears as a dedicated Insert sidebar page, with both presets,
  five shared custom slots and editable gameplay settings.
- Insert sidebar, category tabs and option tiles now enable controller focus.
  Focus outlines draw above tile backgrounds; B/Circle closes when no popup is open.
- Full MSVC Release build and both CTest regressions passed. The controller test
  now navigates and activates custom InvisibleButton tiles, matching the menu.
- Installed ReSkate.dll SHA256: 9865D5B2184CCF76051E1FD60E8267D4B46C0692E263C8B2D0D026F21B0FD665.
- Official launcher updates are disabled during testing, since it otherwise
  replaces the patched runtime. Original launcher settings are backed up locally.
- Controls INI and companion DLL are unchanged. In-game confirmation remains pending.
