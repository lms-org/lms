#include "lms/profiler.h"

namespace lms {

Profiler::Profiler() : m_enabled(false),
    m_lastTimestamp(lms::extra::PrecisionTime::now())
{}

Profiler::~Profiler() {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_stream.is_open()) {
        m_stream.close();
    }
}

void Profiler::markBegin(const std::string &label) {
    mark(BEGIN, label);
}

void Profiler::markEnd(const std::string &label) {
    mark(END, label);
}

void Profiler::mark(Type type, const std::string &label) {
    if(m_enabled) {
        lms::extra::PrecisionTime now = lms::extra::PrecisionTime::now();

        std::unique_lock<std::mutex> lock(m_mutex);

        lms::extra::PrecisionTime diff = now - m_lastTimestamp;

        if(m_stream.is_open()) {
            m_stream << static_cast<int>(type) << "," << diff.micros() << "," <<
                label << "\n";
        }

        m_lastTimestamp = now;
    }
}

void Profiler::enable(const std::string &file) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(! m_enabled) {
        m_stream.open(file);
        m_enabled = true;
    }
}

}  // namespace lms
