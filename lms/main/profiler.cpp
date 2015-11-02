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

        // lock after checking enabled flag and saving the current time
        std::unique_lock<std::mutex> lock(m_mutex);

        lms::extra::PrecisionTime diff = now - m_lastTimestamp;
        m_lastTimestamp = now;

        MappingType::iterator it = m_stringMapping.find(label);
        size_t id;

        if(it == m_stringMapping.end()) {
            // id is an ascending value starting with 0
            id = m_stringMapping.size();
            // insert new string mapping
            m_stringMapping[label] = id;

            // write string mapping to file (for later parsing)
            m_stream << static_cast<int>(MAPPING) << "," << id << ","
                << label << "\n";
        } else {
            // otherwise just take the stored id
            id = it->second;
        }

        m_stream << static_cast<int>(type) << "," << id << "," <<
            diff.micros() << "\n";
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
