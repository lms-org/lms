#include <lms/logger.h>

namespace lms {
namespace logging {

ThresholdFilter::ThresholdFilter(Level defaultThreshold)
    : m_defaultThreshold(defaultThreshold) {}

void ThresholdFilter::addPrefix(const std::string &prefix, Level threshold) {
    m_mappings[prefix] = threshold;
}

void ThresholdFilter::clearPrefixes() { m_mappings.clear(); }

void ThresholdFilter::defaultThreshold(Level threshold) {
    m_defaultThreshold = threshold;
}

Level ThresholdFilter::defaultThreshold() const { return m_defaultThreshold; }

bool ThresholdFilter::decide(Level level, const std::string &tag) {
    for (const auto &pair : m_mappings) {
        if (tag.compare(0, pair.first.size(), pair.first) == 0) {
            return level >= pair.second;
        }
    }

    return level >= m_defaultThreshold;
}

} // namespace logging
} // namespace lms
