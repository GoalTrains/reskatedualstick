param([string]$GameDirectory = (Split-Path $PSScriptRoot -Parent), [switch]$CheckOnly, [switch]$Interactive)
$ErrorActionPreference = 'Stop'
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Import-Module (Join-Path $PSScriptRoot 'DualStickUpdater.psm1') -Force
    $decision = $null
    $previous = $null
    if ($Interactive -and -not $CheckOnly) {
        . (Join-Path $PSScriptRoot 'Update-Dialog.ps1')
        $previous = Get-DualStickInstalledVersion (Join-Path $GameDirectory 'version.dll')
        $decision = { param($installed, $release) Show-DualStickUpdateChoice $installed $release }
    }
    $result = Invoke-DualStickUpdate $GameDirectory -CheckOnly:$CheckOnly -Decide $decision
    Write-DualStickUpdateLog $GameDirectory $result
    Write-Output $result
    if ($Interactive -and $result -like 'updated-*') {
        $from = if ($previous) { 'v' + $previous } else { 'Not installed' }
        Show-DualStickUpdateNotice "Dual Stick updated: $from -> v$($result.Substring(8)).`r`n`r`nYour settings were kept. Press OK to open ReSkate."
    }
} catch {
    if (Get-Command Write-DualStickUpdateLog -ErrorAction SilentlyContinue) {
        Write-DualStickUpdateLog $GameDirectory ('Update skipped: ' + $_.Exception.Message)
    }
    Write-Output ('Update skipped: ' + $_.Exception.Message)
    if ($Interactive -and (Get-Command Show-DualStickUpdateNotice -ErrorAction SilentlyContinue)) {
        Show-DualStickUpdateNotice ("No update was installed. ReSkate will open with your current version.`r`n`r`n" + $_.Exception.Message) -Failed
    }
    exit 1
}
