#pragma once

#include <memory>
#include <string>

#include <fmt/format.h>

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
    ~Logger();

    class Impl;
    std::unique_ptr<Impl> pimpl_;

    void log_impl(LogLevel level, const char* file, int line, const char* function, const std::string& message);
public:
    static Logger& getInstance();

    void initialize(
        const std::string& logFile = "../build/logs/ChromaForge.log", 
        LogLevel consoleLevel = LogLevel::INFO, 
        LogLevel fileLevel = LogLevel::DEBUG
    );

    void setConsoleLevel(LogLevel level);
    void setFileLevel(LogLevel level);
    void setLoggerLevel(LogLevel level);

    template<typename... Args>
    void log(
        LogLevel level,
        const char* file,
        int line,
        const char* function,
        const char* fmt,
        const Args&... args)
    {
        log_impl(level, file, line, function, fmt::format(fmt, args...));
    }

    void flush();
};

// Макросы с автоматическим определением контекста
#define LOG_TRACE(...)    Logger::getInstance().log(LogLevel::TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getInstance().log(LogLevel::DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)     Logger::getInstance().log(LogLevel::INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)     Logger::getInstance().log(LogLevel::WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)    Logger::getInstance().log(LogLevel::ERR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().log(LogLevel::CRITICAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
