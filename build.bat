@echo off
REM Build script for Discord Voice Receive Prototype on Windows with vcpkg

echo ==========================================
echo   Discord Voice Receive - Build Script
echo ==========================================

REM Check for vcpkg
if not defined VCPKG_ROOT (
    echo Error: VCPKG_ROOT environment variable is not set.
    echo Please set VCPKG_ROOT to your vcpkg installation directory.
    echo Example: set VCPKG_ROOT=C:\vcpkg
    exit /b 1
)

REM Check if vcpkg directory exists
if not exist "%VCPKG_ROOT%" (
    echo Error: vcpkg directory not found at %VCPKG_ROOT%
    exit /b 1
)

REM Bootstrap vcpkg if needed
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    cd /d "%VCPKG_ROOT%"
    call bootstrap-vcpkg.bat
)

REM Install dependencies
echo.
echo Installing dependencies via vcpkg...
cd /d "%VCPKG_ROOT%"
call vcpkg install opus openssl zlib:x64-windows

REM Create build directory
set BUILD_DIR=%~dp0build
if exist "%BUILD_DIR%" (
    echo Removing existing build directory...
    rmdir /s /q "%BUILD_DIR%"
)

echo.
echo Creating build directory...
mkdir "%BUILD_DIR%"

echo.
echo Configuring with CMake...
cd /d "%BUILD_DIR%"
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DVCPKG_MANIFEST_MODE=ON

if %ERRORLEVEL% neq 0 (
    echo.
    echo CMake configuration failed!
    exit /b 1
)

echo.
echo Building...
cmake --build . --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed!
    exit /b 1
)

echo.
echo ==========================================
echo   Build complete!
echo ==========================================
echo.
echo To run the bot:
echo   set DISCORD_BOT_TOKEN=your-token
echo   set DISCORD_GUILD_ID=your-guild-id
echo   set DISCORD_VOICE_CHANNEL_ID=your-channel-id
echo   build\Release\discord_voice_receive.exe
echo.
