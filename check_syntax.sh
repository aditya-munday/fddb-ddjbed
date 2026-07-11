#!/bin/bash
# Syntax check script - verifies C++ code without full compilation

set -e

echo "=========================================="
echo "  Discord Voice Receive - Syntax Check"
echo "=========================================="

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Create mock headers for syntax checking
MOCK_DIR="/tmp/mock_headers_$$"
mkdir -p "$MOCK_DIR"

# Create mock DPP header
cat > "$MOCK_DIR/dpp.h" << 'DPP_MOCK'
#pragma once
#include <string>
#include <functional>
#include <cstdint>

namespace dpp {
    using snowflake = uint64_t;
    
    enum class log_level { ll_debug, ll_info, ll_warning, ll_error, ll_critical };
    enum class intent : int { i_default_intents = 0, i_message_content = 1 };
    enum class start_type { st_wait };
    enum class error_t { et_unknown };
    
    struct ready_t { std::string version; snowflake me_id; };
    struct log_t { std::string message; log_level severity; };
    struct error_t_event { error_t error; std::string what; };
    struct voice_state_update_t { snowflake guild_id; snowflake channel_id; snowflake user_id; struct { snowflake guild_id; snowflake channel_id; } state; };
    struct voice_ready_t { snowflake voice_channel_id; };
    struct voice_disconnect_t { snowflake voice_channel_id; };
    struct voice_session_start_t { uint32_t ssrc; };
    struct voice_receive_t { const uint8_t* audio; size_t audio_size; uint32_t user_id; uint32_t ssrc; };
    
    inline std::string error_message(error_t) { return "error"; }
    
    struct me_t {
        std::string username;
        snowflake id;
    };
    
    struct cluster {
        me_t me;
        template<typename T> void on_ready(T&&) {}
        template<typename T> void on_log(T&&) {}
        template<typename T> void on_error(T&&) {}
        template<typename T> void on_voice_state_update(T&&) {}
        template<typename T> void on_voice_ready(T&&) {}
        template<typename T> void on_voice_disconnect(T&&) {}
        template<typename T> void on_voice_session_start(T&&) {}
        template<typename T> void on_voice_receive(T&&) {}
        void start(start_type) {}
        void shutdown() {}
        void connect_voice(snowflake, snowflake, bool, bool) {}
    };
}
DPP_MOCK

# Create mock Opus header
cat > "$MOCK_DIR/opus.h" << 'OPUS_MOCK'
#pragma once
#include <cstdint>

#define OPUS_OK 0
#define OPUS_BAD_ARG -4

typedef struct OpusDecoder OpusDecoder;

OpusDecoder* opus_decoder_create(int fs, int channels, int* error);
void opus_decoder_destroy(OpusDecoder*);
const char* opus_strerror(int error);
int opus_decode(OpusDecoder*, const unsigned char*, opus_int32, opus_int16*, int, int);
typedef int32_t opus_int32;
typedef int16_t opus_int16;
OPUS_MOCK

# Create mock config, logger, wav_writer, stats headers
cp "$PROJECT_DIR/include/config.h" "$MOCK_DIR/"
cp "$PROJECT_DIR/include/logger.h" "$MOCK_DIR/"
cp "$PROJECT_DIR/include/wav_writer.h" "$MOCK_DIR/"
cp "$PROJECT_DIR/include/stats.h" "$MOCK_DIR/"

# Modify logger.h to not include <fstream> and <mutex> for simpler mock
cat > "$MOCK_DIR/logger.h" << 'LOGGER_MOCK'
#pragma once
#include <string>
#include <chrono>

enum class LogLevel {
    DEBUG, INFO, WARNING, ERROR, CRITICAL
};

class Logger {
public:
    static Logger& getInstance();
    void initialize(const std::string& filename = "", LogLevel level = LogLevel::INFO);
    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    template<typename... Args>
    void logf(LogLevel level, const char* format, Args&&... args);
    template<typename... Args>
    void debugf(const char* format, Args&&... args);
    template<typename... Args>
    void infof(const char* format, Args&&... args);
    template<typename... Args>
    void warningf(const char* format, Args&&... args);
    template<typename... Args>
    void errorf(const char* format, Args&&... args);
    template<typename... Args>
    void criticalf(const char* format, Args&&... args);
    void flush();
    static const char* levelToString(LogLevel level);
};
LOGGER_MOCK

