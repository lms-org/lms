#include <lms/logger.h>

namespace lms {
namespace logging {

PrefixAndLevelFilter::PrefixAndLevelFilter(LogLevel minLevel, const std::vector<std::string> &prefixes)
    : m_minLevel(minLevel), m_prefixes(prefixes) {}

void PrefixAndLevelFilter::addPrefix(const std::string &prefix) {
    m_prefixes.push_back(prefix);
}

void PrefixAndLevelFilter::minLevel(LogLevel minLevel) {
    m_minLevel = minLevel;
}

bool PrefixAndLevelFilter::filter(LogLevel level, const std::string &tag) {
    if(level < m_minLevel) {
        return false;
    }

    // if no prefixes are given, then all tags are valid
    if(m_prefixes.empty()) {
        return true;
    }

    for(const std::string &prefix : m_prefixes) {
        if(tag.compare(0, prefix.size(), prefix) == 0) {
            return true;
        }
    }

    return false;
}

} // namespace logging
} // namespace lms

