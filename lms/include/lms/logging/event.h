#ifndef LMS_LOGGING_EVENT_H
#define LMS_LOGGING_EVENT_H

#include <string>
#include <sstream>

#include "level.h"

namespace lms {
namespace logging {

class Sink;
class Context;

/**
 * @brief A log message is not more than a struct consisting
 * of a log level, tag and an appendable log message.
 *
 * A log message is appendable in the following way:
 * message << "Log message No. " << 1;
 *
 * @author Hans Kirchner
 */
class Event {
public:
    /**
     * @brief Create a new log message with the given values.
     * @param sink Non-null pointer to a sink implementation instance
     * @param lvl logging level
     * @param tag logging tag (used for logging hierarchies and log filtering)
     */
    Event(Context &ctx, Level level, const std::string& tag)
        : tag(tag), level(level), ctx(ctx) {}

    /**
     * @brief The destructor will flush the log message to the
     * sink given in the constructor.
     *
     * If the log message is wrapped in a unique pointer,
     * this will happen automatically.
     */
    ~Event();

    /**
     * @brief The tag of this log message
     */
    const std::string tag;

    /**
     * @brief The log level (severity) of this log message.
     */
    const Level level;

    /**
     * @brief A reference to a sink instance.
     *
     * This log message will be flushed to
     * that sink in the destructor.
     */
    Context &ctx;

    /**
     * @brief The logging message.
     */
    std::string messageText() const {
        return messageStream.str();
    }

    /**
     * @brief The logging message stream.
     *
     * You can append to this stream via the << operator.
     */
    std::ostringstream messageStream;
};

/**
 * @brief Make the Event appendable.
 */
std::unique_ptr<Event> operator << (std::unique_ptr<Event> message,
                                         std::ostream& (*pf) (std::ostream&));

/**
 * @brief Make the Event appendable.
 */
template <typename T>
std::unique_ptr<Event> operator << (std::unique_ptr<Event> message, T const& value) {
    if(message) {
        message->messageStream << value;
    }

    return message;
}

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_EVENT_H */

