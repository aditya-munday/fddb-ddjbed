# Discord Voice Receive Prototype

A standalone C++ prototype using the DPP (D++) library that validates reliable Discord voice reception. This prototype serves as the foundation for future real-time voice processing applications.

## Features

- **Discord Bot Integration**: Authenticates using a bot token and joins voice channels
- **Voice Packet Reception**: Receives and processes voice packets from Discord users
- **Opus Decoding**: Decodes incoming Opus packets into raw PCM audio
- **WAV Recording**: Continuously records audio to valid WAV files
- **Multi-user Support**: Handles simultaneous audio from multiple users
- **Logging**: Comprehensive logging with timestamps and severity levels
- **Performance Statistics**: Tracks packets, bytes, decodes, and recording duration

## Requirements

### System Dependencies

- C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.16+
- OpenSSL development libraries
- zlib development libraries
- Opus library (libopus)
- opusfile library (libopusfile)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    zlib1g-dev \
    libopus-dev \
    libopusfile-dev \
    pkg-config
```

**Fedora/RHEL:**
```bash
sudo dnf install \
    gcc-c++ \
    cmake \
    openssl-devel \
    zlib-devel \
    opus-devel \
    opusfile-devel \
    pkg-config
```

**macOS:**
```bash
brew install opus opusfile pkg-config
brew install openssl
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --parallel

# The binary will be at: build/discord_voice_receive
```

## Configuration

### Environment Variables

| Variable | Description | Required |
|----------|-------------|----------|
| `DISCORD_BOT_TOKEN` | Discord bot token | Yes |
| `DISCORD_GUILD_ID` | Guild (server) ID | Yes |
| `DISCORD_VOICE_CHANNEL_ID` | Voice channel ID | Yes |
| `OUTPUT_DIRECTORY` | Recording output directory | No (default: ./recordings) |
| `LOG_FILE` | Log file path | No (default: ./logs/discord_voice.log) |
| `LOG_LEVEL` | Log level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL) | No (default: 1) |
| `SAMPLE_RATE` | Audio sample rate | No (default: 48000) |
| `CHANNELS` | Audio channels | No (default: 2) |
| `BITS_PER_SAMPLE` | Audio bit depth | No (default: 16) |
| `RECONNECT_ATTEMPTS` | Max reconnection attempts | No (default: 5) |
| `VERBOSE` | Enable verbose logging | No |

### Command-Line Arguments

```
--token <token>      Discord bot token
--guild <id>         Guild (server) ID
--channel <id>       Voice channel ID
--output <dir>       Output directory for recordings
--verbose            Enable verbose logging
--help               Show help message
```

## Usage

### Basic Usage

```bash
# Set environment variables
export DISCORD_BOT_TOKEN="your-bot-token"
export DISCORD_GUILD_ID="123456789"
export DISCORD_VOICE_CHANNEL_ID="987654321"

# Run the bot
./discord_voice_receive
```

### Using Command-Line Arguments

```bash
./discord_voice_receive \
    --token "your-bot-token" \
    --guild "123456789" \
    --channel "987654321" \
    --output "./my_recordings"
```

## Bot Setup

1. Go to the [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application
3. Go to the "Bot" section
4. Click "Reset Token" to get your bot token
5. Enable "Message Content Intent" in the Bot settings
6. Generate an invite link with these permissions:
   - `CONNECT` - Voice channel access
   - `SPEAK` - Voice channel speaking
7. Invite the bot to your server

## Audio Format

- **Sample Rate**: 48,000 Hz
- **Bit Depth**: 16-bit
- **Channels**: Stereo
- **Encoding**: PCM (WAV)

## Output Files

Recordings are saved to the output directory with the naming pattern:
```
recording_<timestamp>_<recording_id>.wav
```

For example: `recording_1699900000_0001.wav`

## Logging

Logs are written to both console and file with the following format:
```
[2024-11-13 12:00:00.123] [INFO ] Bot logged in successfully
[2024-11-13 12:00:01.456] [INFO ] Voice connection ready
[2024-11-13 12:00:05.789] [INFO ] Recording started
```

## Performance Statistics

The application tracks and reports:
- **Runtime**: Uptime and recording duration
- **Connection**: Connected users, joins/leaves
- **Packets**: Received, lost, loss percentage
- **Audio**: PCM bytes, WAV bytes, Opus decodes/errors, throughput

Statistics are printed every 60 seconds and on shutdown.

## Error Handling

The application handles:
- Invalid bot token
- Missing permissions
- Voice connection failures
- File write failures
- Unexpected Discord disconnects
- Decoder failures

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    main.cpp                          │
│  ┌─────────────┐  ┌─────────────┐  ┌────────────┐ │
│  │ Config      │  │ Logger      │  │ Stats      │ │
│  └─────────────┘  └─────────────┘  └────────────┘ │
└─────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────┐
│              DiscordVoiceBot (DPP)                   │
│  ┌─────────────────────────────────────────────────┐ │
│  │ Voice Events: ready, disconnect, receive        │ │
│  └─────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
          │                    │
          ▼                    ▼
┌─────────────────┐  ┌─────────────────┐
│ Opus Decoder    │  │ WAV Writer      │
│ (libopus)       │  │                 │
└─────────────────┘  └─────────────────┘
```

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please read the contribution guidelines before submitting PRs.
