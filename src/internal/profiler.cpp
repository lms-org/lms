#include <algorithm>

#include "profiler.h"
#include "debug_server.h"
#include "lms/endian.h"

namespace lms {
namespace internal {

Profiler::Profiler() {}

void Profiler::markBegin(const std::string &label) { mark(BEGIN, label); }

void Profiler::markEnd(const std::string &label) { mark(END, label); }

void Profiler::mark(Type type, const std::string &label) {
    lms::Time now = lms::Time::now();

    for (auto &listener : m_listeners) {
        listener->onMarker(type, now, label);
    }
}

void Profiler::appendListener(ProfilingListener *listener) {
    m_listeners.push_back(std::unique_ptr<ProfilingListener>(listener));
}

FileProfiler::FileProfiler(std::string const &file)
    : m_lastTimestamp(Time::now()) {
    m_stream.open(file);
}

FileProfiler::~FileProfiler() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}

void FileProfiler::onMarker(Profiler::Type type, Time now,
                            std::string const &label) {
    // lock after checking enabled flag and saving the current time
    std::unique_lock<std::mutex> lock(m_mutex);

    Time diff = now - m_lastTimestamp;
    m_lastTimestamp = now;

    MappingType::iterator it = m_stringMapping.find(label);
    size_t id;

    if (it == m_stringMapping.end()) {
        // id is an ascending value starting with 0
        id = m_stringMapping.size();
        // insert new string mapping
        m_stringMapping[label] = id;

        // write string mapping to file (for later parsing)
        m_stream << static_cast<int>(Profiler::MAPPING) << "," << id << ","
                 << label << "\n";
    } else {
        // otherwise just take the stored id
        id = it->second;
    }

    m_stream << static_cast<int>(type) << "," << id << "," << diff.micros()
             << "\n";
}

DebugServerProfiler::DebugServerProfiler(DebugServer *server)
    : m_server(server) {}

void DebugServerProfiler::onMarker(Profiler::Type type, Time now,
                                   std::string const &label) {
    std::uint8_t labelLen = std::min(
        label.size(), size_t(std::numeric_limits<std::uint8_t>::max()));

    internal::DebugServer::Datagram datagram(
        DebugServer::MessageType::PROFILING, 10 + labelLen);

    datagram.data()[0] = static_cast<std::uint8_t>(type);
    *reinterpret_cast<uint64_t *>(&datagram.data()[1]) =
        Endian::htobe(static_cast<uint64_t>(now.micros()));
    datagram.data()[9] = labelLen;

    std::copy(label.begin(), label.begin() + labelLen, &datagram.data()[10]);

    m_server->broadcast(datagram);
}

} // namespace internal
} // namespace lms
