#ifndef LMS_TYPE_MODULE_CONFIG_H
#define LMS_TYPE_MODULE_CONFIG_H

#include "lms/config.h"
#include "lms/deprecated.h"

namespace lms {
namespace type {

/**
 * @brief Moved lms::type::ModuleConfig to lms::ModuleConfig (lms::Config)
 *
 * This header and typedefs makes the change backwards compatible.
 *
 * TODO Remove this file in a future release.
 */
DEPRECATED typedef lms::Config ModuleConfig;

}  // namespace type
}  // namespace lms

#endif // LMS_TYPE_MODULE_CONFIG_H