# Create mock wav_writer.h
cat > "$MOCK_DIR/wav_writer.h" << 'WAV_MOCK'
#pragma once
#include <string>
#include <cstdint>

class WavWriter {
public:
    WavWriter(const std::string& filename, uint32_t sampleRate = 48000, uint16_t channels = 2, uint16_t bitsPerSample = 16);
    ~WavWriter();
    WavWriter(const WavWriter&) = delete;
    WavWriter& operator=(const WavWriter&) = delete;
    WavWriter(WavWriter&& other) noexcept;
    WavWriter& operator=(WavWriter&& other) noexcept;
    bool write(const uint8_t* data, size_t size);
    bool writeStereo(const int16_t* leftChannel, const int16_t* rightChannel, size_t frameCount);
    bool finalize();
    bool isOpen() const;
    uint64_t getBytesWritten() const;
    double getDuration() const;
};
WAV_MOCK

# Create mock stats.h
cat > "$MOCK_DIR/stats.h" << 'STATS_MOCK'
#pragma once
#include <atomic>
#include <cstdint>
#include <chrono>
#include <string>

class Stats {
public:
    Stats();
    std::atomic<uint64_t> packetsReceived;
    std::atomic<uint64_t> packetsLost;
    std::atomic<uint64_t> opusDecodes;
    std::atomic<uint64_t> opusErrors;
    std::atomic<uint64_t> pcmBytesReceived;
    std::atomic<uint64_t> wavBytesWritten;
    std::atomic<uint64_t> usersJoined;
    std::atomic<uint64_t> usersLeft;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point connectionTime;
    std::chrono::steady_clock::time_point recordingStartTime;
    std::chrono::steady_clock::time_point lastPacketTime;
    std::atomic<bool> isRecording;
    std::atomic<bool> isConnected;
    std::atomic<bool> isRunning;
    std::atomic<uint32_t> activeUsers;
    void start();
    void recordConnection();
    void startRecording();
    void stopRecording();
    void recordPacket(uint64_t bytes);
    void recordUserJoined();
    void recordUserLeft();
    double getUptime() const;
    double getRecordingDuration() const;
    double getPacketLossPercent() const;
    double getThroughput() const;
    std::string generateReport() const;
    void print() const;
};
STATS_MOCK

# Create mock config.h
cat > "$MOCK_DIR/config.h" << 'CONFIG_MOCK'
#pragma once
#include <string>

struct Config {
    std::string botToken;
    std::string guildId;
    std::string voiceChannelId;
    std::string outputDirectory;
    std::string logFile;
    bool verbose;
    int logLevel;
    int sampleRate;
    int channels;
    int bitsPerSample;
    int reconnectAttempts;
    int reconnectDelay;
    void load();
    bool validate() const;
    static Config defaults();
};
void printConfig(const Config& config);
CONFIG_MOCK

echo ""
echo "Syntax checking source files..."

# Check each source file
cd "$PROJECT_DIR"
for src in src/*.cpp; do
    echo "  Checking: $src"
    g++ -std=c++20 -fsyntax-only -I"$MOCK_DIR" -I"$PROJECT_DIR/include" "$src" 2>&1 || {
        echo "  [FAIL] $src"
        FAILED=true
    }
done

# Clean up
rm -rf "$MOCK_DIR"

echo ""
if [ "$FAILED" = true ]; then
    echo "=========================================="
    echo "  Syntax check FAILED"
    echo "=========================================="
    exit 1
else
    echo "=========================================="
    echo "  Syntax check PASSED"
    echo "=========================================="
    echo ""
    echo "Note: This only checks syntax. Full compilation"
    echo "requires DPP, Opus, and other dependencies."
    exit 0
fi
