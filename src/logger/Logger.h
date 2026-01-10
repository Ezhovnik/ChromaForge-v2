#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

enum class LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERR,
    CRITICAL,
    OFF
};

class Logger {
private:
    Logger();
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> logger_;

    static spdlog::level::level_enum toSpdlogLevel(LogLevel level);
public:
    static Logger& getInstance();

    void initialize(const std::string& name = "ChromaForge", const std::string& logFile = "../logs/ChromaForge.log", LogLevel consoleLevel = LogLevel::INFO, LogLevel fileLevel = LogLevel::DEBUG);

    template<typename... Args>
    void trace(const char* fmt, const Args&... args) {
        logger_->trace(fmt, args...);
    }
    
    template<typename... Args>
    void debug(const char* fmt, const Args&... args) {
        logger_->debug(fmt, args...);
    }
    
    template<typename... Args>
    void info(const char* fmt, const Args&... args) {
        logger_->info(fmt, args...);
    }
    
    template<typename... Args>
    void warn(const char* fmt, const Args&... args) {
        logger_->warn(fmt, args...);
    }
    
    template<typename... Args>
    void error(const char* fmt, const Args&... args) {
        logger_->error(fmt, args...);
    }
    
    template<typename... Args>
    void critical(const char* fmt, const Args&... args) {
        logger_->critical(fmt, args...);
    }
    
    void setLevel(LogLevel level);
    void flush() { logger_->flush(); }
};

#define LOG_TRACE(...)    Logger::getInstance().trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().critical(__VA_ARGS__)

#endif // LOGGER_LOGGER_H_
