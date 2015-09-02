#ifndef LMS_LOGGING_CONTEXT_H
#define LMS_LOGGING_CONTEXT_H

#include <memory>
#include <string>
#include <iostream>
#include <vector>

namespace lms {
namespace logging {

class Sink;
class LoggingFilter;

/**
 * @brief Instantiate one logging context per application.
 *
 * The context object
 *
 * @author Hans Kirchner
 */
class Context {
public:

    /**
     * @brief Return default logging context for this process.
     *
     * The context object is lazy-initialized. It will be created the
     * first time this method is called.
     */
    static Context& getDefault();

    /**
     * @brief Create a new context with no filters and no sinks.
     */
    Context();

    /**
     * @brief Do not allow copy constructing.
     */
    Context(Context const&) = delete;

    /**
     * @brief Do not allow copy assignments.
     */
    Context& operator=(Context const&) = delete;

    /**
     * @brief Add a new logging sink.
     *
     * The instance is managed by this context
     * and gets deleted automatically when the context
     * is destroyed.
     *
     * @param sink a sink instance
     */
    void appendSink(Sink *sink);

    /**
     * @brief Remove all appended sinks.
     */
    void clearSinks();

    /**
     * @brief Install a logging filter
     *
     * The instance will be managed by this context
     * and gets deleted automatically when the context
     * is destroyed.
     *
     * @param filter a filter instance
     */
    void filter(Filter *filter);

    Filter* filter() const;

    /**
     * @brief Check if a filter was previously installed.
     *
     * @return true if filter is installed, false otherwise
     */
    bool hasFilter();

    void processMessage(const Event &message);
private:
    std::vector<std::unique_ptr<Sink>> m_sinks;
    std::unique_ptr<Filter> m_filter;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_CONTEXT_H */

