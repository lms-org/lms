#include <memory>
#include <cstring>
#include <cerrno>

#include <lms/logger.h>

namespace lms {
namespace logging {

Logger::Logger(Context *context, const std::string &name, Level threshold)
    : context(context), name(name), threshold(threshold) {}

Logger::Logger(const std::string &name, Level threshold)
    : context(&Context::getDefault()), name(name), threshold(threshold) {}

std::unique_ptr<Event> Logger::debug(const std::string &tag) {
    return log(Level::DEBUG, tag);
}

std::unique_ptr<Event> Logger::info(const std::string &tag) {
    return log(Level::INFO, tag);
}

std::unique_ptr<Event> Logger::warn(const std::string &tag) {
    return log(Level::WARN, tag);
}

std::unique_ptr<Event> Logger::error(const std::string &tag) {
    return log(Level::ERROR, tag);
}

std::unique_ptr<Event> Logger::perror(const std::string &tag) {
    // http://stackoverflow.com/a/901316
    // http://linux.die.net/man/3/strerror
    char msg[64];
    char *msgPtr = msg;

#if defined(__GNUC__) && defined(__CYGWIN__)
    msgPtr = strerror(errno); // simple, not thread-safe implementation
#elif defined(_WIN32)
    if (strerror_s(msg, sizeof msg, errno) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        msg[sizeof msg - 1] = '\0';
    }
#elif((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE) || \
    defined(__APPLE__)
    if (strerror_r(errno, msg, sizeof msg) != 0) {
        strncpy(msg, "Unknown error", sizeof msg);
        msg[sizeof msg - 1] = '\0';
    }
#else
    msgPtr = strerror_r(errno, msg, sizeof msg);
#endif

    return log(Level::ERROR, tag) << msgPtr << " - ";
}

void Logger::time(const std::string &tag) {
    context->time(tag.empty() ? name : name + "." + tag);
}

void Logger::timeEnd(const std::string &tag) {
    context->timeEnd(tag.empty() ? name : name + "." + tag);
}

std::unique_ptr<Event> Logger::log(Level lvl, const std::string &tag) {
    if (context == nullptr) {
        std::cerr << "LOGGER " << name << " HAS NO VALID CONTEXT" << std::endl;
        return nullptr;
    }

    Filter *filter = context->filter();

    std::string newTag;

    if (tag.empty()) {
        // if no tag was given, just use the logger's name
        newTag = name;
    } else {
        // otherwise concatenate with the given tag
        newTag = name + "." + tag;
    }

    if (lvl >= threshold &&
        (filter == nullptr || filter->decide(lvl, newTag))) {
        return std::unique_ptr<Event>(new Event(*context, lvl, newTag, lms::Time::now()));
    } else {
        return nullptr;
    }
}

} // namespace logging
} // namespace lms
