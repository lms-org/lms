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
enum class Level : std::uint8_t {
    //! The lowest possible rank, will turn on all logging.
    ALL = 0,
    //! Detailed information for debugging.
    DEBUG = 1,
    //! Should be used for startup/shutdown events.
    INFO = 2,
    //! Almost errors that do not interrupt the program's flow.
    WARN = 3,
    //! Runtime errors, hardware problems.
    ERROR = 4,
    //! The highest possible rank, will turn off all logging.
    OFF = 255
};

/**
 * @brief Returns the name of the given log level.
 *
 * Example: Level::DEBUG returns "DEBUG".
 *
 * @param lvl a log level
 * @return string representation of the given log level
 */
std::string levelName(Level lvl);

/**
 * @brief Returns the log level from a given name.
 *
 * Example: levelFromName("WARN") returns Level::WARN.
 *
 * @param name exact name of a log level
 * @param level the result will be stored in this parameter
 * @return true if parsing successful, false otherwise
 */
bool levelFromName(const std::string &name, Level &level);

/**
 * @brief Returns a linux terminal compatible color code
 * suitable for the log level.
 *
 * Example: Level::ERROR returns COLOR_RED
 *
 * @param lvl a log level
 * @return an ANSI escape color code
 */
std::string levelColor(Level lvl);

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_LOG_LEVEL_H */

