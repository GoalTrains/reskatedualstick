Dual Stick Controls v5.134 — installation

1. Close Skate completely.
2. Open the game folder containing Skate.exe.
3. Back up version.dll and session_controls.ini.
4. Copy the supplied version.dll into that folder, replacing the old mod DLL.
5. Updating from v5.133: keep your INI. For older installs a fresh session_controls.ini is recommended; back up first, because replacing it resets settings and custom preset slots.
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
