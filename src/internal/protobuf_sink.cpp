#include "protobuf_sink.h"
#include "messages.pb.h"

namespace lms {
namespace internal {

ProtobufSink::ProtobufSink(int socket) : socket(socket) {}

void ProtobufSink::sink(const lms::logging::Event &message) {
    lms::LogEvent log;
    log.set_tag(message.tag);
    log.set_level(static_cast<lms::LogEvent_Level>(message.level));
    log.set_text(message.messageText());

    {
        std::lock_guard<std::mutex> lck(mtx);
        socket.writeMessage(log);
    }
}

}
}
