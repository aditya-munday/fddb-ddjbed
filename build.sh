#!/bin/bash
# Build script for Discord Voice Receive Prototype

set -e

echo "=========================================="
echo "  Discord Voice Receive - Build Script"
echo "=========================================="

# Check for required dependencies
check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed."
        MISSING_DEPS=true
    fi
}

MISSING_DEPS=false
check_dependency cmake
check_dependency pkg-config
check_dependency git

# Check for libraries
check_lib() {
    if ! pkg-config --exists "$1" 2>/dev/null; then
        echo "Error: $1 is not installed."
        MISSING_DEPS=true
    fi
}

check_lib opus
check_lib opusfile
check_lib openssl

if [ "$MISSING_DEPS" = true ]; then
    echo ""
    echo "Please install the missing dependencies:"
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y \\"
    echo "    build-essential cmake pkg-config git \\"
    echo "    libssl-dev zlib1g-dev \\"
    echo "    libopus-dev libopusfile-dev"
    echo ""
    exit 1
fi

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo ""
echo "Creating build directory..."
mkdir -p "$BUILD_DIR"

echo ""
echo "Configuring with CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ""
echo "Building..."
cmake --build . --parallel $(nproc)

echo ""
echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "To run the bot:"
echo "  export DISCORD_BOT_TOKEN='your-token'"
echo "  export DISCORD_GUILD_ID='your-guild-id'"
echo "  export DISCORD_VOICE_CHANNEL_ID='your-channel-id'"
echo "  ./discord_voice_receive"
echo ""
