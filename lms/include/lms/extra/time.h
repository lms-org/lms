#ifndef LMS_EXTRA_TIME_H
#define LMS_EXTRA_TIME_H

#include "../time.h"
#include "../deprecated.h"

namespace lms {
namespace extra {

DEPRECATED
typedef lms::Time PrecisionTime;

DEPRECATED
const auto& currentTimeString = lms::currentTimeString;

}  // namespace extra
}  // namespace lms

#endif  // LMS_EXTRA_TIME_H
