#include "debug_server_sink.h"
#include "lms/logging/event.h"
#include <limits>
#include <algorithm>

namespace lms {
namespace logging {

DebugServerSink::DebugServerSink(internal::DebugServer *server)
    : m_server(server) {}

void DebugServerSink::sink(const Event &message) {
    constexpr std::uint8_t MAX_LEN = std::numeric_limits<std::uint8_t>::max();

    std::string text = message.messageText();
    std::string tag = message.tag;

    uint8_t tagLen = std::min(size_t(MAX_LEN), tag.size());
    uint8_t textLen = std::min(size_t(MAX_LEN), text.size());

    // Format: Level + Tag Size + Tag + Message Size + Message
    uint32_t len = 3 + tagLen + textLen;

    internal::DebugServer::Datagram datagram(
        internal::DebugServer::MessageType::LOGGING, len);

    datagram.data()[0] = static_cast<std::uint8_t>(message.level);
    datagram.data()[1] = tagLen;
    datagram.data()[2] = textLen;
    std::copy(tag.begin(), tag.begin() + tagLen, &datagram.data()[3]);
    std::copy(text.begin(), text.begin() + textLen,
              &datagram.data()[3 + tagLen]);

    m_server->broadcast(datagram);
}

} // namespace logging
} // namespace lms
