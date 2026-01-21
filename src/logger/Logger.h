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

    void initialize(const std::string& name = "ChromaForge", 
                   const std::string& logFile = "../logs/ChromaForge.log", 
                   LogLevel consoleLevel = LogLevel::INFO, 
                   LogLevel fileLevel = LogLevel::DEBUG);

    // Методы для ручного вызова (без автоматического контекста)
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

#define LOG_TRACE_NC(...)    Logger::getInstance().trace(__VA_ARGS__)
#define LOG_DEBUG_NC(...)    Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO_NC(...)     Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN_NC(...)     Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR_NC(...)    Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL_NC(...) Logger::getInstance().critical(__VA_ARGS__)

// Макросы с автоматическим определением контекста
#define LOG_TRACE(...)    Logger::getInstance().trace_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getInstance().debug_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)     Logger::getInstance().info_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)     Logger::getInstance().warn_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)    Logger::getInstance().error_context(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().critical_context(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif // LOGGER_LOGGER_H_
