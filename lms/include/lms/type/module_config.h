#ifndef LMS_TYPE_MODULE_CONFIG_H
#define LMS_TYPE_MODULE_CONFIG_H

#include "lms/module_config.h"
#include "lms/deprecated.h"

namespace lms {
namespace type {

/**
 * @brief Moved lms::type::ModuleConfig to lms::ModuleConfig
 *
 * This header and typedefs makes the change backwards compatible.
 *
 * TODO Remove this file in a future release.
 */
DEPRECATED typedef lms::ModuleConfig ModuleConfig;

}  // namespace type
}  // namespace lms

#endif // LMS_TYPE_MODULE_CONFIG_H
