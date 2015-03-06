#include <memory>

#include <core/logger.h>
#include <core/extra/colors.h>

namespace lms {
namespace logging {

std::unique_ptr<LogMessage> Logger::debug(const std::string& tag) {
    return log(LogLevel::DEBUG, tag);
}

std::unique_ptr<LogMessage> Logger::info(const std::string& tag) {
    return log(LogLevel::INFO, tag);
}

std::unique_ptr<LogMessage> Logger::warn(const std::string& tag) {
    return log(LogLevel::WARN, tag);
}

std::unique_ptr<LogMessage> Logger::error(const std::string& tag) {
    return log(LogLevel::ERROR, tag);
}

std::string Logger::levelName(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return "DEBUG";
    case LogLevel::INFO :  return "INFO ";
    case LogLevel::WARN :  return "WARN ";
    case LogLevel::ERROR : return "ERROR";
    default : return "_"; // this should never happen
    }
}

LogLevel Logger::levelFromName(const std::string &name) {
    if(name == "DEBUG") return LogLevel::DEBUG;
    if(name == "INFO") return LogLevel::INFO;
    if(name == "WARN") return LogLevel::WARN;
    if(name == "ERROR") return LogLevel::ERROR;
    // TODO better error handling here
    std::cerr << "Unknown logging level: " << name << std::endl;
    return LogLevel::DEBUG;
}

std::string Logger::levelColor(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return COLOR_GREEN;
    case LogLevel::INFO : return COLOR_BLUE;
    case LogLevel::WARN : return COLOR_YELLOW;
    case LogLevel::ERROR : return COLOR_RED;
    default: return COLOR_WHITE; // this should never happen
    }
}

} // namespace logging
} // namespace lms

