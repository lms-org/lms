#ifndef LMS_LOGGING_DEBUG_SERVER_SINK_H
#define LMS_LOGGING_DEBUG_SERVER_SINK_H

#include "sink.h"
#include "../internal/debug_server.h"

namespace lms {
namespace logging {

class DebugServerSink : public Sink {
public:
    DebugServerSink(internal::DebugServer *server);

    void sink(const Event &message);

private:
    internal::DebugServer *m_server;
};

} // namespace logging
} // namespace lms

#endif // LMS_LOGGING_DEBUG_SERVER_SINK_H
