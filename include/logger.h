#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <mutex>

/**
 * @brief Logging levels for the application
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Thread-safe logger for the Discord voice receive prototype
 * 
 * Outputs logs to both console and file with timestamps and log levels.
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger
     * @param filename Log file path (empty for console only)
     * @param level Minimum log level to output
     */
    void initialize(const std::string& filename = "", LogLevel level = LogLevel::INFO);

    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     */
    void setLevel(LogLevel level);

    /**
     * @brief Log a message
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Log with format-style arguments
     * @param level Log level
     * @param format Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void logf(LogLevel level, const char* format, Args&&... args);

    // Convenience methods
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

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

    /**
     * @brief Flush all pending log entries
     */
    void flush();

    /**
     * @brief Get the log level as a string
     * @param level Log level
     * @return String representation
     */
    static const char* levelToString(LogLevel level);

private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getTimestamp();
    void writeToFile(const std::string& message);

    std::ofstream m_file;
    std::mutex m_mutex;
    LogLevel m_level = LogLevel::INFO;
    bool m_fileOutput = false;
};

// Template implementation for format-style logging
template<typename... Args>
void Logger::logf(LogLevel level, const char* format, Args&&... args) {
    // Simple format implementation using snprintf
    char buffer[4096];
    int len = snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
    if (len > 0 && len < static_cast<int>(sizeof(buffer))) {
        log(level, std::string(buffer, static_cast<size_t>(len)));
    }
}

template<typename... Args>
void Logger::debugf(const char* format, Args&&... args) {
    logf(LogLevel::DEBUG, format, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::infof(const char* format, Args&&... args) {
    logf(LogLevel::INFO, format, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::warningf(const char* format, Args&&... args) {
    logf(LogLevel::WARNING, format, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::errorf(const char* format, Args&&... args) {
    logf(LogLevel::ERROR, format, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::criticalf(const char* format, Args&&... args) {
    logf(LogLevel::CRITICAL, format, std::forward<Args>(args)...);
}
