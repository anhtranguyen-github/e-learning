#include "common/logger.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

namespace logger {

Logger* serverLogger = nullptr;
Logger* clientLogger = nullptr;
Logger* heartbeatLogger = nullptr;

Logger::Logger(const std::string& filename, LogLevel minLevel)
    : minLevel(minLevel) {
    // Create logs directory if it doesn't exist
    mkdir("logs", 0755);
    
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel) return;

    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string logEntry = "[" + getCurrentTime() + "] [" + 
                          levelToString(level) + "] " + message;
    
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush();
    }
    
    // Also print to console
    //std::cout << logEntry << std::endl;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void initServerLogger(const std::string& filename) {
    if (serverLogger == nullptr) {
        serverLogger = new Logger(filename, LogLevel::DEBUG);
    }
}

void initClientLogger(const std::string& filename) {
    if (clientLogger == nullptr) {
        clientLogger = new Logger(filename, LogLevel::DEBUG);
    }
}

void initHeartbeatLogger(const std::string& filename) {
    if (heartbeatLogger == nullptr) {
        heartbeatLogger = new Logger(filename, LogLevel::DEBUG);
    }
}

} // namespace logger
