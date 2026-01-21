#include "Logger.h"

#include <iostream>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

Logger::Logger() : logger_(nullptr) {
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& name, const std::string& logFile, LogLevel consoleLevel, LogLevel fileLevel) {
    try {
        // Создаем сенки (выходы) для логов
        #ifdef _WIN32
        // Используем wincolor_sink для Windows
        auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
        #else
        // Используем stdout_color_sink для других ОС
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        #endif
        console_sink->set_level(toSpdlogLevel(consoleLevel));
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] %v");
        
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
        file_sink->set_level(toSpdlogLevel(fileLevel));
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] [%!] %v");
        
        // Создаем логгер с несколькими сенками
        logger_ = std::make_shared<spdlog::logger>(name, 
            spdlog::sinks_init_list{console_sink, file_sink});
        
        // Устанавливаем уровень логирования (берем минимальный из сенков)
        logger_->set_level(spdlog::level::trace);
        logger_->flush_on(spdlog::level::warn);
        
        spdlog::register_logger(logger_);
        
        LOG_INFO("Logger initialized. File: {}", logFile);
    }
    catch (const spdlog::spdlog_ex& e) {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        throw;
    }
}

spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:    return spdlog::level::trace;
        case LogLevel::DEBUG:    return spdlog::level::debug;
        case LogLevel::INFO:     return spdlog::level::info;
        case LogLevel::WARN:     return spdlog::level::warn;
        case LogLevel::ERR:      return spdlog::level::err;
        case LogLevel::CRITICAL: return spdlog::level::critical;
        case LogLevel::OFF:      return spdlog::level::off;
        default:                 return spdlog::level::info;
    }
}

void Logger::setLevel(LogLevel level) {
    logger_->set_level(toSpdlogLevel(level));
}
