$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'DualStickUpdater.psm1') -Force
Add-Type -AssemblyName System.IO.Compression.FileSystem
$module = Get-Module DualStickUpdater
& $module { function script:Test-DualStickGameRunning { return $script:GameRunning }; $script:GameRunning = $false }
$root = Join-Path (Split-Path $PSScriptRoot -Parent) ('build/updater-tests-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($root) | Out-Null
$releaseRoot = Join-Path (Split-Path $PSScriptRoot -Parent) 'releases/v5.158'
$manifest = Get-Content -Raw (Join-Path $releaseRoot 'manifest.json') | ConvertFrom-Json
$package = Join-Path $releaseRoot 'Dual_Stick_Controls_v5.158.zip'
$release = [pscustomobject]@{ Version = [version]'5.158'; Manifest = $manifest; Package = $manifest.packages.'Dual_Stick_Controls_v5.158.zip'; Url = 'unused-in-tests' }
function Assert($Condition, [string]$Message) { if (-not $Condition) { throw $Message } }
function New-Case([string]$Name) { $p = Join-Path $root $Name; [IO.Directory]::CreateDirectory($p) | Out-Null; return $p }
function Read-Dll([string]$ZipPath) {
    $z = [IO.Compression.ZipFile]::OpenRead($ZipPath)
    try { $s = $z.GetEntry('Dual_Stick_Controls/version.dll').Open(); $m = New-Object IO.MemoryStream; $s.CopyTo($m); $s.Dispose(); return ,$m.ToArray() }
    finally { $z.Dispose() }
}
$old = Read-Dll (Join-Path (Split-Path $PSScriptRoot -Parent) 'releases/v5.157/Dual_Stick_Controls_v5.157.zip')
$oldHash = Get-DualStickHash $old
function Expect-Unchanged([string]$Name, [scriptblock]$Action) {
    $dir = New-Case $Name; [IO.File]::WriteAllBytes((Join-Path $dir 'version.dll'), $old)
    $failed = $false
    try { & $Action $dir } catch { $failed = $true }
    Assert $failed "$Name did not reject the update"
    Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $dir 'version.dll')))) -eq $oldHash) "$Name changed the installed DLL"
}
$dir = New-Case 'preserve-settings'; [IO.File]::WriteAllBytes((Join-Path $dir 'version.dll'), $old)
$ini = [Text.Encoding]::Unicode.GetBytes("[SessionControls]`r`nMyCustomSetting=42`r`n")
[IO.File]::WriteAllBytes((Join-Path $dir 'session_controls.ini'), $ini)
Install-DualStickPackage $dir $package $release
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $dir 'version.dll')))) -eq $manifest.dll_sha256) 'Update DLL mismatch'
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $dir 'version.dll.dualstick-previous')))) -eq $oldHash) 'Old DLL was not backed up'
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $dir 'session_controls.ini')))) -eq (Get-DualStickHash $ini)) 'Existing INI changed'
$fresh = New-Case 'fresh'; Install-DualStickPackage $fresh $package $release
Assert ([IO.File]::Exists((Join-Path $fresh 'session_controls.ini'))) 'Fresh INI missing'
$badPackage = Join-Path $root 'corrupt.zip'; [IO.File]::WriteAllBytes($badPackage, [byte[]](1,2,3))
Expect-Unchanged 'bad-download' { param($d) Install-DualStickPackage $d $badPackage $release }
& $module { $script:GameRunning = $true }
Expect-Unchanged 'game-running' { param($d) Install-DualStickPackage $d $package $release }
& $module { $script:GameRunning = $false }
Expect-Unchanged 'locked-dll' { param($d)
    $lock = New-Object IO.FileStream((Join-Path $d 'version.dll'), [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
    try { Install-DualStickPackage $d $package $release } finally { $lock.Dispose() }
}
$newer = New-Case 'newer-local'; $text = [Text.Encoding]::ASCII.GetString($old)
$offset = $text.IndexOf('---- Dual Stick Controls 5.157') + '---- Dual Stick Controls '.Length
$future = [byte[]]$old.Clone(); [Array]::Copy([Text.Encoding]::ASCII.GetBytes('5.999'), 0, $future, $offset, 5)
[IO.File]::WriteAllBytes((Join-Path $newer 'version.dll'), $future)
$failed = $false; try { Install-DualStickPackage $newer $package $release } catch { $failed = $true }
Assert $failed 'Newer local build was downgraded'
Assert ((Get-DualStickInstalledVersion (Join-Path $newer 'version.dll')) -eq [version]'5.999') 'Newer DLL changed'
$preview = New-Case 'internal-catch-preview'; $previewDll=Join-Path $preview 'version.dll'
[IO.File]::WriteAllText($previewDll,'---- Dual Stick Controls 6.15 EARLY MANUAL-CATCH TEST')
Assert ((Get-DualStickInstalledVersion $previewDll) -eq [version]'6.15') 'Internal preview version was hidden'
$previewVersion=[version]'6.15'; $futureVersion=[version]'6.22'
Assert ((& $module { param($v) Get-DualStickComparisonVersion $v } $previewVersion) -eq [version]'6.0') 'Internal catch preview would block the v6.1 stable update'
Assert ((& $module { param($v) Get-DualStickComparisonVersion $v } $futureVersion) -eq [version]'6.22') 'Future stable versions were incorrectly treated as previews'

foreach ($cameraPreview in @('6.16','6.17','6.18','6.19','6.20','6.21')) {
    Assert ((& $module { param($v) Get-DualStickComparisonVersion $v } ([version]$cameraPreview)) -eq [version]'6.0') 'Private camera/menu build blocks stable 6.2'
}
Assert ((& $module { Get-DualStickComparisonVersion ([version]'6.2') }) -eq [version]'6.2') 'Stable 6.2 treated as a preview'

# Verify archive ambiguity, altered DLL and traversal names without extracting paths.
foreach ($kind in @('duplicate','tampered','traversal')) {
    $path = Join-Path $root ($kind + '.zip'); $z = [IO.Compression.ZipFile]::Open($path, [IO.Compression.ZipArchiveMode]::Create)
    try {
        $dll = Read-Dll $package
        if ($kind -eq 'tampered') { $dll[200] = $dll[200] -bxor 1 }
        $names = @('Dual_Stick_Controls/version.dll','Dual_Stick_Controls/session_controls.ini')
        if ($kind -eq 'duplicate') { $names += 'Dual_Stick_Controls/version.dll' }
        if ($kind -eq 'traversal') { $names += '../escape.txt' }
        foreach ($name in $names) {
            $s = $z.CreateEntry($name).Open()
            $b = if ($name -like '*.dll') { $dll } else { [Text.Encoding]::UTF8.GetBytes('test') }
            $s.Write($b, 0, $b.Length); $s.Dispose()
        }
    } finally { $z.Dispose() }
    $copy = $release | Select-Object *
    $copy.Package = [pscustomobject]@{ size = (Get-Item $path).Length; sha256 = Get-DualStickHash ([IO.File]::ReadAllBytes($path)) }
    if ($kind -eq 'traversal') {
        $d = New-Case $kind; Install-DualStickPackage $d $path $copy
        Assert (-not [IO.File]::Exists((Join-Path $root 'escape.txt'))) 'Archive traversal wrote outside destination'
    } else { Expect-Unchanged $kind { param($d) Install-DualStickPackage $d $path $copy } }
}
& $module { function script:Get-DualStickRelease { throw 'offline fixture' } }
Expect-Unchanged 'offline' { param($d) Invoke-DualStickUpdate $d }
$disabled = New-Case 'disabled'; [IO.File]::WriteAllText((Join-Path $disabled 'dual_stick_updater.json'), '{"enabled":false}')
Assert ((Invoke-DualStickUpdate $disabled) -eq 'disabled') 'Disabled updater attempted network'
& $module { param($r) $script:FixtureRelease = $r; function script:Get-DualStickRelease { return $script:FixtureRelease } } $release
Assert ((Invoke-DualStickUpdate $fresh) -eq 'current') 'Current DLL attempted reinstall'
$check = New-Case 'check-only'; [IO.File]::WriteAllBytes((Join-Path $check 'version.dll'), $old)
Assert ((Invoke-DualStickUpdate $check -CheckOnly) -eq 'available-5.158') 'Check-only result wrong'
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $check 'version.dll')))) -eq $oldHash) 'Check-only changed DLL'
Assert ((Invoke-DualStickUpdate $check -Decide { 'later' }) -eq 'deferred') 'Closing dialog did not defer'
Assert (-not [IO.File]::Exists((Join-Path $check 'dual_stick_updater.json'))) 'Closing dialog saved an unwanted preference'
Assert ((Invoke-DualStickUpdate $check -Decide { 'skip' }) -eq 'skipped-5.158') 'Skip release failed'
Assert ((Invoke-DualStickUpdate $check -Decide { throw 'Skipped release prompted again' }) -eq 'skipped-5.158') 'Skipped release was not retained'
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $check 'version.dll')))) -eq $oldHash) 'Declined update changed DLL'
$next = $release | Select-Object *; $next.Version = [version]'5.159'
& $module { param($r) $script:FixtureRelease = $r } $next
Assert ((Invoke-DualStickUpdate $check -Decide { 'disable' }) -eq 'disabled') 'Next release did not allow disabling'
$saved = [IO.File]::ReadAllText((Join-Path $check 'dual_stick_updater.json')) | ConvertFrom-Json
Assert ($saved.enabled -eq $false -and $saved.skipped_version -eq '5.158') 'Preference save lost existing settings'
Assert ((Invoke-DualStickUpdate $check -Decide { throw 'Disabled updater prompted' }) -eq 'disabled') 'Disable was not retained'
& $module { param($r, $path) $script:FixtureRelease = $r; $script:FixturePackage = $path; function script:Invoke-WebRequest { param($Uri, $OutFile, $TimeoutSec, $Headers, [switch]$UseBasicParsing) [IO.File]::Copy($script:FixturePackage, $OutFile) } } $release $package
$accepted = New-Case 'accepted'; [IO.File]::WriteAllBytes((Join-Path $accepted 'version.dll'), $old)
Assert ((Invoke-DualStickUpdate $accepted -Decide { param($installed, $available) if ($installed -ne [version]'5.157' -or $available.Version -ne [version]'5.158') { throw 'Dialog version data wrong' }; 'update' }) -eq 'updated-5.158') 'Approved update failed'
Assert ((Get-DualStickHash ([IO.File]::ReadAllBytes((Join-Path $accepted 'version.dll')))) -eq $manifest.dll_sha256) 'Approved DLL mismatch'
Write-Output 'PASS: verified install/backup, exact INI preservation, fresh install, current/check-only, no downgrade, running/locked game, bad hash, duplicate/tampered entries, traversal, offline and disabled behavior'
Write-Output 'PASS: close defers without preferences; skip persists for only that release; newer releases prompt; disable persists without losing other preferences; approved update receives correct versions and installs verified bytes'
