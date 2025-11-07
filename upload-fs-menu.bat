@echo off
REM Try to find PlatformIO
set PIO_CMD=
where pio >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=pio
    goto :menu
)

where platformio >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=platformio
    goto :menu
)

if exist "%USERPROFILE%\.platformio\penv\Scripts\pio.exe" (
    set "PIO_CMD=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
    goto :menu
)

if exist "%USERPROFILE%\.platformio\penv\Scripts\platformio.exe" (
    set "PIO_CMD=%USERPROFILE%\.platformio\penv\Scripts\platformio.exe"
    goto :menu
)

python -m platformio --version >nul 2>&1
if %errorlevel% equ 0 (
    set PIO_CMD=python -m platformio
    goto :menu
)

echo ERROR: PlatformIO not found!
echo Please ensure PlatformIO is installed and in your PATH.
pause
exit /b 1

:menu
cls
echo ========================================
echo   ESP32 Filesystem Tools
echo ========================================
echo Using: %PIO_CMD%
echo.
echo 1. Build filesystem only
echo 2. Upload filesystem only
echo 3. Build and Upload (Full)
echo 4. Clean and Rebuild
echo 5. Exit
echo.
set /p choice="Choose option (1-5): "

if "%choice%"=="1" goto build
if "%choice%"=="2" goto upload
if "%choice%"=="3" goto full
if "%choice%"=="4" goto clean
if "%choice%"=="5" exit
goto menu

:build
echo.
echo Building filesystem image...
"%PIO_CMD%" run --target buildfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    goto menu
)
echo.
echo SUCCESS! Filesystem image built.
pause
goto menu

:upload
echo.
echo Uploading filesystem...
"%PIO_CMD%" run --target uploadfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Upload failed!
    pause
    goto menu
)
echo.
echo SUCCESS! Filesystem uploaded.
pause
goto menu

:full
echo.
echo [1/2] Building filesystem image...
"%PIO_CMD%" run --target buildfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    goto menu
)
echo.
echo [2/2] Uploading filesystem...
"%PIO_CMD%" run --target uploadfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Upload failed!
    pause
    goto menu
)
echo.
echo ========================================
echo   SUCCESS! Filesystem uploaded!
echo ========================================
pause
goto menu

:clean
echo.
echo Cleaning build...
"%PIO_CMD%" run --target clean
echo.
echo [1/2] Building filesystem image...
"%PIO_CMD%" run --target buildfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    goto menu
)
echo.
echo [2/2] Uploading filesystem...
"%PIO_CMD%" run --target uploadfs
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Upload failed!
    pause
    goto menu
)
echo.
echo ========================================
echo   SUCCESS! Clean rebuild completed!
echo ========================================
pause
goto menu

