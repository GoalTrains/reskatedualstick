Set-StrictMode -Version 2
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression

function Get-DualStickHash([byte[]]$Bytes) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash($Bytes))).Replace('-', '').ToLowerInvariant() }
    finally { $sha.Dispose() }
}

function Get-DualStickInstalledVersion([string]$Path) {
    if (-not [IO.File]::Exists($Path)) { return $null }
    $text = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($Path))
    $match = [regex]::Match($text, '---- Dual Stick Controls (\d+\.\d+(?:\.\d+)?)')
    if (-not $match.Success) { throw 'version.dll is not a recognised Dual Stick build; leaving it unchanged.' }
    return [version]$match.Groups[1].Value
}

function Get-DualStickComparisonVersion([version]$Version) {
    # These were local catch/camera/menu builds, never stable releases. Allow
    # the stable release to replace them despite their internal build tags.
    if ($Version -and $Version.Major -eq 6 -and $Version.Minor -ge 13 -and $Version.Minor -le 21) {
        return [version]'6.0'
    }
    return $Version
}

function Test-DualStickGameRunning([string]$GameDirectory) {
    foreach ($process in @(Get-Process -Name Skate -ErrorAction SilentlyContinue)) {
        try {
            if ([IO.Path]::GetFullPath($process.Path) -ieq (Join-Path $GameDirectory 'Skate.exe')) { return $true }
        } catch { return $true } # Unknown ownership: do not risk replacing a loaded DLL.
    }
    return $false
}

function Write-DualStickUpdateLog([string]$GameDirectory, [string]$Message) {
    try {
        $path = Join-Path $GameDirectory 'dual_stick_updater.log'
        if ([IO.File]::Exists($path) -and (Get-Item -LiteralPath $path).Length -gt 65536) {
            [IO.File]::WriteAllText($path, '')
        }
        [IO.File]::AppendAllText($path, ('{0:u} {1}{2}' -f [DateTime]::UtcNow, $Message, [Environment]::NewLine))
    } catch { } # Logging must not prevent starting the official launcher.
}

function Get-DualStickRemoteJson([string]$Url) {
    $response = Invoke-WebRequest -UseBasicParsing -Uri $Url -TimeoutSec 8 -Headers @{
        'User-Agent' = 'ReSkate-DualStick-Updater'; 'Accept' = 'application/vnd.github+json'
    }
    if ($response.Content.Length -gt 1048576) { throw 'Update metadata is too large.' }
    return ($response.Content | ConvertFrom-Json)
}

function Get-DualStickRelease {
    $release = Get-DualStickRemoteJson 'https://api.github.com/repos/GoalTrains/reskatedualstick/releases/latest'
    if ($release.draft -or $release.prerelease) { throw 'Only published stable releases can be installed.' }
    $tag = [string]$release.tag_name
    if ($tag -notmatch '^v?\d+\.\d+(?:\.\d+)?$') { throw 'Unrecognised stable release tag.' }
    $number = $tag.TrimStart('v')
    $manifest = Get-DualStickRemoteJson ("https://raw.githubusercontent.com/GoalTrains/reskatedualstick/$tag/releases/v$number/manifest.json")
    if ([string]$manifest.version -ne $number -or $manifest.prerelease -ne $false) { throw 'Release/manifest version mismatch.' }
    $name = "Dual_Stick_Controls_v$number.zip"
    $assets = @($release.assets | Where-Object { $_.name -ceq $name })
    if ($assets.Count -ne 1) { throw 'The stable Windows package is missing or ambiguous.' }
    $asset = $assets[0]
    $expected = $manifest.packages.PSObject.Properties[$name].Value
    if (-not $expected -or $expected.sha256 -notmatch '^[a-fA-F0-9]{64}$' -or
        $manifest.dll_sha256 -notmatch '^[a-fA-F0-9]{64}$' -or
        $expected.size -lt 1 -or $expected.size -gt 16777216 -or $asset.size -ne $expected.size) {
        throw 'Invalid release hashes or package size.'
    }
    $url = "https://github.com/GoalTrains/reskatedualstick/releases/download/$tag/$name"
    if ([string]$asset.browser_download_url -cne $url) { throw 'Unexpected release download location.' }
    if ($asset.PSObject.Properties['digest'] -and $asset.digest -and $asset.digest -ne ('sha256:' + $expected.sha256)) {
        throw 'GitHub asset digest differs from the release manifest.'
    }
    $notes = if ($release.PSObject.Properties['body']) { [string]$release.body } else { '' }
    if ($notes.Length -gt 32768) { $notes = $notes.Substring(0, 32768) + "`r`n[Notes truncated]" }
    if ([string]::IsNullOrWhiteSpace($notes)) { $notes = 'No patch notes were provided for this release.' }
    return [pscustomobject]@{ Version = [version]$number; Manifest = $manifest; Package = $expected; Url = $url; Notes = $notes }
}

