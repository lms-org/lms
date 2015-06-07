#include "lms/logging/multi_sink.h"

namespace lms {
namespace logging {

void MultiSink::add(Sink *sink) {
    m_sinks.push_back(std::unique_ptr<Sink>(sink));
}

void MultiSink::sink(const LogMessage &message) {
    for(std::unique_ptr<Sink>& sink : m_sinks) {
        sink->sink(message);
    }
}

}  // namespace logging
}  // namespace lms
