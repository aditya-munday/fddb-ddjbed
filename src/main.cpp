#include <dpp/dpp.h>
#include <opus/opus.h>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <map>

#include "config.h"
#include "logger.h"
#include "wav_writer.h"
#include "stats.h"

// Global state
std::atomic<bool> g_shutdown{false};
std::unique_ptr<WavWriter> g_wavWriter;
std::mutex g_wavMutex;
std::mutex g_decoderMutex;
std::map<uint32_t, OpusDecoder*> g_decoders;
std::mutex g_decodersMutex;
std::atomic<bool> g_recording{false};
std::atomic<uint64_t> g_recordingId{0};
Stats g_stats;
Config g_config;

// Signal handler
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        Logger::getInstance().info("Shutdown signal received, cleaning up...");
        g_shutdown = true;
    }
}

/**
 * @brief Get or create an Opus decoder for a user
 */
OpusDecoder* getDecoder(int sampleRate = 48000, int channels = 2) {
    uint32_t key = static_cast<uint32_t>(sampleRate * 100 + channels);
    
    std::lock_guard<std::mutex> lock(g_decodersMutex);
    
    auto it = g_decoders.find(key);
    if (it != g_decoders.end()) {
        return it->second;
    }
    
    int error;
    OpusDecoder* decoder = opus_decoder_create(sampleRate, channels, &error);
    if (error != OPUS_OK) {
        Logger::getInstance().errorf("Failed to create Opus decoder: %s", opus_strerror(error));
        return nullptr;
    }
    
    g_decoders[key] = decoder;
    Logger::getInstance().infof("Created new Opus decoder (sampleRate=%d, channels=%d)", sampleRate, channels);
    return decoder;
}

/**
 * @brief Initialize WAV file for recording
 */
bool startRecording() {
    std::lock_guard<std::mutex> lock(g_wavMutex);
    
    if (g_wavWriter) {
        // Finalize existing recording
        g_wavWriter->finalize();
        g_stats.wavBytesWritten += g_wavWriter->getBytesWritten();
    }
    
    // Generate filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    char filename[256];
    snprintf(filename, sizeof(filename), 
              "%s/recording_%ld_%04llu.wav",
              g_config.outputDirectory.c_str(),
              time,
              static_cast<unsigned long long>(++g_recordingId));
    
    g_wavWriter = std::make_unique<WavWriter>(
        filename,
        g_config.sampleRate,
        g_config.channels,
        g_config.bitsPerSample
    );
    
    if (!g_wavWriter->isOpen()) {
        Logger::getInstance().error("Failed to create WAV file");
        return false;
    }
    
    g_recording = true;
    g_stats.startRecording();
    
    Logger::getInstance().infof("Started recording to: %s", filename);
    return true;
}

/**
 * @brief Stop and finalize current recording
 */
void stopRecording() {
    std::lock_guard<std::mutex> lock(g_wavMutex);
    
    g_recording = false;
    g_stats.stopRecording();
    
    if (g_wavWriter) {
        g_wavWriter->finalize();
        g_stats.wavBytesWritten += g_wavWriter->getBytesWritten();
        Logger::getInstance().infof("Recording finalized: %llu bytes written", 
                                    static_cast<unsigned long long>(g_wavWriter->getBytesWritten()));
    }
}

/**
 * @brief Process received voice packet
 */
void processVoicePacket(const std::string& opusData, uint32_t userId) {
    if (opusData.empty()) {
        return;
    }
    
    g_stats.recordPacket(opusData.size());
    
    // Get decoder for this user
    OpusDecoder* decoder = getDecoder();
    if (!decoder) {
        g_stats.opusErrors++;
        return;
    }
    
    // Calculate output buffer size
    // Discord sends 20ms frames at 48kHz stereo = 1920 samples * 2 channels * 2 bytes
    const int maxFrameSize = 4800;  // Max samples per frame
    std::vector<opus_int16> pcmBuffer(maxFrameSize * 2);  // Stereo
    
    int frameSize = opus_decode(
        decoder,
        reinterpret_cast<const unsigned char*>(opusData.data()),
        opusData.size(),
        pcmBuffer.data(),
        maxFrameSize,
        0  // No FEC
    );
    
    if (frameSize < 0) {
        g_stats.opusErrors++;
        Logger::getInstance().warningf("Opus decode error for user %u: %s", 
                                        userId, opus_strerror(frameSize));
        return;
    }
    
    g_stats.opusDecodes++;
    
    if (g_recording) {
        std::lock_guard<std::mutex> lock(g_wavMutex);
        if (g_wavWriter && g_wavWriter->isOpen()) {
            // Write interleaved stereo PCM
            g_wavWriter->write(reinterpret_cast<const uint8_t*>(pcmBuffer.data()), 
                              frameSize * 2 * sizeof(opus_int16));
        }
    }
}

