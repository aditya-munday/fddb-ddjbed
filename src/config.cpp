#include "config.h"
#include "logger.h"
#include <cstdlib>
#include <cctype>

namespace {

std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.length();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(start, end - start);
}

} // anonymous namespace

Config Config::defaults() {
    Config cfg;
    cfg.botToken = "";
    cfg.guildId = "";
    cfg.voiceChannelId = "";
    cfg.outputDirectory = "./recordings";
    cfg.logFile = "./logs/discord_voice.log";
    cfg.verbose = false;
    cfg.logLevel = 1;  // INFO
    
    // Audio settings (Discord voice defaults)
    cfg.sampleRate = 48000;
    cfg.channels = 2;
    cfg.bitsPerSample = 16;
    
    // Connection settings
    cfg.reconnectAttempts = 5;
    cfg.reconnectDelay = 5;
    
    return cfg;
}

void Config::load() {
    // Load from environment variables
    botToken = std::getenv("DISCORD_BOT_TOKEN") ? std::getenv("DISCORD_BOT_TOKEN") : "";
    guildId = std::getenv("DISCORD_GUILD_ID") ? std::getenv("DISCORD_GUILD_ID") : "";
    voiceChannelId = std::getenv("DISCORD_VOICE_CHANNEL_ID") ? std::getenv("DISCORD_VOICE_CHANNEL_ID") : "";
    
    // Optional settings with defaults
    outputDirectory = std::getenv("OUTPUT_DIRECTORY") ? std::getenv("OUTPUT_DIRECTORY") : "./recordings";
    logFile = std::getenv("LOG_FILE") ? std::getenv("LOG_FILE") : "./logs/discord_voice.log";
    
    if (std::getenv("VERBOSE")) {
        std::string v = std::getenv("VERBOSE");
        verbose = (v == "1" || v == "true" || v == "TRUE");
    }
    
    if (std::getenv("LOG_LEVEL")) {
        logLevel = std::atoi(std::getenv("LOG_LEVEL"));
    }
    
    // Audio settings
    if (std::getenv("SAMPLE_RATE")) {
        sampleRate = std::atoi(std::getenv("SAMPLE_RATE"));
    }
    if (std::getenv("CHANNELS")) {
        channels = std::atoi(std::getenv("CHANNELS"));
    }
    if (std::getenv("BITS_PER_SAMPLE")) {
        bitsPerSample = std::atoi(std::getenv("BITS_PER_SAMPLE"));
    }
    
    // Connection settings
    if (std::getenv("RECONNECT_ATTEMPTS")) {
        reconnectAttempts = std::atoi(std::getenv("RECONNECT_ATTEMPTS"));
    }
    if (std::getenv("RECONNECT_DELAY")) {
        reconnectDelay = std::atoi(std::getenv("RECONNECT_DELAY"));
    }
}

bool Config::validate() const {
    if (botToken.empty()) {
        Logger::getInstance().error("Bot token is required");
        return false;
    }
    
    if (guildId.empty()) {
        Logger::getInstance().error("Guild ID is required");
        return false;
    }
    
    if (voiceChannelId.empty()) {
        Logger::getInstance().error("Voice channel ID is required");
        return false;
    }
    
    // Validate numeric IDs
    try {
        std::stoll(guildId);
        std::stoll(voiceChannelId);
    } catch (...) {
        Logger::getInstance().error("Invalid guild or channel ID format");
        return false;
    }
    
    // Validate audio settings
    if (sampleRate <= 0 || sampleRate > 192000) {
        Logger::getInstance().error("Invalid sample rate");
        return false;
    }
    
    if (channels < 1 || channels > 8) {
        Logger::getInstance().error("Invalid channel count");
        return false;
    }
    
    if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32) {
        Logger::getInstance().error("Invalid bits per sample (must be 16, 24, or 32)");
        return false;
    }
    
    return true;
}

void printConfig(const Config& config) {
    Logger& logger = Logger::getInstance();
    
    logger.info("=== Configuration ===");
    logger.infof("Bot Token: %s", config.botToken.empty() ? "(not set)" : "***");
    logger.infof("Guild ID: %s", config.guildId.c_str());
    logger.infof("Voice Channel ID: %s", config.voiceChannelId.c_str());
    logger.infof("Output Directory: %s", config.outputDirectory.c_str());
    logger.infof("Log File: %s", config.logFile.c_str());
    logger.infof("Verbose: %s", config.verbose ? "true" : "false");
    logger.infof("Log Level: %d", config.logLevel);
    logger.info("=== Audio Settings ===");
    logger.infof("Sample Rate: %d Hz", config.sampleRate);
    logger.infof("Channels: %d", config.channels);
    logger.infof("Bits Per Sample: %d", config.bitsPerSample);
    logger.info("=== Connection Settings ===");
    logger.infof("Reconnect Attempts: %d", config.reconnectAttempts);
    logger.infof("Reconnect Delay: %d seconds", config.reconnectDelay);
    logger.info("=====================");
}
