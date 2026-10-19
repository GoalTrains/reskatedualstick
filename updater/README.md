# Update before opening ReSkate

Windows: open **DualStickLauncher.exe** instead of ReSkateLauncher.exe. It checks the latest stable release of GoalTrains/reskatedualstick, shows the installed/new versions and scrollable patch notes, and asks before updating. It then opens the unchanged official ReSkate launcher. The update attempt finishes before you can press Play. It also forwards launcher arguments, including `--no-gui`.

Opening the official launcher directly bypasses this optional updater. Direct integration into the official launcher would require an upstream launcher change. No ReSkate update settings, client DLL or launcher executable are replaced.

## Install

Copy `DualStickLauncher.exe`, `dual_stick_updater.json`, and the `updater` folder beside `ReSkateLauncher.exe`. Create a shortcut to DualStickLauncher.exe and name it **ReSkate + Dual Stick**. Keep your session_controls.ini. If updating the helper, keep your existing dual_stick_updater.json preferences.

The updater uses Windows PowerShell and .NET Framework supplied with Windows. No Python, GitHub login or VS installation is needed. This first updater is for Windows; the mod's Linux/Proton installation continues to work as before.

Only stable published releases are installed. The release manifest is read from that release's tag, both zip and DLL SHA256 values are checked, and only the named DLL/initial INI entries are read from the archive. These hashes detect incomplete or mismatched downloads; the GitHub repository/release account remains the trusted distribution source.

Existing INI files are preserved byte for byte. A fresh INI is created only when absent. The previous DLL is saved as `version.dll.dualstick-previous`. Newer local builds and unrelated version.dll files are left alone. Updates are skipped if Skate is already running; a file lock or failed replacement leaves the existing DLL intact.

The v6.2 updater recognizes the author's private v6.13-v6.21 catch, camera and menu DLLs as previews, allowing them to update to the stable release. Other newer local builds are still protected from downgrade.

Offline, GitHub rate-limit and download failures do not block opening the official launcher. Network requests use short timeouts. The choice and confirmation windows stay open until you respond. Details are in `dual_stick_updater.log`. Do not run the wrapper as administrator unless your game folder already requires that access.

## Update choices

- **Update now:** download and verify the update, then show a success notice before opening ReSkate.
- **Skip this release:** keep your current build; this release will not prompt again, but a newer one will.
- **Disable update checks:** keep your current build and stop checking. Set `"enabled": true` in dual_stick_updater.json to re-enable.
- **Close / Escape:** skip this launch only, without saving a preference.

No popup appears when already current. A failed check/update explains that the installed version will be used.

## Disable or remove

Set `"enabled": false` in `dual_stick_updater.json`, or use ReSkateLauncher.exe directly. You can remove DualStickLauncher.exe and the updater folder without changing the installed mod. To roll back the DLL, close Skate, back up version.dll, then copy version.dll.dualstick-previous to version.dll.

## Development

Run `updater/build.ps1` to build the GUI launcher. Run `updater/Test-Updater.ps1` under Windows PowerShell to check archive validation, backups, settings preservation, locks, offline operation and downgrade protection. Run `updater/Update-DualStick.ps1 -GameDirectory <folder> -CheckOnly` to query GitHub without installing an update.

## Local interactive test

Run Test-Interactive.ps1 to open a clearly labelled local-test prompt using an old DLL in a disposable build folder. It reads real GitHub patch notes and can update only that test copy. A stub launcher confirms completion; the actual game is not started. Every run creates a new test folder, so all choices can be tried. No GitHub publication is performed.
