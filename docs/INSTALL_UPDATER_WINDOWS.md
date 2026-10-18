Dual Stick v6 - optional Windows updater

Install once:
1. Close Skate. Open the folder containing Skate.exe and ReSkateLauncher.exe.
2. Install the normal Windows mod zip if you have not installed Dual Stick yet.
3. Copy DualStickLauncher.exe, dual_stick_updater.json and the updater folder from this download into that game folder. Keep an existing dual_stick_updater.json to preserve updater preferences.
4. Open DualStickLauncher.exe. You can create a shortcut to it named ReSkate + Dual Stick.
5. Use that shortcut for future launches. It finishes its update attempt before opening the official ReSkate launcher. Opening ReSkateLauncher.exe directly bypasses mod update checks.

When a stable update is available, the window shows your installed version, the new version and scrollable GitHub patch notes.
- Update now: install the verified DLL, then show confirmation before opening ReSkate.
- Skip this release: keep your version; do not ask again until a newer release is available.
- Disable update checks: stop checking until you set enabled to true in dual_stick_updater.json.
- Close or Escape: skip updating for this launch only.

Existing session_controls.ini settings and custom presets are preserved. The previous mod DLL is retained as version.dll.dualstick-previous. Close Skate before restoring that backup.

If checking/downloading fails, ReSkate still opens with the installed mod. Details are in dual_stick_updater.log. There is no popup when already current. Running games and newer local mod builds are left alone.

The official ReSkate.dll, ReSkateLauncher.exe and ReSkate update settings are not replaced or modified. This does not depend on the ReSkate menu PR.

This optional helper requires Windows PowerShell and .NET Framework, normally included with Windows. No Python, Visual Studio or GitHub login is required. Linux/Steam Deck should use the normal Linux mod zip instead.

Keep this helper installed between mod releases. Download it again only if the updater itself changes. To stop using it, launch ReSkateLauncher.exe directly; removing the helper does not remove Dual Stick.
