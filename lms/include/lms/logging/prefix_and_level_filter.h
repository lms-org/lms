#ifndef LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H
#define LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H

#include <vector>

namespace lms {
namespace logging {

/**
 * @brief Simple filter implementation that filters
 * for minimal logging levels and tag prefixes.
 *
 * If no prefixes are given then only the minimum
 * logging level will be filtered.
 *
 * @author Hans Kirchner
 */
class PrefixAndLevelFilter : public LoggingFilter {
public:
    /**
     * @brief Create a new filter with the given minimum level and
     * prefixes.
     *
     * @param minLevel defaults to lms::logging::SMALLEST_LEVEL
     * @param prefixes tag prefixes to filter for
     */
    explicit PrefixAndLevelFilter(LogLevel minLevel = SMALLEST_LEVEL,
                                  const std::vector<std::string> &prefixes = {});

    /**
     * @brief Add a tag prefix to the the filter list.
     *
     * @param prefix tag prefix
     */
    void addPrefix(const std::string &prefix);

    /**
     * @brief Set the minimum logging level.
     *
     * @param minLevel new minimum level
     */
    void minLevel(LogLevel minLevel);

    /**
     * @brief Overridden filter method.
     */
    bool filter(LogLevel level, const std::string &tag) override;

private:
    LogLevel m_minLevel;
    std::vector<std::string> m_prefixes;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H */

