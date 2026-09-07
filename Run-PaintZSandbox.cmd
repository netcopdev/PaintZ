@echo off
setlocal

where pwsh.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    pwsh.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\sandbox\Start-PaintZSandbox.ps1" %*
) else (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\sandbox\Start-PaintZSandbox.ps1" %*
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo PaintZ sandbox failed. Review the error above.
    pause
)

