#ifndef LMS_LOGGING_LOG_LEVEL_H
#define LMS_LOGGING_LOG_LEVEL_H

#include <cstdint>
#include <string>

namespace lms {
namespace logging {

/**
 * @brief All available log levels
 *
 * The log levels should be ordered by their importance.
 *
 * NOTE: This is a C++11 enum class. Look it up if
 * you do not know what that is.
 *
 * @author Hans Kirchner
 */
enum class LogLevel : std::int8_t {
    DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4
};

const LogLevel SMALLEST_LEVEL = LogLevel::DEBUG;

/**
 * @brief Returns the name of the given log level.
 *
 * Example: LogLevel::DEBUG returns "DEBUG".
 *
 * @param lvl a log level
 * @return string representation of the given log level
 */
std::string levelName(LogLevel lvl);

/**
 * @brief Returns the log level from a given name.
 *
 * Example: levelFromName("WARN") returns LogLevel::WARN.
 *
 * @param name exact name of a log level
 * @return log level
 */
LogLevel levelFromName(const std::string &name);

/**
 * @brief Returns a linux terminal compatible color code
 * suitable for the log level.
 *
 * Example: LogLevel::ERROR returns COLOR_RED
 *
 * @param lvl a log level
 * @return an ANSI escape color code
 */
std::string levelColor(LogLevel lvl);

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_LOG_LEVEL_H */

