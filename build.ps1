#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"

Write-Host "Building..." -ForegroundColor Cyan

$BuildDir = "$PSScriptRoot\build"
Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $BuildDir | Out-Null

Push-Location $BuildDir
try {
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release --parallel
} finally {
    Pop-Location
}

Write-Host "Done!" -ForegroundColor Green
