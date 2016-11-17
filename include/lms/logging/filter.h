#ifndef LMS_LOGGING_FILTER_H
#define LMS_LOGGING_FILTER_H

#include <string>

#include "level.h"

namespace lms {
namespace logging {

/**
 * @brief A logging filter can prevent a log message
 * from becoming sinked.
 *
 * NOTE: This class is abstract. You must override filter()
 * in your implementation.
 *
 * @author Hans Kirchner
 */
class Filter {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~Filter() {}

    /**
     * @brief This method will be called for every log message before being sent
     * to a sink.
     * @param level logging level
     * @param tag logging tag
     * @return true if the log message should be sinked, false if the log
     * message
     * should be ignored.
     */
    virtual bool decide(Level level, const std::string &tag) = 0;
};

} // namespace logging
} // namespace lms

#endif /* LMS_LOGGING_FILTER_H */
