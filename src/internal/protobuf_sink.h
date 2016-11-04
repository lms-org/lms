#pragma once

#include <mutex>

#include "lms/logging/sink.h"
#include "lms/logging/event.h"
#include "lms/protobuf_socket.h"

namespace lms {
namespace internal {

class ProtobufSink : public lms::logging::Sink {
public:
    ProtobufSink(int socket, logging::Level level);

    ~ProtobufSink() {}

    void sink(const lms::logging::Event &message) override;
private:
    lms::ProtobufSocket socket;
    std::mutex mtx;
    logging::Level level;
};

}  // namespace internal
}  // namespace lms
