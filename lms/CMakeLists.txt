project(LightWeightModularSystem)

if(APPLE)
    # /bin suffix is needed on OS X
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

#include pugixml headers (hpp-files)
include_directories(include)
include_directories(xml/pugixml-1.5/src)
include_directories(tclap-1.2.1/include)

set (HEADERS
    "include/lms/framework.h"
    "include/lms/signalhandler.h"
    "include/lms/extra/colors.h"
    "include/lms/datamanager.h"
    "include/lms/argumenthandler.h"
    "include/lms/logger.h"
    "include/lms/executionmanager.h"
    "include/lms/module.h"
    "include/lms/loader.h"
    "include/lms/lms_exports.h"
    "include/lms/messaging.h"
    "include/lms/serializable.h"
    "include/lms/clock.h"
    "include/lms/deprecated.h"
    "include/lms/xml_parser.h"
    "include/lms/config_observer.h"
    "include/lms/module_wrapper.h"
    "include/lms/profiler.h"
    "include/lms/module_config.h"

    "include/lms/type/module_config.h"

    "include/lms/logging/level.h"
    "include/lms/logging/logger.h"
    "include/lms/logging/event.h"
    "include/lms/logging/sink.h"
    "include/lms/logging/filter.h"
    "include/lms/logging/context.h"
    "include/lms/logging/console_sink.h"
    "include/lms/logging/file_sink.h"
    "include/lms/logging/threshold_filter.h"

    "include/lms/extra/type.h"
    "include/lms/extra/string.h"
    "include/lms/extra/time.h"
    "include/lms/extra/file_monitor.h"
    "include/lms/extra/backtrace_formatter.h"
    "include/lms/extra/os.h"
)

set (SOURCE
    "main/loader.cpp"
    "main/module.cpp"
    "main/datamanager.cpp"
    "main/executionmanager.cpp"
    "main/framework.cpp"
    "main/signalhandler.cpp"
    "main/argumenthandler.cpp"
    "main/profiler.cpp"
    "main/messaging.cpp"
    "main/clock.cpp"
    "main/xml_parser.cpp"
    "main/config_observer.cpp"
    "main/module_config.cpp"

    "main/logging/level.cpp"
    "main/logging/logger.cpp"
    "main/logging/context.cpp"
    "main/logging/event.cpp"
    "main/logging/console_sink.cpp"
    "main/logging/file_sink.cpp"
    "main/logging/threshold_filter.cpp"

    "main/extra/type.cpp"
    "main/extra/string.cpp"
    "main/extra/time.cpp"
    "main/extra/file_monitor.cpp"
    "main/extra/os.cpp"
)

# Add system-specific source
if(UNIX)
    message(STATUS "Use Unix specific sources")
    set (SOURCE ${SOURCE}
        "main/loader_unix.cpp"
        "main/framework_unix.cpp"
        "main/extra/backtrace_formatter_unix.cpp"
        "main/signalhandler_unix.cpp"
        "main/extra/file_monitor_unix.cpp"
    )
elseif(WIN32)
    message(STATUS "Use Win32 specific sources")
    set (SOURCE ${SOURCE}
        "main/loader_win.cpp"
        "main/framework_win.cpp"
        "main/extra/backtrace_formatter_win.cpp"
        "main/signalhandler_win.cpp"
        "main/extra/file_monitor_win.cpp"
    )
else()
    message(FATAL_ERROR "OS is probably not supported")
endif()

# LMS core library
add_library (lmscore SHARED
    ${HEADERS}
    ${SOURCE}
)

# Compile xml-parser pugixml
add_library(pugixml STATIC         
    "xml/pugixml-1.5/src/pugixml.cpp"
    "xml/pugixml-1.5/src/pugixml.hpp"
    "xml/pugixml-1.5/src/pugiconfig.hpp"
)
set_target_properties(pugixml PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_link_libraries(lmscore PRIVATE pugixml)

# System specific link
# http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
if(UNIX)
    target_link_libraries(lmscore PRIVATE dl pthread)
endif()

# LMS executable
add_executable(lms
    main/main.cpp
)
target_link_libraries(lms PRIVATE lmscore)
