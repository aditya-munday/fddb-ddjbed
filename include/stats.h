#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>
#include <string>

/**
 * @brief Performance statistics tracker for the voice receive prototype
 */
class Stats {
public:
    Stats();

    // Counters (atomic for thread safety)
    std::atomic<uint64_t> packetsReceived;
    std::atomic<uint64_t> packetsLost;
    std::atomic<uint64_t> opusDecodes;
    std::atomic<uint64_t> opusErrors;
    std::atomic<uint64_t> pcmBytesReceived;
    std::atomic<uint64_t> wavBytesWritten;
    std::atomic<uint64_t> usersJoined;
    std::atomic<uint64_t> usersLeft;

    // Timestamps
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point connectionTime;
    std::chrono::steady_clock::time_point recordingStartTime;
    std::chrono::steady_clock::time_point lastPacketTime;

    // Recording state
    std::atomic<bool> isRecording;
    std::atomic<bool> isConnected;
    std::atomic<bool> isRunning;

    // User tracking
    std::atomic<uint32_t> activeUsers;

    /**
     * @brief Start tracking statistics
     */
    void start();

    /**
     * @brief Record connection established
     */
    void recordConnection();

    /**
     * @brief Start recording
     */
    void startRecording();

    /**
     * @brief Stop recording
     */
    void stopRecording();

    /**
     * @brief Record a packet received
     */
    void recordPacket(uint64_t bytes);

    /**
     * @brief Record user joined
     */
    void recordUserJoined();

    /**
     * @brief Record user left
     */
    void recordUserLeft();

    /**
     * @brief Get uptime in seconds
     * @return Seconds since start
     */
    double getUptime() const;

    /**
     * @brief Get recording duration in seconds
     * @return Seconds since recording started
     */
    double getRecordingDuration() const;

    /**
     * @brief Get packet loss percentage
     * @return Packet loss as percentage (0-100)
     */
    double getPacketLossPercent() const;

    /**
     * @brief Get throughput in bytes per second
     * @return Bytes per second
     */
    double getThroughput() const;

    /**
     * @brief Generate a summary report
     * @return Formatted statistics report
     */
    std::string generateReport() const;

    /**
     * @brief Print current statistics
     */
    void print() const;
};
