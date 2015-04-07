#ifndef LMS_LOGGING_CHILD_LOGGER_H
#define LMS_LOGGING_CHILD_LOGGER_H

#include <string>
#include <iostream>

#include "log_level.h"
#include "logger.h"
#include "logging_filter.h"

namespace lms {
namespace logging {

/**
 * @brief A child logger is forwarding all logging
 * messages to its parent.
 *
 * The parent logger can be the root logger or another
 * child logger.
 *
 * @author Hans Kirchner
 */
class ChildLogger : public Logger {
public:
    /**
     * @brief Create a new child logger with the given name and parent.
     *
     * NOTE: if the parent logger gets deleted before the child logger
     * is deleted then that will cause undefined behavior. So make
     * sure you delete all childs first.
     *
     * @param name logger's name, will be prepended to the tag
     * @param parent all logging messages will be delegated to this parent
     */
    ChildLogger(const std::string &name, Logger *parent,
                std::unique_ptr<LoggingFilter> filter = nullptr);

    // TODO remove this, it's only for debugging
    ~ChildLogger() { std::cout << "Delete child logger " << name << std::endl; }

    /**
     * @brief Set the new filter for this child logger.
     *
     * The old filter will be deleted.
     *
     * @param filter a filter instance
     */
    void filter(std::unique_ptr<LoggingFilter> filter);

    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) override;
private:
    /**
     * @brief Delegate all logging outputs to this parent logger.
     */
    Logger *parent;

    /**
     * @brief Name of the child logger
     */
    std::string name;

    std::unique_ptr<LoggingFilter> m_filter;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_CHILD_LOGGER_H */
