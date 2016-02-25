#ifndef LMS_INTERFACE_H
#define LMS_INTERFACE_H

#include "lms/definitions.h"

#define LMS_EXPORT_BEGIN extern "C" { \
uint32_t lms_version() { \
    return LMS_VERSION_CODE; \
}

#define LMS_EXPORT_MODULE(CLASS) \
lms::LifeCycle* lms_module_ ## CLASS () { \
    return new CLASS; \
}

#define LMS_EXPORT_MODULE_NS(NAMESPACE, CLASS) \
lms::LifeCycle* lms_module_ ## CLASS () { \
    return new NAMESPACE ## :: ## CLASS; \
}

#define LMS_EXPORT_SERVICE(CLASS) \
lms::LifeCycle* lms_service_ ## CLASS () { \
    return new CLASS; \
}

#define LMS_EXPORT_SERVICE_NS(NAMESPACE, CLASS) \
lms::LifeCycle* lms_service_ ## CLASS () { \
    return new NAMESPACE ## :: ## CLASS; \
}

#define LMS_EXPORT_END }

// DEPRECATED
#define LMS_MODULE_INTERFACE(CLASS) \
LMS_EXPORT_BEGIN \
LMS_EXPORT_MODULE(CLASS) \
LMS_EXPORT_END

// DEPRECATED
#define LMS_SERVICE_INTERFACE(CLASS) \
LMS_EXPORT_BEGIN \
LMS_EXPORT_SERVICE(CLASS) \
LMS_EXPORT_END

#endif // LMS_INTERFACE_H
