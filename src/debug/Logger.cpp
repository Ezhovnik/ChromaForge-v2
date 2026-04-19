#include <debug/Logger.h>

#include <iostream>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

static spdlog::level::level_enum to_spdlog_level(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::TRACE: return spdlog::level::trace;
        case LogLevel::DEBUG: return spdlog::level::debug;
        case LogLevel::INFO: return spdlog::level::info;
        case LogLevel::WARN: return spdlog::level::warn;
        case LogLevel::ERR: return spdlog::level::err;
        case LogLevel::CRITICAL: return spdlog::level::critical;
        case LogLevel::OFF: return spdlog::level::off;
        default: return spdlog::level::info;
    }
}

class Logger::Impl {
private:
    std::shared_ptr<spdlog::logger> logger_;
public:
    Impl() = default;

    void initialize(const std::string& logFile, LogLevel consoleLevel, LogLevel fileLevel) {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(to_spdlog_level(consoleLevel));
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
            file_sink->set_level(to_spdlog_level(fileLevel));
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] [%!] %v");

            logger_ = std::make_shared<spdlog::logger>("ChromaForge", spdlog::sinks_init_list{console_sink, file_sink});
            logger_->set_level(spdlog::level::trace);
            logger_->flush_on(spdlog::level::err);

            spdlog::register_logger(logger_);
        } catch (const spdlog::spdlog_ex& e) {
            std::cerr << "Logger initialization failed: " << e.what() << std::endl;
            throw std::runtime_error("Logger initialization failed");
        }
    }

    void set_console_level(LogLevel level) {
        if (!logger_) return;
        for (auto& sink : logger_->sinks()) {
            if (sink == logger_->sinks()[0]) {
                sink->set_level(to_spdlog_level(level));
                break;
            }
        }
    }

    void set_file_level(LogLevel level) {
        if (!logger_ || logger_->sinks().size() < 2) return;
        logger_->sinks()[1]->set_level(to_spdlog_level(level));
    }

    void set_logger_level(LogLevel level) {
        if (logger_) logger_->set_level(to_spdlog_level(level));
    }

    void log(LogLevel level, const char* file, int line, const char* function, const std::string& message) {
        if (!logger_) return;
        spdlog::source_loc loc{file, line, function};
        logger_->log(loc, to_spdlog_level(level), "{}", message);
    }

    void flush() {
        if (logger_) logger_->flush();
    }
};

Logger::Logger() : pimpl_(std::make_unique<Impl>()) {}

Logger::~Logger() = default;

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& logFile, LogLevel consoleLevel, LogLevel fileLevel) {
    pimpl_->initialize(logFile, consoleLevel, fileLevel);
}

void Logger::setConsoleLevel(LogLevel level) {
    pimpl_->set_console_level(level);
}

void Logger::setFileLevel(LogLevel level) {
    pimpl_->set_file_level(level);
}

void Logger::setLoggerLevel(LogLevel level) {
    pimpl_->set_logger_level(level);
}

void Logger::log_impl(LogLevel level, const char* file, int line, const char* function, const std::string& message) {
    pimpl_->log(level, file, line, function, message);
}

void Logger::flush() {
    pimpl_->flush();
}
