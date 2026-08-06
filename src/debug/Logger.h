#pragma once

#include <sstream>
#include <string>

namespace debug {
    enum class LogLevel {
        Print,
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class Logger;

    class LogMessage {
        Logger* logger;
        LogLevel level;
        std::stringstream ss;
    public:
        LogMessage(Logger* logger, LogLevel level);

        ~LogMessage();

        LogMessage(const LogMessage&) = delete;
        LogMessage& operator=(const LogMessage&) = delete;
        LogMessage(LogMessage&&) = default;
        LogMessage& operator=(LogMessage&&) = default;

        template <class T>
        LogMessage& operator<<(const T& x) {
            ss << x;
            return *this;
        }
    };

    class Logger {
    public:
        explicit Logger(const std::string& name);

        static Logger& getInstance();
        static void init(const std::string& filename);
        static void flush();

        void log(LogLevel level, std::string message);

        LogMessage print() {
            return LogMessage(this, LogLevel::Print);
        }

        LogMessage trace() {
            return LogMessage(this, LogLevel::Trace);
        }

        LogMessage debug() {
            return LogMessage(this, LogLevel::Debug);
        }

        LogMessage info() {
            return LogMessage(this, LogLevel::Info);
        }

        LogMessage warning() {
            return LogMessage(this, LogLevel::Warning);
        }

        LogMessage error() {
            return LogMessage(this, LogLevel::Error);
        }

        LogMessage critical() {
            return LogMessage(this, LogLevel::Critical);
        }
    private:
        std::string name;
    };
};
