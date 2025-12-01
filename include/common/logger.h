#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace logger {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel minLevel;

    std::string getCurrentTime();
    std::string levelToString(LogLevel level);

public:
    Logger(const std::string& filename, LogLevel minLevel = LogLevel::INFO);
    ~Logger();

    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void logMessage(const std::string& sender, const std::string& message);
};

// Global logger instances
extern Logger* serverLogger;
extern Logger* clientLogger;
extern Logger* heartbeatLogger;
extern Logger* messageLogger;

// Initialize loggers
void initServerLogger(const std::string& filename = "logs/server.log");
void initClientLogger(const std::string& filename = "logs/client.log");
void initHeartbeatLogger(const std::string& filename = "logs/heartbeat.log");
void initMessageLogger(const std::string& filename = "logs/msg.log");

} // namespace logger

#endif // LOGGER_H
