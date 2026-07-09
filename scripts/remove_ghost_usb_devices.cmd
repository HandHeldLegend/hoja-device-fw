@echo off
setlocal

:: Remove unplugged USB/HID device entries from Device Manager
:: (the gray/ghost devices shown with View -> Show hidden devices)
::
:: Run as Administrator. Re-plug devices afterward if you still use them —
:: Windows will re-enumerate them cleanly.

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Requesting Administrator privileges...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b 1
)

echo.
echo Removing unplugged USB and HID device entries...
echo.

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0remove_ghost_usb_devices.ps1"

echo.
pause
endlocal
