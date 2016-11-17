#ifndef LMS_LOGGING_CONTEXT_H
#define LMS_LOGGING_CONTEXT_H

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>

#include "trace.h"
#include "level.h"

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
    static Context &getDefault();

    /**
     * @brief Create a new context with no filters and no sinks.
     */
    Context();

    /**
     * @brief Destroy context.
     *
     * This will make all dependent loggers invalid
     */
    ~Context();

    /**
     * @brief Do not allow copy constructing.
     */
    Context(Context const &) = delete;

    /**
     * @brief Do not allow copy assignments.
     */
    Context &operator=(Context const &) = delete;

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

    /**
     * @brief Return the installed logging filter.
     * @return installed logging filter, or nullptr if none is installed
     */
    Filter *filter() const;

    /**
     * @brief Check if a filter was previously installed.
     *
     * @return true if filter is installed, false otherwise
     */
    bool hasFilter();

    /**
     * @brief Forward the logging event to all installed sinks.
     * @param message logging event
     */
    void processMessage(const Event &message);

    /**
     * @brief Reset profiling. Set internal state back to zero.
     */
    void resetProfiling();

    /**
     * @brief Start profiling an
     * @param tag Marker name
     * @param timestamp
     */
    void time(const std::string &tag);

    /**
     * @brief End profiling and save
     * @param tag Marker name
     * @param timestamp
     */
    void timeEnd(const std::string &tag);

    /**
     * @brief Get summary of time()/timeEnd() calls.
     *
     * Every tag used in time()/timeEnd() leads to a pair in the mapping.
     *
     * @param measurements mapping of tag to summary
     */
    void profilingSummary(std::map<std::string, Trace<double>> &measurements);

    /**
     * @brief Set global log level
     * @param level new logging level
     */
    void setLevel(logging::Level level);

private:
    struct Private;
    Private *dptr;
    inline Private *dfunc() { return dptr; }
    inline const Private *dfunc() const { return dptr; }
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_CONTEXT_H */
