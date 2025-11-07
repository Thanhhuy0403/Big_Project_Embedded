@echo off
echo ========================================
echo   Building and Uploading Filesystem
echo ========================================
echo.

REM Try to find PlatformIO
set PIO_CMD=
where pio >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=pio
    goto :found
)

where platformio >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=platformio
    goto :found
)

REM Try common PlatformIO installation paths
if exist "%USERPROFILE%\.platformio\penv\Scripts\pio.exe" (
    set "PIO_CMD=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
    goto :found
)

if exist "%USERPROFILE%\.platformio\penv\Scripts\platformio.exe" (
    set "PIO_CMD=%USERPROFILE%\.platformio\penv\Scripts\platformio.exe"
    goto :found
)

REM Try Python module
python -m platformio --version >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=python -m platformio
    goto :found
)

REM If still not found, show error
echo ERROR: PlatformIO not found!
echo.
echo Please ensure PlatformIO is installed and in your PATH.
echo Or install it via: pip install platformio
echo.
pause
exit /b 1

:found
echo Using PlatformIO: %PIO_CMD%
echo.

echo [1/2] Building filesystem image...
"%PIO_CMD%" run --target buildfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build filesystem failed!
    pause
    exit /b %errorlevel%
)

echo.
echo [2/2] Uploading filesystem to ESP32...
"%PIO_CMD%" run --target uploadfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Upload filesystem failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================
echo   SUCCESS! Filesystem uploaded!
echo ========================================
pause

