#include "protobuf_sink.h"
#include "messages.pb.h"

namespace lms {
namespace internal {

ProtobufSink::ProtobufSink(int socket) : socket(socket) {}

void ProtobufSink::sink(const lms::logging::Event &message) {
    Response response;
    Response::LogEvent *event = response.mutable_log_event();
    event->set_tag(message.tag);
    event->set_level(static_cast<Response::LogEvent::Level>(message.level));
    event->set_text(message.messageText());

    {
        std::lock_guard<std::mutex> lck(mtx);
        socket.writeMessage(response);
    }
}

}
}
