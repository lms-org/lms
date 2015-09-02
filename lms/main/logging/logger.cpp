#include <memory>
#include <cstring>
#include <cerrno>

#include <lms/logger.h>

namespace lms {
namespace logging {

std::unique_ptr<LogMessage> Logger::debug(const std::string& tag) {
    return log(Level::DEBUG, tag);
}

std::unique_ptr<LogMessage> Logger::info(const std::string& tag) {
    return log(Level::INFO, tag);
}

std::unique_ptr<LogMessage> Logger::warn(const std::string& tag) {
    return log(Level::WARN, tag);
}

std::unique_ptr<LogMessage> Logger::error(const std::string& tag) {
    return log(Level::ERROR, tag);
}

std::unique_ptr<LogMessage> Logger::perror(const std::string &tag) {
    // http://stackoverflow.com/a/901316
    // http://linux.die.net/man/3/strerror
    char msg[64];
    char *msgPtr = msg;

    #if defined(__GNUC__) && defined(__CYGWIN__)
    msgPtr = strerror(errno);  // simple, not thread-safe implementation
    #elif defined(_WIN32)
    if (strerror_s(msg, sizeof msg, errno) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        msg[sizeof msg - 1] = '\0';
    }
    #elif ( (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE ) || defined(__APPLE__)
    if (strerror_r(errno, msg, sizeof msg) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        msg[sizeof msg - 1] = '\0';
    }
    #else
    msgPtr = strerror_r(errno, msg, sizeof msg);
    #endif

    return log(Level::ERROR, tag) << msgPtr << " - ";
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

} // namespace logging
} // namespace lms

