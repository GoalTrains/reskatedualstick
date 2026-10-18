$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$compiler = Join-Path $env:WINDIR 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
$root = Join-Path $project ('build/launcher ordering ' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($root) | Out-Null
$stub = Join-Path $root 'Stub.cs'
[IO.File]::WriteAllText($stub, @'
using System;
using System.IO;
class Stub {
    static void Main(string[] args) {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        File.AppendAllText(Path.Combine(root,"order.txt"),"launcher-started\n");
        File.WriteAllLines(Path.Combine(root,"arguments.txt"),args);
    }
}
'@)
& $compiler /nologo /target:winexe "/out:$(Join-Path $root 'ReSkateLauncher.exe')" $stub
if ($LASTEXITCODE -ne 0) { throw 'Fixture compilation failed' }
Copy-Item -LiteralPath (Join-Path $project 'build/DualStickLauncher.exe') -Destination $root
$scripts = Join-Path $root 'updater'; [IO.Directory]::CreateDirectory($scripts) | Out-Null
[IO.File]::WriteAllText((Join-Path $scripts 'Update-DualStick.ps1'), @'
param([string]$GameDirectory)
[IO.File]::AppendAllText((Join-Path $GameDirectory 'order.txt'), "update-finished`n")
exit 1
'@)
$process = Start-Process -FilePath (Join-Path $root 'DualStickLauncher.exe') -ArgumentList '--no-gui "a path with spaces" "a\"quote"' -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit(10000)) { throw 'Launcher wrapper did not finish' }
$argumentsPath = Join-Path $root 'arguments.txt'
$timer = [Diagnostics.Stopwatch]::StartNew()
while (-not [IO.File]::Exists($argumentsPath) -and $timer.ElapsedMilliseconds -lt 5000) { Start-Sleep -Milliseconds 25 }
$order = [IO.File]::ReadAllLines((Join-Path $root 'order.txt'))
if ($order.Length -ne 2 -or $order[0] -ne 'update-finished' -or $order[1] -ne 'launcher-started') { throw 'Official launcher started before update attempt finished' }
$arguments = [IO.File]::ReadAllLines($argumentsPath)
if ($arguments.Length -ne 3 -or $arguments[0] -ne '--no-gui' -or $arguments[1] -ne 'a path with spaces' -or $arguments[2] -ne 'a"quote') { throw 'Launcher arguments changed' }
Write-Output 'PASS: update completes before official launcher; failed updater still opens launcher; paths with spaces and quoted arguments preserved; no real game was started'
