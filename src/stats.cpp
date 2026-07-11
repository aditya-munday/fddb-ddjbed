#include "stats.h"
#include "logger.h"
#include <iomanip>
#include <sstream>

Stats::Stats()
    : packetsReceived(0)
    , packetsLost(0)
    , opusDecodes(0)
    , opusErrors(0)
    , pcmBytesReceived(0)
    , wavBytesWritten(0)
    , usersJoined(0)
    , usersLeft(0)
    , isRecording(false)
    , isConnected(false)
    , isRunning(false)
    , activeUsers(0)
{
}

void Stats::start() {
    startTime = std::chrono::steady_clock::now();
    isRunning = true;
}

void Stats::recordConnection() {
    connectionTime = std::chrono::steady_clock::now();
    isConnected = true;
    Logger::getInstance().info("Connection established");
}

void Stats::startRecording() {
    recordingStartTime = std::chrono::steady_clock::now();
    isRecording = true;
    Logger::getInstance().info("Recording started");
}

void Stats::stopRecording() {
    isRecording = false;
    Logger::getInstance().info("Recording stopped");
}

void Stats::recordPacket(uint64_t bytes) {
    packetsReceived++;
    pcmBytesReceived += bytes;
    lastPacketTime = std::chrono::steady_clock::now();
}

void Stats::recordUserJoined() {
    usersJoined++;
    activeUsers++;
}

void Stats::recordUserLeft() {
    usersLeft++;
    if (activeUsers > 0) {
        activeUsers--;
    }
}

double Stats::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count() / 1000.0;
}

double Stats::getRecordingDuration() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - recordingStartTime);
    return duration.count() / 1000.0;
}

double Stats::getPacketLossPercent() const {
    uint64_t total = packetsReceived.load() + packetsLost.load();
    if (total == 0) {
        return 0.0;
    }
    return (static_cast<double>(packetsLost.load()) / total) * 100.0;
}

double Stats::getThroughput() const {
    double uptime = getUptime();
    if (uptime <= 0.0) {
        return 0.0;
    }
    return static_cast<double>(pcmBytesReceived.load()) / uptime;
}

std::string Stats::generateReport() const {
    std::stringstream ss;
    
    ss << std::fixed << std::setprecision(2);
    
    ss << "\n";
    ss << "==============================================\n";
    ss << "       DISCORD VOICE RECEIVE STATISTICS       \n";
    ss << "==============================================\n";
    
    ss << "Runtime Statistics:\n";
    ss << "  Uptime:              " << getUptime() << " seconds\n";
    ss << "  Recording Duration:  " << getRecordingDuration() << " seconds\n";
    
    ss << "\nConnection Statistics:\n";
    ss << "  Connected:           " << (isConnected ? "Yes" : "No") << "\n";
    ss << "  Recording:           " << (isRecording ? "Yes" : "No") << "\n";
    ss << "  Active Users:        " << activeUsers.load() << "\n";
    ss << "  Total Users Joined:  " << usersJoined.load() << "\n";
    ss << "  Total Users Left:    " << usersLeft.load() << "\n";
    
    ss << "\nPacket Statistics:\n";
    ss << "  Packets Received:    " << packetsReceived.load() << "\n";
    ss << "  Packets Lost:        " << packetsLost.load() << "\n";
    ss << "  Packet Loss:         " << getPacketLossPercent() << "%\n";
    
    ss << "\nAudio Statistics:\n";
    ss << "  PCM Bytes Received:  " << pcmBytesReceived.load() << "\n";
    ss << "  WAV Bytes Written:    " << wavBytesWritten.load() << "\n";
    ss << "  Opus Decodes:        " << opusDecodes.load() << "\n";
    ss << "  Opus Errors:         " << opusErrors.load() << "\n";
    ss << "  Throughput:          " << getThroughput() << " bytes/sec\n";
    
    ss << "==============================================\n";
    
    return ss.str();
}

void Stats::print() const {
    Logger::getInstance().info(generateReport());
}
