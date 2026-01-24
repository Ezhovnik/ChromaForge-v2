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
    
    template<typename... Args>
    void logWithContext(spdlog::level::level_enum level, 
                        const char* file, int line, const char* function,
                        const char* fmt, const Args&... args) {
        logger_->log(spdlog::source_loc{file, line, function}, level, fmt, args...);
    }
    
public:
    static Logger& getInstance();

    void initialize(const std::string& logFile, LogLevel consoleLevel, LogLevel fileLevel);
    
    // Методы с контекстом (для использования внутри макросов)
    template<typename... Args>
    void trace_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::trace, file, line, function, fmt, args...);
    }
    
    template<typename... Args>
    void debug_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::debug, file, line, function, fmt, args...);
    }
    
    template<typename... Args>
    void info_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::info, file, line, function, fmt, args...);
    }
    
    template<typename... Args>
    void warn_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::warn, file, line, function, fmt, args...);
    }
    
    template<typename... Args>
    void error_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::err, file, line, function, fmt, args...);
    }
    
    template<typename... Args>
    void critical_context(const char* file, int line, const char* function, const char* fmt, const Args&... args) {
        logWithContext(spdlog::level::critical, file, line, function, fmt, args...);
    }
    
    void setLevel(LogLevel level);
    void flush() { logger_->flush(); }
};

// Макросы с автоматическим определением контекста
#define LOG_TRACE(...)    Logger::getInstance().trace_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getInstance().debug_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)     Logger::getInstance().info_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)     Logger::getInstance().warn_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)    Logger::getInstance().error_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().critical_context(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif // LOGGER_LOGGER_H_
