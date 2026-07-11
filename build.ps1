#!/usr/bin/env pwsh
# Build script for Discord Voice Receive Prototype on Windows with vcpkg (PowerShell)

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Discord Voice Receive - Build Script" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Check for vcpkg
if (-not $env:VCPKG_ROOT) {
    Write-Host "Error: VCPKG_ROOT environment variable is not set." -ForegroundColor Red
    Write-Host "Please set VCPKG_ROOT to your vcpkg installation directory." -ForegroundColor Yellow
    Write-Host "Example: `$env:VCPKG_ROOT = 'C:\vcpkg'" -ForegroundColor Yellow
    exit 1
}

# Check if vcpkg directory exists
if (-not (Test-Path $env:VCPKG_ROOT)) {
    Write-Host "Error: vcpkg directory not found at $env:VCPKG_ROOT" -ForegroundColor Red
    exit 1
}

# Bootstrap vcpkg if needed
$vcpkgExe = Join-Path $env:VCPKG_ROOT "vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    Write-Host "Bootstrapping vcpkg..." -ForegroundColor Yellow
    Push-Location $env:VCPKG_ROOT
    & cmd /c "bootstrap-vcpkg.bat"
    Pop-Location
}

# Install dependencies
Write-Host ""
Write-Host "Installing dependencies via vcpkg..." -ForegroundColor Cyan
Push-Location $env:VCPKG_ROOT
& .\vcpkg install opus:x64-windows openssl:x64-windows zlib:x64-windows
Pop-Location

# Create build directory
$BuildDir = Join-Path $PSScriptRoot "build"
if (Test-Path $BuildDir) {
    Write-Host "Removing existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host ""
Write-Host "Creating build directory..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

Write-Host ""
Write-Host "Configuring with CMake..." -ForegroundColor Cyan
Push-Location $BuildDir
cmake .. `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release `
    -DVCPKG_MANIFEST_MODE=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host ""
Write-Host "Building Release..." -ForegroundColor Cyan
cmake --build . --config Release --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Don't build debug to save time
Write-Host ""
Write-Host "Skipping Debug build (only Release built)" -ForegroundColor Yellow

Pop-Location

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "  Build complete!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
Write-Host "To run the bot:" -ForegroundColor Cyan
Write-Host '  $env:DISCORD_BOT_TOKEN = "your-token"' -ForegroundColor White
Write-Host '  $env:DISCORD_GUILD_ID = "your-guild-id"' -ForegroundColor White
Write-Host '  $env:DISCORD_VOICE_CHANNEL_ID = "your-channel-id"' -ForegroundColor White
Write-Host "  .\build\Release\discord_voice_receive.exe" -ForegroundColor White
Write-Host ""
