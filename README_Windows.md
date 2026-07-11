# Building on Windows with MSVC and vcpkg

This guide covers building the Discord Voice Receive Prototype on Windows using Visual Studio and vcpkg.

## Prerequisites

### 1. Install Visual Studio 2022

Download and install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with:
- **Desktop development with C++** workload
- Windows 11 SDK

### 2. Clone and Bootstrap vcpkg

```powershell
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg

# Bootstrap vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

### 3. Set Environment Variables

```powershell
# Add to your PowerShell profile or set before building
$env:VCPKG_ROOT = "C:\vcpkg"
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows"
```

## Building

### Option 1: Using PowerShell Script (Recommended)

```powershell
# Set environment variables
$env:VCPKG_ROOT = "C:\vcpkg"

# Run the build script
.\build.ps1
```

### Option 2: Using Batch Script

```cmd
# In Command Prompt
set VCPKG_ROOT=C:\vcpkg
.\build.bat
```

### Option 3: Manual Build

```powershell
# Navigate to project directory
cd C:\path\to\discord-voice-receive

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release --parallel
```

## Dependencies

The following dependencies will be installed automatically by vcpkg:

| Package | Version | Purpose |
|---------|---------|---------|
| opus | latest | Opus audio codec |
| openssl | latest | SSL/TLS for Discord |
| zlib | latest | Compression |

## Running

### Set Environment Variables

```powershell
# Required
$env:DISCORD_BOT_TOKEN = "your-bot-token"
$env:DISCORD_GUILD_ID = "123456789"
$env:DISCORD_VOICE_CHANNEL_ID = "987654321"

# Optional (with defaults)
$env:OUTPUT_DIRECTORY = ".\recordings"
$env:LOG_FILE = ".\logs\discord_voice.log"
$env:LOG_LEVEL = "1"  # 0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL
```

### Run the Bot

```powershell
.\build\Release\discord_voice_receive.exe
```

Or with command-line arguments:

```powershell
.\build\Release\discord_voice_receive.exe `
    --token "your-bot-token" `
    --guild "123456789" `
    --channel "987654321" `
    --output ".\recordings"
```

## Troubleshooting

### vcpkg not found

```
Error: VCPKG_ROOT environment variable is not set.
```

**Solution:** Set the `VCPKG_ROOT` environment variable to your vcpkg installation directory.

### MSVC toolset not found

```
CMake Error: Could not find any instance of Visual Studio.
```

**Solution:** Open the build in a Developer Command Prompt or use the correct generator:
```powershell
cmake .. -G "Visual Studio 17 2022" -A x64
```

### Missing DLLs at runtime

If you get DLL errors when running, ensure the vcpkg DLLs are in your PATH:

```powershell
$env:PATH = "$env:VCPKG_ROOT\installed\x64-windows\bin;" + $env:PATH
```

Or copy the required DLLs to your output directory:
```powershell
Copy-Item "$env:VCPKG_ROOT\installed\x64-windows\bin\*.dll" -Destination ".\build\Release\"
```

### Build fails with linker errors

If you see linker errors related to library conflicts, try a clean build:

```powershell
Remove-Item -Recurse -Force .\build
.\build.ps1
```

## Project Structure

```
discord-voice-receive/
├── CMakeLists.txt          # CMake configuration
├── vcpkg.json              # vcpkg manifest
├── build.bat               # Windows batch build script
├── build.ps1               # PowerShell build script
├── include/
│   ├── config.h            # Configuration
│   ├── logger.h            # Logging
│   ├── stats.h             # Statistics
│   └── wav_writer.h        # WAV file writer
├── src/
│   ├── main.cpp            # Main application
│   ├── config.cpp          # Configuration implementation
│   ├── logger.cpp          # Logging implementation
│   ├── stats.cpp           # Statistics implementation
│   └── wav_writer.cpp      # WAV writer implementation
├── recordings/              # Output directory for recordings
├── logs/                    # Output directory for logs
└── README.md               # General documentation
```

## Next Steps

1. Follow the [main README](README.md) for usage instructions
2. Complete the [test report](TEST_REPORT.md) after testing
3. Configure your Discord bot with appropriate intents and permissions
