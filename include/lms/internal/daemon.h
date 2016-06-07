#pragma once

namespace lms {
namespace internal {

/**
 * Create a daemon of this process. This performs the seven steps to daemonize.
 *
 * Any errors during creation result in an exit(EXIT_FAILURE).
 * @return true in the child, false in the parent
 */
bool daemonize();

} // namespace internal
} // namespace lms