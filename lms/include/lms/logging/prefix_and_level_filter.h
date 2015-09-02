#ifndef LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H
#define LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H

#include <map>
#include <string>

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
class ThresholdFilter : public Filter {
public:
    /**
     * @brief Create a new filter with the given minimum level and
     * prefixes.
     *
     * @param minLevel defaults to lms::logging::SMALLEST_LEVEL
     * @param prefixes tag prefixes to filter for
     */
    explicit ThresholdFilter(Level defaultThreshold = Level::ALL);

    /**
     * @brief Add a tag prefix to the the filter list.
     *
     * @param prefix tag prefix
     */
    void addPrefix(const std::string &prefix, Level threshold);

    /**
     * @brief Delete all added prefixes.
     */
    void clearPrefixes();

    /**
     * @brief Set the minimum logging level.
     *
     * @param minLevel new minimum level
     */
    void defaultThreshold(Level threshold);

    /**
     * @brief Returns the current default threshold.
     */
    Level defaultThreshold() const;

    /**
     * @brief Overridden filter method.
     */
    bool decide(Level level, const std::string &tag) override;

private:
    Level m_defaultThreshold;

    std::map<std::string,Level> m_mappings;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_PREFIX_AND_LEVEL_FILTER_H */

