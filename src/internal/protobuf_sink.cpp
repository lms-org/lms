#include "protobuf_sink.h"
#include "messages.pb.h"

namespace lms {
namespace internal {

ProtobufSink::ProtobufSink(int socket, logging::Level level) : socket(socket), level(level) {}

void ProtobufSink::sink(const lms::logging::Event &message) {
    if(message.level < level) return;

    Response response;
    Response::LogEvent *event = response.mutable_log_event();
    event->set_tag(message.tag);
    event->set_level(static_cast<Response::LogEvent::Level>(message.level));
    event->set_text(message.messageText());
    event->set_timestamp(message.timestamp.micros());

    {
        std::lock_guard<std::mutex> lck(mtx);
        socket.writeMessage(response);
    }
}

}
}
