# Remove unplugged USB/HID device entries (Device Manager ghost devices).
# Requires Administrator. Uses pnputil (built into Windows 10 2004+).

$pnputil = Join-Path $env:Windir 'System32\pnputil.exe'
if (-not (Test-Path -LiteralPath $pnputil)) {
    Write-Error 'pnputil.exe not found. Requires Windows 10 version 2004 or later.'
    exit 1
}

$raw = & $pnputil /enum-devices /disconnected 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) {
    Write-Error "pnputil failed: $raw"
    exit 1
}

$removed = 0
$failed = 0
$skipped = 0
$found = 0

$blocks = $raw -split '(?=\r?\nInstance ID:)'
foreach ($block in $blocks) {
    if ($block -notmatch 'Instance ID:\s+(\S+)') {
        continue
    }

    $id = $Matches[1]
    if ($id -notmatch '^(USB|HID)\\') {
        $skipped++
        continue
    }

    $found++
    $desc = '(no name)'
    if ($block -match 'Device Description:\s+(.+)') {
        $desc = $Matches[1].Trim()
    }

    Write-Host "Removing: $desc"
    Write-Host "  $id"

    $result = & $pnputil /remove-device $id 2>&1 | Out-String
    if ($LASTEXITCODE -eq 0) {
        $removed++
    }
    else {
        Write-Host "  Failed: $($result.Trim())" -ForegroundColor Red
        $failed++
    }
}

Write-Host ''
if ($found -eq 0) {
    Write-Host 'No unplugged USB/HID ghost devices found.'
}
else {
    Write-Host "Done. Removed: $removed, Failed: $failed"
}
