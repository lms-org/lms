#include "lms/logging/debug_server_sink.h"
#include "lms/logging/event.h"
#include <limits>

namespace lms {
namespace logging {

DebugServerSink::DebugServerSink(internal::DebugServer *server) : m_server(server) {
}

void DebugServerSink::sink(const Event &message) {
    constexpr std::uint8_t MAX_LEN = std::numeric_limits<std::uint8_t>::max();

    std::string text = message.messageText();
    if(text.size() > MAX_LEN) {
        text = text.substr(0, MAX_LEN);
    }

    std::string tag = message.tag;
    if(tag.size() > MAX_LEN) {
        tag = tag.substr(0, MAX_LEN);
    }

    internal::DebugServer::Datagram datagram;

    // Format: Level + Tag Size + Tag + Message Size + Message
    datagram.data.resize(3 + tag.size() + text.size());

    datagram.data[0] = static_cast<std::uint8_t>(message.level);
    datagram.data[1] = tag.size();
    datagram.data[2] = text.size();
    std::copy(tag.begin(), tag.end(), &datagram.data[3]);
    std::copy(text.begin(), text.end(), &datagram.data[3 + tag.size()]);

    m_server->broadcast(static_cast<std::uint8_t>(internal::DebugServer::MessageType::LOGGING), datagram);
}

}  // namespace logging
}  // namespace lms
