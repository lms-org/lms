#include <memory>

#include <lms/logger.h>
#include <lms/extra/colors.h>

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

void Logger::time(const std::string &timerName) {
    // trigger a debug message to inform the programmer
    debug(timerName) << "started";

    // at LAST: save the current time in our cache
    timestampCache[timerName] = extra::PrecisionTime::now();
}

void Logger::timeEnd(const std::string &timerName) {
    // at FIRST: save the current time
    extra::PrecisionTime endTime = extra::PrecisionTime::now();

    // check if time() was called with the same timer name.
    auto it = timestampCache.find(timerName);

    if(it == timestampCache.end()) {
        // if not found: trigger bad debug message
        debug(timerName) << "timeEnd() was called without time()";
    } else {
        // compute time difference
        extra::PrecisionTime deltaTime = endTime - it->second;

        // print delta time as debug message
        debug(timerName) << deltaTime;

        // remove timestamp from our cache
        timestampCache.erase(it);
    }
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

