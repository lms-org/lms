#include <memory>
#include <cstring>

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

std::unique_ptr<LogMessage> Logger::perror(const std::string &tag) {
    // http://stackoverflow.com/a/901316
    // http://linux.die.net/man/3/strerror
    char msg[64];
    char *msgPtr = msg;

    #if defined(_WIN32)
    if (strerror_s(msg, sizeof msg, errno) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        msg[sizeof msg - 1] = '\0';
    }
    #elif (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
    if (strerror_r(err_code, msg, sizeof msg) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        sys_msg[sizeof msg - 1] = '\0';
    }
    #else
    msgPtr = strerror_r(errno, msg, sizeof msg);
    #endif

    return log(LogLevel::ERROR, tag) << msgPtr << " - ";
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

