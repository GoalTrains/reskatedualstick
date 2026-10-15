Dual Stick Controls v5.133 — installation

1. Close Skate completely.
2. Open the game folder containing Skate.exe.
3. Back up version.dll and session_controls.ini.
4. Copy the supplied version.dll into that folder, replacing the old mod DLL.
5. A fresh session_controls.ini is recommended. Back up your old INI before replacing it: this resets settings and custom preset slots.
6. Launch normally, press End and choose Session-style or Skater XL-style. Fresh settings start with Session-style.

Keep the official ReSkate.dll. Do not replace it with a PR/test runtime.
End opens Dual Stick. Insert opens official ReSkate. D-pad/arrows navigate, A/Enter selects, B goes back, and LB/RB browses diagrams. Hold LB + RB + L3 + R3 for three seconds to toggle Dual Stick. Pumping stays on R3.

Disable hold-fakie is ON by default under Custom. It temporarily makes Skate's Hold Fakie Stance and Hold Fakie Push read as off while Dual Stick is active, without changing saved Skate preferences. Automatic pose resets and heavy diagnostics are OFF.

If keeping an old INI, reapply your preferred preset and set these under [SessionControls]:
DisableHoldFakie=1
NativeMenuRespawnRecovery=0
NativeGroundPoseRecovery=0
SteerCurvePercent=100

Windows controllers
PlayStationPads=1 reads supported PlayStation pads and presents Xbox input. For native DualSense use PlayStationPads=2, disable Steam Input and restart. If Steam Input, DS4Windows or DSX supplies Xbox input, use PlayStationPads=0.
