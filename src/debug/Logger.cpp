#include <debug/Logger.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <utility>

static std::ofstream file;
static std::mutex mutex;
static std::string utcOffset = "";
constexpr unsigned int moduleLen = 20;

using namespace debug;

Logger::Logger(const std::string& name) : name(name) {}

LogMessage::LogMessage(Logger* logger, LogLevel level)
    : logger(logger), level(level) {
}

LogMessage::~LogMessage() {
    logger->log(level, ss.str());
}

Logger& Logger::getInstance() {
    static Logger instance("ChromaForge");
    return instance;
}

static const char* plainLevelPrefix(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
        case LogLevel::Debug:
#ifdef NDEBUG
            return "";
#else
            return (level == LogLevel::Trace) ? "[T]" : "[D]";
#endif
        case LogLevel::Info:     return "[I]";
        case LogLevel::Warning:  return "[W]";
        case LogLevel::Error:    return "[E]";
        case LogLevel::Critical: return "[C]";
        default:                 return "[?]";
    }
}

static std::string coloredLevelPrefix(LogLevel level) {
    const char* color = "";
    switch (level) {
        case LogLevel::Trace:
        case LogLevel::Debug:
#ifdef NDEBUG
            return "";
#else
            color = (level == LogLevel::Trace) ? "\033[90m" : "\033[36m"; break;
#endif
        case LogLevel::Info:     color = "\033[32m"; break;
        case LogLevel::Warning:  color = "\033[33m"; break;
        case LogLevel::Error:    color = "\033[31m"; break;
        case LogLevel::Critical: color = "\033[35m"; break;
        default:                 return plainLevelPrefix(level);
    }
    return std::string(color) + plainLevelPrefix(level) + "\033[0m";
}

static void write(
    LogLevel level, const std::string& name, const std::string& message
) {
    #ifdef NDEBUG
        if (level == LogLevel::Trace || level == LogLevel::Debug) return;
    #endif

    if (level == LogLevel::Print) {
        std::cout << "[" << name << "]    " << message << std::endl;
        return;
    }

    std::stringstream ss;
    time_t tm = std::time(nullptr);
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ) % 1000;
    ss << " " << std::put_time(std::localtime(&tm), "%Y/%m/%d %T");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    ss << utcOffset << " [" << std::setfill(' ') << std::setw(moduleLen) << name << "] ";
    ss << message;
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto string = ss.str();
        if (file.good()) {
            file << plainLevelPrefix(level) << string << '\n';
            file.flush();
        }
        std::cout << coloredLevelPrefix(level) << string << std::endl;
    }
}

void Logger::init(const std::string& filename) {
    file.open(filename);

    time_t tm = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tm), "%z");
    utcOffset = ss.str();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex);
    file.flush();
}

void Logger::log(LogLevel level, std::string message) {
    write(level, name, std::move(message));
}
