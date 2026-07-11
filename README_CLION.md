# Building in CLion

This guide covers building the Discord Voice Receive Prototype in JetBrains CLion.

## Prerequisites

1. **CLion 2022.3+** with C++20 support
2. **vcpkg** installed (e.g., `C:\vcpkg`)
3. **Visual Studio 2022** with C++ toolset (for Windows)

## Setup

### 1. Open Project in CLion

1. Open CLion
2. File → Open → Select the project folder
3. Click "Trust Project"

### 2. Configure vcpkg Toolchain (Windows)

CLion should auto-detect vcpkg if `VCPKG_ROOT` is set. If not:

1. File → Settings → Build, Execution, Deployment → CMake
2. Under "CMake Options", add:
   ```
   -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```
3. Or use CMake Presets (recommended - see below)

### 3. CMake Profiles

The project includes `CMakePresets.json` with pre-configured profiles:

| Preset | Description |
|--------|-------------|
| `debug` | Debug build with symbols |
| `release` | Release build with optimizations |

Select a preset from:
- CLion toolbar: Build Type dropdown
- Or: File → Settings → Build, Execution, Deployment → CMake → Profile

### 4. Build

1. Select "discord_voice_receive" as the target
2. Press **Ctrl+Shift+F9** to build
3. Or use: Build → Build Project

### 5. Run Configuration

Create a run configuration:

1. **Run → Edit Configurations...**
2. Click **+** → CMake Application
3. Configure:
   - **Name**: Discord Voice Receive
   - **Target**: discord_voice_receive
   - **Executable**: Leave as default (will auto-detect)
   - **Program arguments**: (optional) `--help`
   - **Environment variables**:
     ```
     DISCORD_BOT_TOKEN=your-bot-token
     DISCORD_GUILD_ID=your-guild-id
     DISCORD_VOICE_CHANNEL_ID=your-channel-id
     ```

## Troubleshooting

### "No CMake profile selected"

1. File → Settings → Build, Execution, Deployment → CMake
2. Click **+** to add a profile
3. Select "release" preset
4. Click OK

### "Cannot find vcpkg packages"

1. Ensure vcpkg is bootstrapped:
   ```cmd
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   ```
2. Ensure packages are installed:
   ```cmd
   .\vcpkg install opus:x64-windows openssl:x64-windows zlib:x64-windows
   ```

### "Compiler not found"

Ensure Visual Studio 2022 is installed with:
- MSVC v143 toolset
- Windows 11 SDK

### Indexing Issues

If CLion shows errors but the project builds:

1. File → Invalidate Caches → Invalidate and Restart
2. Wait for re-indexing to complete

## Project Structure in CLion

```
discord-voice-receive/
├── CMakeLists.txt          # Main CMake config
├── CMakePresets.json       # Build presets
├── .clangd                 # clangd configuration
├── .idea/                  # CLion settings (don't edit)
├── include/                # Header files
│   ├── config.h
│   ├── logger.h
│   ├── stats.h
│   └── wav_writer.h
├── src/                    # Source files
│   ├── main.cpp
│   ├── config.cpp
│   ├── logger.cpp
│   ├── stats.cpp
│   └── wav_writer.cpp
├── recordings/             # Output audio files
└── logs/                   # Log files
```

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Build | Ctrl+Shift+F9 |
| Run | Shift+F10 |
| Debug | Shift+F9 |
| CMake Reload | Ctrl+Shift+O |
| Build All | Ctrl+F9 |
