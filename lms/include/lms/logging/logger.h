#ifndef LMS_LOGGING_LOGGER_H
#define LMS_LOGGING_LOGGER_H

#include <string>
#include <memory>
#include <map>

#include "level.h"
#include "lms/extra/time.h"

namespace lms {
namespace logging {

class Event;
class Context;

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
     * @brief Create a new logger with the given name and context.
     *
     * @param name logger's name, will be prepended to the tag
     * @param parent all logging messages will be delegated to the context
     * @threshold minimum logging level
     */
    Logger(Context *context, const std::string &name, Level threshold = Level::ALL);

    /**
     * @brief Create a new logger with the default context and the given name.
     * @param name logger's name, will be prepended to the tag
     * @param threshold minimum logging level
     */
    explicit Logger(const std::string &name, Level threshold = Level::ALL);

    /**
     * @brief Log a debug message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<Event> debug(const std::string& tag = "");

    /**
     * @brief Log an info message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<Event> info(const std::string& tag = "");

    /**
     * @brief Log a warning message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<Event> warn(const std::string& tag = "");

    /**
     * @brief Log an error message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<Event> error(const std::string& tag = "");

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
    std::unique_ptr<Event> perror(const std::string &tag = "");

    /**
     * @brief Log a message with the given level and tag.
     *
     * Usually you should use debug(), info(), warn() or error().
     *
     * @param lvl logging level (severity)
     * @param tag logging tag
     * @return an appendable logging message that will be automatically flushed
     */
    std::unique_ptr<Event> log(Level lvl, const std::string& tag = "");

    /**
     * @brief Delegate all logging outputs to the logging context.
     */
    Context *context;

    /**
     * @brief Name of the child logger
     */
    std::string name;

    /**
     * Minimum logging level.
     */
    Level threshold;
private:
    std::map<std::string, extra::PrecisionTime> m_timestampCache;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_LOGGER_H */

