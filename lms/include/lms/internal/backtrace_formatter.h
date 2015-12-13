#ifndef LMS_INTERNAL_BACKTRACE_FORMATTER_H
#define LMS_INTERNAL_BACKTRACE_FORMATTER_H

namespace lms {
namespace internal {

/**
 * @brief Print the current stack trace on stdout.
 */
void printStacktrace();

}  // namespace internal
}  // namespace lms

#endif  // LMS_INTERNAL_BACKTRACE_FORMATTER_H
