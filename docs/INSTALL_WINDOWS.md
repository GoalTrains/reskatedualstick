Dual Stick Controls v6.5 — installation

1. Close Skate completely.
2. Open the game folder containing Skate.exe.
3. Back up version.dll and session_controls.ini.
4. Copy the supplied version.dll into that folder, replacing the old mod DLL.
5. Updating from v5.133 or newer: keep your INI. For older installs a fresh session_controls.ini is recommended; back up first, because replacing it resets settings and custom preset slots.
6. Launch normally, press End and choose Session-style or Skater XL-style. Fresh settings start with Session-style.

Manual Catch is OFF by default; enable it under End > Custom > Manual catch. Click L3, R3 or both in the air to catch. Some tricks settle at an angle rather than continuing to rotate; catch timing still matters for a clean result. Missed or mistimed catches can produce an awkward automatic catch. Independent foot catches, dark catches and forced missed-catch bails are not implemented.

Skate handles trick rotation speed; the v6.3 stick-speed override has been removed. Any old GestureSpeed INI key is ignored.

Keep the official ReSkate.dll. Do not replace it with a PR/test runtime.
End opens Dual Stick. Insert opens official ReSkate. D-pad/arrows navigate, A/Enter selects, B goes back, and LB/RB browses diagrams. Hold LB + RB + L3 + R3 for three seconds to toggle Dual Stick. Pumping stays on R3.

Disable hold-fakie is ON by default under Custom. It normally makes Skate's Hold Fakie Stance and Hold Fakie Push read as off without changing saved preferences. Fresh held manual/setup input temporarily enables effective Hold Fakie Stance to defer normalization; release restores the compatibility override. Automatic pose resets and heavy diagnostics are OFF.

If keeping an old INI, reapply your preferred preset and set these under [SessionControls]:
DisableHoldFakie=1
NativeMenuRespawnRecovery=0
NativeGroundPoseRecovery=0
SteerCurvePercent=100

Windows controllers
PlayStationPads=1 reads supported PlayStation pads and presents Xbox input. For native DualSense use PlayStationPads=2, disable Steam Input and restart. If Steam Input, DS4Windows or DSX supplies Xbox input, use PlayStationPads=0.

FOOT SETUP grind controls are enabled in fresh settings. If keeping your INI, enable Custom > Grind setup > FOOT SETUP (SplitGrindControls=1). For dedicated two-foot setup, block single-stick flips.

Optional Windows updater (separate zip)
Copy DualStickLauncher.exe, dual_stick_updater.json and the updater folder beside ReSkateLauncher.exe. Keep an existing updater JSON to retain your preferences. Open DualStickLauncher.exe (or create a ReSkate + Dual Stick shortcut) for future launches. It shows patch notes and asks before updating. Skip this release keeps your build until a newer release; Disable update checks stops checks; closing the window skips this launch. The official ReSkate files and updater settings are untouched.

D-pad camera sides is ON by default: tap left/right to select a side, hold 0.5 seconds for the original action. The mod applies the required camera settings and keeps L3 available to its other controls. Turn it off under End > Custom if desired. An existing DpadCameraSide=0 is respected.
