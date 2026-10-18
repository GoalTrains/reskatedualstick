$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$folder = Join-Path $project ('build/interactive updater test ' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory((Join-Path $folder 'updater')) | Out-Null
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead((Join-Path $project 'releases/v5.157/Dual_Stick_Controls_v5.157.zip'))
try { [IO.Compression.ZipFileExtensions]::ExtractToFile($archive.GetEntry('Dual_Stick_Controls/version.dll'), (Join-Path $folder 'version.dll')) }
finally { $archive.Dispose() }
[IO.File]::WriteAllText((Join-Path $folder 'session_controls.ini'), "[SessionControls]`r`nLocalTest=preserve-me`r`n")
Copy-Item -LiteralPath (Join-Path $project 'build/DualStickLauncher.exe') -Destination $folder
foreach ($name in @('Update-DualStick.ps1','DualStickUpdater.psm1','Update-Dialog.ps1')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $folder 'updater')
}
$dialogPath = Join-Path $folder 'updater/Update-Dialog.ps1'
$text = [IO.File]::ReadAllText($dialogPath).Replace('Dual Stick Controls - update available', 'LOCAL TEST - Dual Stick update')
$text = $text.Replace('Read the changes below. Updating keeps your settings.', 'LOCAL TEST: only disposable files change. Your real game is untouched.')
[IO.File]::WriteAllText($dialogPath, $text)
$source = Join-Path $folder 'TestLauncher.cs'
[IO.File]::WriteAllText($source, @'
using System;
using System.IO;
using System.Windows.Forms;
class TestLauncher {
    [STAThread] static void Main() {
        var root = AppDomain.CurrentDomain.BaseDirectory;
        try {
            bool preserved = File.ReadAllText(Path.Combine(root,"session_controls.ini")).Contains("preserve-me");
            if (!preserved) throw new Exception("Test settings were not preserved.");
            File.AppendAllText(Path.Combine(root,"test-result.txt"),"SUCCESS: test launcher opened after updater; test settings preserved.\n");
            MessageBox.Show("Success - the local test finished.\n\nThe updater completed before this test launcher opened. Your test settings were preserved.\n\nYour real game files were not changed, and the game was not started.", "Local test successful", MessageBoxButtons.OK, MessageBoxIcon.Information);
        } catch (Exception error) {
            File.AppendAllText(Path.Combine(root,"test-result.txt"),"ERROR: " + error.ToString() + "\n");
            MessageBox.Show(error.Message,"Local test failed",MessageBoxButtons.OK,MessageBoxIcon.Error);
        }
    }
}
'@)
$compiler = Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
& $compiler /nologo /target:winexe /reference:System.Windows.Forms.dll "/out:$(Join-Path $folder 'ReSkateLauncher.exe')" $source
if ($LASTEXITCODE -ne 0) { throw 'Test launcher compilation failed' }
Start-Process -FilePath (Join-Path $folder 'DualStickLauncher.exe') -WorkingDirectory $folder -WindowStyle Hidden
Write-Output ('Interactive test opened in: ' + $folder)
