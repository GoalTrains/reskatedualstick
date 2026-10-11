param([string]$LlvmBin = '')
$ErrorActionPreference = 'Stop'
function Find-LlvmTool([string]$Name) {
    if ($LlvmBin) {
        $toolPath = Join-Path $LlvmBin ($Name + '.exe')
        if (-not (Test-Path -LiteralPath $toolPath)) { throw "Missing LLVM tool: $toolPath" }
        return $toolPath
    }
    $found = Get-Command ($Name + '.exe') -ErrorAction SilentlyContinue
    if (-not $found) { throw "LLVM tool $Name.exe is required. Pass -LlvmBin with an LLVM bin directory." }
    return $found.Source
}
$clangPath = Find-LlvmTool 'clang'
$linkFlavor = @()
try { $linkPath = Find-LlvmTool 'lld-link' }
catch { $linkPath = Find-LlvmTool 'ld.lld'; $linkFlavor = @('-flavor', 'link') }
$dlltoolPath = Find-LlvmTool 'llvm-dlltool'
$buildPath = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Path $buildPath -Force | Out-Null
Push-Location $PSScriptRoot
try {
    foreach ($libraryName in @('kernel32', 'user32', 'gdi32')) {
        & $dlltoolPath -m i386:x86-64 -d "src/$libraryName.def" -l "build/$libraryName.lib"
        if ($LASTEXITCODE -ne 0) { throw "Import-library generation failed for $libraryName" }
    }
    & $clangPath --target=x86_64-pc-windows-msvc -O2 -ffreestanding -fno-stack-protector -mno-stack-arg-probe -fno-math-errno -c src/version_proxy.c -o build/version_proxy.obj
    if ($LASTEXITCODE -ne 0) { throw 'C compilation failed' }
    & $linkPath @linkFlavor /nologo /dll /nodefaultlib /entry:DllMain /def:src/version.def /out:build/version.dll build/version_proxy.obj build/kernel32.lib build/user32.lib build/gdi32.lib
    if ($LASTEXITCODE -ne 0) { throw 'DLL linking failed' }
    Get-FileHash -LiteralPath (Join-Path $buildPath 'version.dll') -Algorithm SHA256
} finally { Pop-Location }