/**
 * @brief Main bot class for Discord voice receive
 */
class DiscordVoiceBot {
private:
    dpp::cluster bot;
    dpp::snowflake guildId;
    dpp::snowflake voiceChannelId;
    bool voiceConnected;
    std::atomic<bool> joining{false};

public:
    DiscordVoiceBot(const std::string& token, const std::string& guildIdStr, 
                     const std::string& voiceChannelIdStr)
        : bot(token, dpp::i_default_intents | dpp::i_message_content)
        , voiceConnected(false)
    {
        guildId = dpp::snowflake(guildIdStr);
        voiceChannelId = dpp::snowflake(voiceChannelIdStr);
        
        setupEvents();
    }
    
    void setupEvents() {
        // Log when bot is ready
        bot.on_ready([this](const dpp::ready_t& event) {
            (void)event;  // Unused
            Logger::getInstance().info("Bot logged in successfully");
            Logger::getInstance().infof("Bot username: %s", bot.me.username.c_str());
            Logger::getInstance().infof("Bot ID: %llu", static_cast<unsigned long long>(bot.me.id));
            
            g_stats.start();
            g_stats.recordConnection();
            
            // Join voice channel
            joinVoiceChannel();
        });
        
        // Handle voice state updates
        bot.on_voice_state_update([this](const dpp::voice_state_update_t& event) {
            (void)this;
            Logger::getInstance().debugf("Voice state update for guild %llu, channel %llu",
                                          static_cast<unsigned long long>(event.state.guild_id),
                                          static_cast<unsigned long long>(event.state.channel_id));
            
            // Track user joins/leaves
            if (event.state.channel_id) {
                g_stats.recordUserJoined();
            } else {
                g_stats.recordUserLeft();
            }
        });
        
        // Handle voice session start (incoming audio)
        bot.on_voice_session_start([](const dpp::voice_session_start_t& event) {
            Logger::getInstance().infof("Voice session started (ssrc: %u)", event.ssrc);
        });
        
        // Handle voice receive (individual packets)
        bot.on_voice_receive([](const dpp::voice_receive_t& event) {
            if (!event.audio || event.audio_size == 0) {
                return;
            }
            
            std::string opusData(reinterpret_cast<const char*>(event.audio), event.audio_size);
            processVoicePacket(opusData, event.user_id);
        });
        
        // Handle voice ready
        bot.on_voice_ready([this](const dpp::voice_ready_t& event) {
            (void)event;
            voiceConnected = true;
            Logger::getInstance().info("Voice connection ready - audio receiving enabled");
            
            // Start recording when voice is ready
            startRecording();
        });
        
        // Handle voice disconnect
        bot.on_voice_disconnect([this](const dpp::voice_disconnect_t& event) {
            (void)event;
            voiceConnected = false;
            Logger::getInstance().warning("Voice disconnected");
            
            // Stop recording
            stopRecording();
        });
        
        // Handle errors
        bot.on_error([](const dpp::error_t& event, const std::string& what) {
            Logger::getInstance().errorf("DPP Error: %s - %s", 
                                          dpp::error_message(event).c_str(), what.c_str());
        });
        
        // Handle log
        bot.on_log([](const dpp::log_t& event) {
            if (event.severity >= dpp::ll_error) {
                Logger::getInstance().error(event.message);
            } else if (event.severity >= dpp::ll_warning) {
                Logger::getInstance().warning(event.message);
            } else if (event.severity >= dpp::ll_info) {
                Logger::getInstance().debug(event.message);
            }
        });
    }
    
