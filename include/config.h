#pragma once

#include <string>

/**
 * @brief Configuration for the Discord Voice Receive application
 */
struct Config {
    std::string botToken;          // Discord bot token
    std::string guildId;            // Discord guild (server) ID
    std::string voiceChannelId;     // Voice channel to join
    std::string outputDirectory;    // Directory for WAV recordings
    std::string logFile;            // Log file path
    bool verbose;                  // Enable verbose logging
    int logLevel;                   // 0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL

    // Audio settings
    int sampleRate;                 // Sample rate in Hz
    int channels;                   // Number of channels
    int bitsPerSample;              // Bits per sample

    // Connection settings
    int reconnectAttempts;          // Max reconnection attempts
    int reconnectDelay;             // Delay between reconnect attempts (seconds)

    /**
     * @brief Load configuration from environment variables or defaults
     */
    void load();

    /**
     * @brief Validate the configuration
     * @return true if valid, false otherwise
     */
    bool validate() const;

    /**
     * @brief Get default configuration
     * @return Config with default values
     */
    static Config defaults();
};

/**
 * @brief Print configuration to logger
 * @param config Configuration to print
 */
void printConfig(const Config& config);
