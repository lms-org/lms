#ifndef LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H
#define LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H

#include <vector>

namespace lms {
namespace logging {

class PrefixAndLevelFilter : public LoggingFilter {
public:
    explicit PrefixAndLevelFilter(LogLevel minLevel = SMALLEST_LEVEL,
                                  const std::vector<std::string> &prefixes = {}) :
        m_minLevel(minLevel), m_prefixes(prefixes) {}

    void addPrefix(const std::string &prefix) {
        m_prefixes.push_back(prefix);
    }

    void minLevel(LogLevel minLevel) {
        m_minLevel = minLevel;
    }

    bool filter(LogLevel level, const std::string &tag) override;

private:
    LogLevel m_minLevel;
    std::vector<std::string> m_prefixes;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H */

