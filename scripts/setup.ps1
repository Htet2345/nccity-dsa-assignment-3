$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$toolsPath = Join-Path $projectRoot '.tools'
$compiler = Join-Path $toolsPath 'w64devkit\bin\gcc.exe'
if (Test-Path -LiteralPath $compiler) {
    Write-Host 'Portable compiler already installed.'
    exit 0
}
New-Item -ItemType Directory -Path $toolsPath -Force | Out-Null
$archivePath = Join-Path $toolsPath 'w64devkit-x64-2.10.0.7z.exe'
$url = 'https://github.com/skeeto/w64devkit/releases/download/v2.10.0/w64devkit-x64-2.10.0.7z.exe'
if (!(Test-Path -LiteralPath $archivePath)) {
    & curl.exe '-fL' '--retry' '2' '--connect-timeout' '15' '--max-time' '300' $url '-o' $archivePath
    if ($LASTEXITCODE -ne 0) { throw 'Compiler download failed. Check your internet connection and retry.' }
}
$expected = '18d0a4c71a166f8401ab6305781bec5882b40b5e06ba9807c61cb5f3b3c6325e'
if ((Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash -ne $expected) {
    throw 'Compiler archive checksum mismatch. Delete the archive in .tools and retry setup.'
}
# Official release is a self-extracting 7-Zip archive. Extract within this project.
$extractor = Start-Process -FilePath $archivePath -ArgumentList @('-y', "-o`"$toolsPath`"") -WindowStyle Hidden -Wait -PassThru
if ($extractor.ExitCode -ne 0 -or !(Test-Path -LiteralPath $compiler)) { throw 'Compiler extraction failed.' }
Write-Host 'Setup complete. Use Ctrl+Shift+B to build or F5 to debug in VS Code.'
