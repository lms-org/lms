#include <lms/logger.h>

namespace lms {
namespace logging {

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

