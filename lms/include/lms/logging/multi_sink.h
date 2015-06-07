#ifndef LMS_LOGGING_MULTI_SINK_H
#define LMS_LOGGING_MULTI_SINK_H

#include <vector>
#include <memory>

#include "lms/logging/sink.h"

namespace lms {
namespace logging {

/**
 * @brief A multi sink is a sink implementation that delegates all sink calls
 * to its child sinks.
 */
class MultiSink : public Sink {
public:
    /**
     * @brief Create a multi sink without any child sinks.
     */
    MultiSink() {}

    /**
     * @brief Add a sink. The instance is managed by this MultiSink instance.
     *
     * Do not delete the given sink instance outside of multi sink.
     *
     * @param sink a sink instance
     */
    void add(Sink *sink);

    void sink(const LogMessage &message);
private:
    std::vector<std::unique_ptr<Sink>> m_sinks;
};

}  // namespace logging
}  // namespace lms

#endif /* LMS_LOGGING_MULTI_SINK_H */
