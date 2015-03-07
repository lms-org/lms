#ifndef LMS_LOGGING_ROOT_LOGGER_H
#define LMS_LOGGING_ROOT_LOGGER_H

#include <memory>
#include <string>
#include <iostream>

namespace lms {
namespace logging {

class Sink;
class LoggingFilter;

/**
 * @brief Instantiate one root logger per application.
 *
 * The root logger forwards all logging messages to its sink.
 *
 * TODO: implement filters
 *
 * @author Hans Kirchner
 */
class RootLogger : public Logger {
public:
    /**
     * @brief Create a new root logger with the given sink instance.
     *
     * NOTE: The sink instance is managed by this root logger.
     * Do not delete it manually, it will be deleted automatically
     * when the logger is deleted.
     *
     * @param sink a logging sink
     */
    explicit RootLogger(std::unique_ptr<Sink> sink, std::unique_ptr<LoggingFilter> filter);

    /**
     * @brief Create a new root logger with a default console sink.
     *
     * You can set the sink with the sink() method later. The old
     * sink will then be deleted.
     */
    RootLogger();

    ~RootLogger() { debug() << "Delete RootLogger"; }

    /**
     * @brief Do not allow the root logger to be copied.
     */
    RootLogger(const RootLogger &logger) = delete;

    /**
     * @brief Set the new sink for this root logger.
     *
     * The old sink will be deleted.
     *
     * @param sink a sink instance
     */
    void sink(std::unique_ptr<Sink> sink);

    /**
     * @brief Set the new filter for this root logger.
     *
     * The old filter will be deleted.
     *
     * @param filter a filter instance
     */
    void filter(std::unique_ptr<LoggingFilter> filter);

    /**
     * @brief Log a message with the given level and tag.
     * @param lvl logging level
     * @param tag logging tag
     * @return an appendable log message
     */
    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) override;
private:
     std::unique_ptr<Sink> m_sink;
     std::unique_ptr<LoggingFilter> m_filter;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_ROOT_LOGGER_H */

