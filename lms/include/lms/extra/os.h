#ifndef LMS_EXTRA_OS_H
#define LMS_EXTRA_OS_H

#include <string>

namespace lms {
namespace extra {

/**
 * @brief Returns the user's login name.
 *
 * This function uses getenv on all systems. This allows the user to overwrite
 * it as needed. (We don't need the real name, just a unique identifier.)
 *
 * @return current login name
 */
std::string username();

}  // namespace extra
}  // namespace lms

#endif /* LMS_EXTRA_OS_H */
