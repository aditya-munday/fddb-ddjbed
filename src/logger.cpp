#include "logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <mutex>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& filename, LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_level = level;
    
    if (!filename.empty()) {
        m_file.open(filename, std::ios::app);
        if (m_file.is_open()) {
            m_fileOutput = true;
        }
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_level) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << "[" << getTimestamp() << "] "
       << "[" << levelToString(level) << "] "
       << message;

    std::string fullMessage = ss.str();
    
    // Console output
    std::cout << fullMessage << std::endl;

    // File output
    if (m_fileOutput && m_file.is_open()) {
        writeToFile(fullMessage);
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file.flush();
    }
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO ";
        case LogLevel::WARNING:  return "WARN ";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRIT ";
        default:                return "UNK  ";
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::writeToFile(const std::string& message) {
    m_file << message << std::endl;
    m_file.flush();
}
