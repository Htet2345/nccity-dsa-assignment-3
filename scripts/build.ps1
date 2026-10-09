param([switch]$Test)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$compiler = Join-Path $projectRoot '.tools\w64devkit\bin\gcc.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'C compiler missing. Run .\scripts\setup.ps1 first.' }
New-Item -ItemType Directory -Path 'build' -Force | Out-Null
$common = @('-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-g', '-O0', '-Iinclude')
if ($Test) {
    & $compiler @common 'tests/test_bank.c' 'src/bank.c' 'src/storage.c' '-o' 'build/test_bank.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    & '.\build\test_bank.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
    & $compiler @common 'tests/test_input.c' 'src/input.c' '-o' 'build/test_input.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Input test compilation failed.' }
    & '.\build\test_input.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Input tests failed.' }
    & $compiler @common 'src/main.c' 'src/bank.c' 'src/input.c' 'src/storage.c' '-o' 'build/bank.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Console test compilation failed.' }
    & (Join-Path $projectRoot 'tests\test_console.ps1')
} else {
    & $compiler @common 'src/main.c' 'src/bank.c' 'src/input.c' 'src/storage.c' '-o' 'build/bank.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    Write-Host 'Built build\bank.exe'
}
