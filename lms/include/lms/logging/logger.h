#ifndef LMS_LOGGING_LOGGER_H
#define LMS_LOGGING_LOGGER_H

#include <string>
#include <memory>
#include <map>

#include "log_level.h"
#include "lms/extra/time.h"

namespace lms {
namespace logging {

class LogMessage;

/**
 * @brief A logger is able to receive logging messages
 * and forwards them to logging sinks.
 *
 * General usage:
 * logger.debug() << "Log message with parameters " << 42 << "!";
 *
 * NOTE: This class is abstract.
 *
 * @author Hans Kirchner
 */
class Logger {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Logger() {}

    /**
     * @brief Log a debug message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> debug(const std::string& tag = "");

    /**
     * @brief Log an info message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> info(const std::string& tag = "");

    /**
     * @brief Log a warning message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> warn(const std::string& tag = "");

    /**
     * @brief Log an error message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> error(const std::string& tag = "");

    /**
     * @brief Start a timer with the specified name.
     *
     * This will save a timestamp. Use timeEnd() with the
     * same timer name to log the duration between the time()
     * and timeEnd() calls.
     *
     * Each logger instance has its own timestamp storage.
     * You can NOT use time() and timeEnd() with the same
     * timer name on different logger instances and expect
     * it to work.
     *
     * @param timerName name for the timer
     */
    void time(const std::string &timerName);

    /**
     * @brief End a timer and log the past time as
     * a debug message.
     *
     * This will retrieve the saved timestamp from time()
     * and subtract it from the current time.
     * A debug log message will be printed.
     * The saved timestamp from time() will be deleted.
     *
     * @param timerName name for the timer
     */
    void timeEnd(const std::string &timerName);

    /**
     * @brief Log an error message prepended by strerror(errno).
     *
     * Useful for linux api calls and calls to standard c functions.
     */
    std::unique_ptr<LogMessage> perror(const std::string &tag = "");

    /**
     * @brief Log a message with the given level and tag.
     *
     * Usually you should use debug(), info(), warn() or error().
     *
     * @param lvl logging level (severity)
     * @param tag logging tag
     * @return an appendable logging message that will be automatically flushed
     */
    virtual std::unique_ptr<LogMessage> log(Level lvl, const std::string& tag) = 0;
protected:
    /**
     * @brief Logger is abstract.
     */
    Logger() {}
private:
    std::map<std::string, extra::PrecisionTime> timestampCache;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_LOGGER_H */

