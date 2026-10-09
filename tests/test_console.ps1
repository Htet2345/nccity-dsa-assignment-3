$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$dataPath = Join-Path $projectRoot 'build\console_audit.tsv'
$badPath = Join-Path $projectRoot 'build\console_invalid.tsv'

function Invoke-Bank([string[]]$Lines, [string]$Path) {
    $output = ($Lines -join "`n") | & '.\build\bank.exe' $Path 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Console run failed: $output" }
    return ($output -join "`n")
}

function Assert-Contains([string]$Output, [string[]]$Expected) {
    foreach ($text in $Expected) {
        if (!$Output.Contains($text)) { throw "Expected console output: $text" }
    }
}

try {
    if (Test-Path -LiteralPath $dataPath) { Remove-Item -LiteralPath $dataPath }
    $output = Invoke-Bank @(
        'abc', '11', '2', '3', '999',
        '1', '101', 'Alice', '100.00',
        '1', '101',
        '1', '202', 'Bob', '0',
        '1', '303', 'Carol', '0',
        '1', '404', 'Dave', '0',
        '7', '303',
        '9', '101', 'Alice Updated', '3', '101',
        '4', '101', '-1', '0', '1.234', '20.00',
        '6', '101', '999', '6', '101', '101',
        '6', '101', '202', '30.00',
        '5', '101', '100.00', '7', '101',
        '10', '101', '10', '202', '8', '0'
    ) $dataPath
    Assert-Contains $output @(
        'No saved accounts found. Starting a new bank.', 'No accounts yet.',
        'Account not found.', 'Account number already exists.', 'Account deleted.',
        'Account information updated.', 'Alice Updated', 'Transaction completed.',
        'Choose a different, existing destination account.', 'Transaction refused:',
        'Withdraw or transfer the remaining balance before deletion.',
        'Opening balance', 'Deposit', 'Transfer out', 'Transfer in', 'Accounts saved.'
    )
    $saved = Get-Content -LiteralPath $dataPath
    if ($saved -notcontains "101`t9000`tAlice Updated" -or
        $saved -notcontains "202`t3000`tBob" -or
        @($saved | Where-Object { $_ -like '303*' }).Count -ne 0 -or
        @($saved | Where-Object { $_ -like "H`t*" }).Count -ne 6) {
        throw 'Unexpected balances, deletion result, or history count.'
    }
    $output = Invoke-Bank @('10', '101', '10', '202', '0') $dataPath
    Assert-Contains $output @('Alice Updated', '90.00', '30.00', 'Deposit', 'Transfer out', 'Transfer in')
    $output = Invoke-Bank @('5', '101', '90.00', '7', '101', '5', '202', '30.00', '7', '202', '7', '404', '2', '0') $dataPath
    Assert-Contains $output @('Account deleted.', 'No accounts yet.')
    if ((Get-Item -LiteralPath $dataPath).Length -ne 0) { throw 'Deleted bank should save an empty file.' }
    # Ending input during creation must not create a partial account.
    $output = Invoke-Bank @('1', '505') $dataPath
    if ((Get-Item -LiteralPath $dataPath).Length -ne 0) { throw 'Partial creation changed stored data.' }
    # An invalid file must fail startup and preserve the original bytes.
    [System.IO.File]::WriteAllText($badPath, "invalid data`n")
    $before = [System.IO.File]::ReadAllText($badPath)
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $output = '0' | & '.\build\bank.exe' $badPath 2>&1
    $failed = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    if ($failed -ne 1 -or [System.IO.File]::ReadAllText($badPath) -ne $before) {
        throw 'Invalid file was accepted or overwritten.'
    }
    Write-Output 'All console workflow tests passed.'
} finally {
    foreach ($path in @($dataPath, $badPath)) {
        if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path }
    }
}
