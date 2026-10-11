param([string]$LlvmBin = '')
$ErrorActionPreference = 'Stop'
if ($LlvmBin) {
    $compilerPath = Join-Path $LlvmBin 'clang.exe'
    if (-not (Test-Path -LiteralPath $compilerPath)) { throw "Missing compiler: $compilerPath" }
} else {
    $compilerPath = (Get-Command clang.exe -ErrorAction Stop).Source
}
$testOutputPath = Join-Path $PSScriptRoot 'build/tests'
New-Item -ItemType Directory -Path $testOutputPath -Force | Out-Null
$originalSearchPath = $env:PATH
try {
    $env:PATH = (Split-Path $compilerPath) + ';' + $env:PATH
    foreach ($testFile in Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'test') -Filter '*.c') {
        $testExePath = Join-Path $testOutputPath ($testFile.BaseName + '.exe')
        & $compilerPath -std=c11 -O2 $testFile.FullName -o $testExePath
        if ($LASTEXITCODE -ne 0) { throw "Compile failed: $($testFile.Name)" }
        & $testExePath
        if ($LASTEXITCODE -ne 0) { throw "Test failed: $($testFile.Name)" }
    }
} finally { $env:PATH = $originalSearchPath }
