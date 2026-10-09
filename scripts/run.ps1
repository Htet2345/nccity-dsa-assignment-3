$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1')
& (Join-Path (Split-Path -Parent $PSScriptRoot) 'build\bank.exe')
exit $LASTEXITCODE
