#ifndef LMS_LOGGING_SINK_H
#define LMS_LOGGING_SINK_H

namespace lms {
namespace logging {

class Event;

/**
 * @brief A logging sink is the abstract concept
 * of a service that is able to write or append a log message
 * to a given target (e.g. terminal, file, network socket)
 *
 * This class is abstract. You must override the sink method
 * in your implementation.
 *
 * @author Hans Kirchner
 */
class Sink {
public:
    /**
     * @brief Virtual destructor. Therefore, can be overridden in subclasses.
     */
    virtual ~Sink() {}

    /**
     * @brief Write or append the given log message to an output.
     * @param message a log message to write
     */
    virtual void sink(const Event &message) = 0;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_SINK_H */