function Get-DualStickZipEntryBytes($Zip, [string]$Name, [long]$Maximum) {
    $entries = @($Zip.Entries | Where-Object { $_.FullName -ceq $Name })
    if ($entries.Count -ne 1 -or $entries[0].Length -lt 1 -or $entries[0].Length -gt $Maximum) {
        throw "Missing, duplicate or oversized package entry: $Name"
    }
    $stream = $entries[0].Open(); $memory = New-Object IO.MemoryStream
    try { $stream.CopyTo($memory); return ,$memory.ToArray() }
    finally { $stream.Dispose(); $memory.Dispose() }
}

function Install-DualStickPackage([string]$GameDirectory, [string]$PackagePath, $Release) {
    $bytes = [IO.File]::ReadAllBytes($PackagePath)
    if ($bytes.Length -ne $Release.Package.size -or (Get-DualStickHash $bytes) -ne $Release.Package.sha256) {
        throw 'Downloaded zip failed SHA256/size validation.'
    }
    $memory = New-Object IO.MemoryStream(,$bytes)
    $zip = New-Object IO.Compression.ZipArchive($memory, [IO.Compression.ZipArchiveMode]::Read)
    try {
        # Read only named entries; never extract arbitrary paths from a remote archive.
        $dll = Get-DualStickZipEntryBytes $zip 'Dual_Stick_Controls/version.dll' 16777216
        $ini = Get-DualStickZipEntryBytes $zip 'Dual_Stick_Controls/session_controls.ini' 1048576
    } finally { $zip.Dispose(); $memory.Dispose() }
    if ((Get-DualStickHash $dll) -ne $Release.Manifest.dll_sha256 -or $dll.Length -lt 256 -or
        $dll[0] -ne 0x4d -or $dll[1] -ne 0x5a) { throw 'Mod DLL failed SHA256/format validation.' }
    $pe = [BitConverter]::ToInt32($dll, 0x3c)
    if ($pe -lt 64 -or $pe -gt $dll.Length - 24 -or [BitConverter]::ToUInt32($dll, $pe) -ne 0x4550 -or
        [BitConverter]::ToUInt16($dll, $pe + 4) -ne 0x8664 -or
        ([BitConverter]::ToUInt16($dll, $pe + 22) -band 0x2000) -eq 0) { throw 'Package is not an x64 Windows DLL.' }
    if (Test-DualStickGameRunning $GameDirectory) { throw 'Skate started during the check; update deferred.' }
    $target = Join-Path $GameDirectory 'version.dll'
    $existing = Get-DualStickInstalledVersion $target
    if ($existing -and $existing -gt $Release.Version) { throw 'A newer local build is installed; downgrade skipped.' }
    $staged = Join-Path $GameDirectory ('version.dll.dualstick-new-' + [guid]::NewGuid().ToString('N'))
    try {
        [IO.File]::WriteAllBytes($staged, $dll)
        # One atomic replacement retains the old DLL if a lock or write error occurs.
        if ([IO.File]::Exists($target)) { [IO.File]::Replace($staged, $target, (Join-Path $GameDirectory 'version.dll.dualstick-previous'), $true) }
        else { [IO.File]::Move($staged, $target) }
    } finally { if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) } }
    $iniPath = Join-Path $GameDirectory 'session_controls.ini'
    if (-not [IO.File]::Exists($iniPath)) {
        $file = New-Object IO.FileStream($iniPath, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write)
        try { $file.Write($ini, 0, $ini.Length) } finally { $file.Dispose() }
    }
}