    void joinVoiceChannel() {
        if (joining.exchange(true)) {
            Logger::getInstance().warning("Already joining a voice channel");
            return;
        }
        
        Logger::getInstance().infof("Joining voice channel %llu in guild %llu",
                                    static_cast<unsigned long long>(voiceChannelId),
                                    static_cast<unsigned long long>(guildId));
        
        bot.connect_voice(guildId, voiceChannelId, false, true);
        
        // Reset joining flag after a delay
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            joining = false;
        }).detach();
    }
    
    void start() {
        Logger::getInstance().info("Starting Discord bot...");
        bot.start(dpp::st_wait);
    }
    
    void shutdown() {
        Logger::getInstance().info("Shutting down bot...");
        
        // Stop recording
        stopRecording();
        
        // Disconnect from voice
        bot.shutdown();
    }
};

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger& logger = Logger::getInstance();
    
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Load configuration
    g_config = Config::defaults();
    g_config.load();
    
    // Also support command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--token" && i + 1 < argc) {
            g_config.botToken = argv[++i];
        } else if (arg == "--guild" && i + 1 < argc) {
            g_config.guildId = argv[++i];
        } else if (arg == "--channel" && i + 1 < argc) {
            g_config.voiceChannelId = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            g_config.outputDirectory = argv[++i];
        } else if (arg == "--verbose") {
            g_config.verbose = true;
        } else if (arg == "--help") {
            std::cout << "Discord Voice Receive Prototype\n"
                      << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --token <token>      Discord bot token\n"
                      << "  --guild <id>          Guild (server) ID\n"
                      << "  --channel <id>        Voice channel ID\n"
                      << "  --output <dir>        Output directory for recordings\n"
                      << "  --verbose             Enable verbose logging\n"
                      << "  --help                Show this help\n\n"
                      << "Environment variables:\n"
                      << "  DISCORD_BOT_TOKEN     Discord bot token\n"
                      << "  DISCORD_GUILD_ID      Guild (server) ID\n"
                      << "  DISCORD_VOICE_CHANNEL_ID  Voice channel ID\n"
                      << "  OUTPUT_DIRECTORY      Output directory\n"
                      << "  LOG_FILE              Log file path\n"
                      << "  LOG_LEVEL             0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL\n";
            return 0;
        }
    }
    
    // Set log level
    LogLevel logLevel = static_cast<LogLevel>(g_config.logLevel);
    logger.setLevel(logLevel);
    
    // Initialize log file
    logger.initialize(g_config.logFile, logLevel);
    
    logger.info("============================================");
    logger.info("   Discord Voice Receive Prototype v1.0");
    logger.info("============================================");
    
    // Print configuration
    printConfig(g_config);
    
    // Validate configuration
    if (!g_config.validate()) {
        logger.critical("Configuration validation failed");
        return 1;
    }
    
    // Create output directory if it doesn't exist
    std::string mkdirCmd = "mkdir -p " + g_config.outputDirectory;
    system(mkdirCmd.c_str());
    
    // Create log directory if it doesn't exist
    std::string logDir = g_config.logFile.substr(0, g_config.logFile.find_last_of('/'));
    if (!logDir.empty() && logDir != g_config.logFile) {
        mkdirCmd = "mkdir -p " + logDir;
        system(mkdirCmd.c_str());
    }
    
    logger.info("Initializing...");
    
    try {
        DiscordVoiceBot bot(g_config.botToken, g_config.guildId, g_config.voiceChannelId);
        
        // Statistics reporting thread
        std::thread statsThread([]() {
            while (!g_shutdown) {
                std::this_thread::sleep_for(std::chrono::seconds(60));
                if (!g_shutdown) {
                    g_stats.print();
                }
            }
        });
        statsThread.detach();
        
        // Main bot loop
        bot.start();
        
    } catch (const std::exception& e) {
        logger.criticalf("Fatal error: %s", e.what());
        return 1;
    }
    
    // Cleanup
    logger.info("Final cleanup...");
    
    // Clean up decoders
    {
        std::lock_guard<std::mutex> lock(g_decodersMutex);
        for (auto& [key, decoder] : g_decoders) {
            opus_decoder_destroy(decoder);
        }
        g_decoders.clear();
    }
    
    // Print final statistics
    g_stats.print();
    
    logger.info("Shutdown complete");
    logger.flush();
    
    return 0;
}
