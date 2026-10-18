$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'DualStickUpdater.psm1') -Force
Add-Type -AssemblyName System.IO.Compression.FileSystem
$project = Split-Path $PSScriptRoot -Parent
$manifest = [IO.File]::ReadAllText((Join-Path $project 'releases/v6.0/manifest.json')) | ConvertFrom-Json
$name = 'Dual_Stick_Controls_v6.0.zip'
$asset = [pscustomobject]@{ name = $name; size = $manifest.packages.$name.size; digest = 'sha256:' + $manifest.packages.$name.sha256; browser_download_url = ('https://github.com/GoalTrains/reskatedualstick/releases/download/6.0/' + $name) }
$release = [pscustomobject]@{ tag_name = '6.0'; draft = $false; prerelease = $false; body = [IO.File]::ReadAllText((Join-Path $project 'docs/RELEASE_6.0.txt')); assets = @($asset, [pscustomobject]@{ name = 'Dual_Stick_Auto_Updater_Windows.zip' }) }
$module = Get-Module DualStickUpdater
& $module {
    param($r, $m, $p)
    $script:FixtureRelease = $r; $script:FixtureManifest = $m; $script:FixturePackage = $p
    function script:Test-DualStickGameRunning { return $false }
    function script:Get-DualStickRemoteJson {
        param($Url)
        if ($Url -eq 'https://api.github.com/repos/GoalTrains/reskatedualstick/releases/latest') { return $script:FixtureRelease }
        if ($Url -eq 'https://raw.githubusercontent.com/GoalTrains/reskatedualstick/6.0/releases/v6.0/manifest.json') { return $script:FixtureManifest }
        throw "Unexpected metadata URL: $Url"
    }
    function script:Invoke-WebRequest { param($Uri, $OutFile, $TimeoutSec, $Headers, [switch]$UseBasicParsing) [IO.File]::Copy($script:FixturePackage, $OutFile) }
} $release $manifest (Join-Path $project ('releases/v6.0/' + $name))
$root = Join-Path $project ('build/v6-update-test-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($root) | Out-Null
$oldZip = [IO.Compression.ZipFile]::OpenRead((Join-Path $project 'releases/v5.158/Dual_Stick_Controls_v5.158.zip'))
try { [IO.Compression.ZipFileExtensions]::ExtractToFile($oldZip.GetEntry('Dual_Stick_Controls/version.dll'), (Join-Path $root 'version.dll')) }
finally { $oldZip.Dispose() }
$oldHash = Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $root 'version.dll')))
[IO.File]::WriteAllText((Join-Path $root 'session_controls.ini'), 'Keep my custom presets')
$result = Invoke-DualStickUpdate $root -Decide {
    param($old, $new)
    if ($old -ne [version]'5.158' -or $new.Version -ne [version]'6.0' -or $new.Notes -notmatch 'v6.0 - Stable release') { throw 'Incorrect v6 prompt data' }
    return 'update'
}
if ($result -ne 'updated-6.0' -or (Get-DualStickInstalledVersion (Join-Path $root 'version.dll')) -ne [version]'6.0') { throw 'v5 to v6 migration failed' }
if ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $root 'version.dll')))) -ne $manifest.dll_sha256) { throw 'v6 installed DLL mismatch' }
if ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $root 'version.dll.dualstick-previous')))) -ne $oldHash) { throw 'v5 backup missing' }
if ([IO.File]::ReadAllText((Join-Path $root 'session_controls.ini')) -ne 'Keep my custom presets') { throw 'v6 migration altered settings' }
if ((Invoke-DualStickUpdate $root -Decide { throw 'Current v6 prompted' }) -ne 'current') { throw 'v6 current check failed' }
$release.prerelease = $true
$failed = $false; try { Invoke-DualStickUpdate $root -CheckOnly } catch { $failed = $true }
if (-not $failed) { throw 'Preview release was accepted' }
$release.prerelease = $false
$asset.browser_download_url = 'https://example.invalid/untrusted.zip'
$failed = $false; try { Invoke-DualStickUpdate $root -CheckOnly } catch { $failed = $true }
if (-not $failed) { throw 'Unexpected download host was accepted' }
Write-Output 'PASS: real v6 package installs from v5 with notes/choice, correct manifest/asset selection, preserved INI and backup; current v6 stays silent; preview and foreign download locations rejected'
