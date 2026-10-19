Dual Stick Controls v6.2 — installation

1. Close Skate completely.
2. Open the game folder containing Skate.exe.
3. Back up version.dll and session_controls.ini.
4. Copy the supplied version.dll into that folder, replacing the old mod DLL.
5. Updating from v5.133 or newer: keep your INI. For older installs a fresh session_controls.ini is recommended; back up first, because replacing it resets settings and custom preset slots.
6. Launch normally, press End and choose Session-style or Skater XL-style. Fresh settings start with Session-style.

Keep the official ReSkate.dll. Do not replace it with a PR/test runtime.
End opens Dual Stick. Insert opens official ReSkate. D-pad/arrows navigate, A/Enter selects, B goes back, and LB/RB browses diagrams. Hold LB + RB + L3 + R3 for three seconds to toggle Dual Stick. Pumping stays on R3.

Disable hold-fakie is ON by default under Custom. It temporarily makes Skate's Hold Fakie Stance and Hold Fakie Push read as off while Dual Stick is active, without changing saved Skate preferences. Automatic pose resets and heavy diagnostics are OFF.

If keeping an old INI, reapply your preferred preset and set these under [SessionControls]:
DisableHoldFakie=1
NativeMenuRespawnRecovery=0
NativeGroundPoseRecovery=0
SteerCurvePercent=100

Linux / Steam Deck (Proton)
Both packages contain the Windows DLL. Add this launch option, preserving existing arguments:
WINEDLLOVERRIDES="version=n,b" %command%
The Linux INI uses PlayStationPads=0 for an external controller route such as Steam Input. Bind End to open settings if needed.

FOOT SETUP grind controls are enabled in fresh settings. If keeping your INI, enable Custom > Grind setup > FOOT SETUP (SplitGrindControls=1). For dedicated two-foot setup, block single-stick flips.

The optional pre-launch updater download is Windows-only. Linux/Steam Deck uses the manual mod zip.

Manual catch is OFF by default. To test this early attempt, open End > Custom > Manual catch (early attempt). It is incomplete and many tricks may not hold reliably; turn it back OFF if it interferes with tricks.

D-pad camera sides is ON by default: tap left/right to select a side, hold 0.5 seconds for the original action. The mod applies the required camera settings and keeps L3 available to its other controls. Turn it off under End > Custom if desired. An existing DpadCameraSide=0 is respected.
