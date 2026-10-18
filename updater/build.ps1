$ErrorActionPreference = 'Stop'
$compiler = Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
$output = Join-Path (Split-Path $PSScriptRoot -Parent) 'build/DualStickLauncher.exe'
& $compiler /nologo /target:winexe /platform:anycpu /optimize+ /reference:System.Windows.Forms.dll "/out:$output" (Join-Path $PSScriptRoot 'DualStickLauncher.cs')
if ($LASTEXITCODE -ne 0) { throw 'Updater launcher compilation failed' }
Get-FileHash -LiteralPath $output -Algorithm SHA256