function Set-DualStickUpdatePreference([string]$GameDirectory, [string]$Name, $Value) {
    $path = Join-Path $GameDirectory 'dual_stick_updater.json'
    $config = if ([IO.File]::Exists($path)) { [IO.File]::ReadAllText($path) | ConvertFrom-Json } else { [pscustomobject]@{ enabled = $true } }
    $config | Add-Member -NotePropertyName $Name -NotePropertyValue $Value -Force
    $staged = $path + '.' + [guid]::NewGuid().ToString('N') + '.tmp'
    try {
        [IO.File]::WriteAllText($staged, ($config | ConvertTo-Json -Depth 8))
        if ([IO.File]::Exists($path)) { [IO.File]::Replace($staged, $path, ($path + '.previous'), $true) }
        else { [IO.File]::Move($staged, $path) }
    } finally { if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) } }
}

function Invoke-DualStickUpdate([string]$GameDirectory, [switch]$CheckOnly, [scriptblock]$Decide) {
    $GameDirectory = [IO.Path]::GetFullPath($GameDirectory)
    $settings = Join-Path $GameDirectory 'dual_stick_updater.json'
    $config = $null
    if ([IO.File]::Exists($settings)) {
        $config = [IO.File]::ReadAllText($settings) | ConvertFrom-Json
        if ($config.PSObject.Properties['enabled'] -and $config.enabled -eq $false) { return 'disabled' }
    }
    if (Test-DualStickGameRunning $GameDirectory) { return 'game-running' }
    $key = Get-DualStickHash ([Text.Encoding]::UTF8.GetBytes($GameDirectory.ToLowerInvariant()))
    $mutex = New-Object Threading.Mutex($false, ('Local\DualStickUpdate-' + $key)); $locked = $false; $folder = $null
    try {
        try { $locked = $mutex.WaitOne(0) } catch [Threading.AbandonedMutexException] { $locked = $true }
        if (-not $locked) { return 'already-checking' }
        $release = Get-DualStickRelease
        $target = Join-Path $GameDirectory 'version.dll'
        $installed = Get-DualStickInstalledVersion $target
        if ($installed -and (Get-DualStickComparisonVersion $installed) -gt $release.Version) { return 'newer-local-build' }
        if ([IO.File]::Exists($target) -and (Get-DualStickHash ([IO.File]::ReadAllBytes($target))) -eq $release.Manifest.dll_sha256) { return 'current' }
        if ($CheckOnly) { return ('available-' + $release.Version) }
        if ($config -and $config.PSObject.Properties['skipped_version'] -and
            [string]$config.skipped_version -eq $release.Version.ToString()) { return ('skipped-' + $release.Version) }
        if ($Decide) {
            $choice = & $Decide $installed $release
            switch ($choice) {
                'update' { }
                'skip' { Set-DualStickUpdatePreference $GameDirectory 'skipped_version' $release.Version.ToString(); return ('skipped-' + $release.Version) }
                'disable' { Set-DualStickUpdatePreference $GameDirectory 'enabled' $false; return 'disabled' }
                default { return 'deferred' } # Close/Escape never means approval.
            }
        }
        $folder = Join-Path $GameDirectory ('.dualstick-update-' + [guid]::NewGuid().ToString('N'))
        [IO.Directory]::CreateDirectory($folder) | Out-Null
        $package = Join-Path $folder 'package.zip'
        Invoke-WebRequest -UseBasicParsing -Uri $release.Url -OutFile $package -TimeoutSec 30 -Headers @{ 'User-Agent' = 'ReSkate-DualStick-Updater' }
        Install-DualStickPackage $GameDirectory $package $release
        return ('updated-' + $release.Version)
    } finally {
        if ($folder -and [IO.Directory]::Exists($folder)) { [IO.Directory]::Delete($folder, $true) }
        if ($locked) { $mutex.ReleaseMutex() }
        $mutex.Dispose()
    }
}

Export-ModuleMember -Function Invoke-DualStickUpdate, Install-DualStickPackage, Get-DualStickHash, Get-DualStickInstalledVersion, Write-DualStickUpdateLog
