#ifndef LMS_INTERNAL_BACKTRACE_FORMATTER_H
#define LMS_INTERNAL_BACKTRACE_FORMATTER_H

#include <iostream>

namespace lms {
namespace internal {

/**
 * @brief Print the current stack trace on stdout.
 */
void printStacktrace(std::ostream &os);

} // namespace internal
} // namespace lms

#endif // LMS_INTERNAL_BACKTRACE_FORMATTER_H
