#ifndef LMS_LOGGING_LOG_LEVEL_H
#define LMS_LOGGING_LOG_LEVEL_H

#include <cstdint>

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

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_LOG_LEVEL_H */

