# Install Dual Stick Controls v5.128

1. Close Skate completely.
2. Find the game folder containing Skate.exe.
3. Back up version.dll and session_controls.ini.
4. Copy the downloaded version.dll into that folder, replacing the old mod DLL.
5. A fresh session_controls.ini is recommended: replace it with the included INI after backing up. This resets settings and custom preset slots.
6. Launch through your normal ReSkate setup, press End and choose Session-style or Skater XL-style. The fresh INI starts with Session-style.

Keep the official ReSkate.dll.

To keep your settings instead, retain your INI and reapply the XL preset for XL controls. To enable the experimental on-board recovery, set these under [SessionControls]:

```ini
NativeMenuRespawnRecovery=1
NativeGroundPoseRecovery=1
```

Set both to 0 to disable the visible reset. Recovery is enabled in the included INI and may stop speed and flicker. The XL foot-job agreement check is separate and does not respawn.

End opens the mod settings; Insert opens ReSkate's menu. D-pad/arrows navigate, A/Enter selects, B goes back, and LB/RB browses diagrams.
Hold LB + RB + L3 + R3 for three seconds to toggle the mod.

The latest goofy changes need gameplay feedback. Report the version, stance, preset and steps, with session_controls.log and session_controls_input.csv. Large pose diagnostics are disabled in the supplied INI.

## Windows controllers

The Windows INI defaults to PlayStationPads=1 (PlayStation pad read by the mod, presented as Xbox input).
For native DualSense, use PlayStationPads=2, disable Steam Input and restart. If using Steam Input/DS4Windows/DSX as an Xbox-input mapper, use PlayStationPads=0.
Try borderless/windowed mode if the settings window is hidden in exclusive fullscreen.
