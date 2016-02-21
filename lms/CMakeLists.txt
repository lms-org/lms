cmake_minimum_required(VERSION 2.8)
project(LightWeightModularSystem)

if(APPLE)
    # /bin suffix is needed on OS X
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include(CMakeData.txt)

if(NOT DEFINED LMS_STANDALONE)
    # LMS is standalone by default.
    # LMS_STANDALONE is set to FALSE in config repos.
    set(LMS_STANDALONE TRUE)
endif()

if(LMS_STANDALONE)
    set(LMS_DIR_CONFIGS "")
    set(LMS_DIR_MODULES "")
    set(LMS_DIR_EXTERNAL "")

    if((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic")
    endif()
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/lms")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/CMakeSharedHeaders.txt" "")
configure_file(include/lms/definitions.h.in
    include/lms/definitions.h
    ESCAPE_QUOTES)

#include pugixml headers (hpp-files)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(include)
include_directories(pugixml-1.7/src)
include_directories(tclap-1.2.1/include)

set (HEADERS
    "${CMAKE_CURRENT_BINARY_DIR}/include/lms/definitions.h"

    "include/lms/internal/framework.h"
    "include/lms/internal/signalhandler.h"
    "include/lms/extra/colors.h"
    "include/lms/internal/datamanager.h"
    "include/lms/internal/argumenthandler.h"
    "include/lms/logger.h"
    "include/lms/internal/executionmanager.h"
    "include/lms/module.h"
    "include/lms/internal/loader.h"
    "include/lms/lms_exports.h"
    "include/lms/messaging.h"
    "include/lms/serializable.h"
    "include/lms/internal/clock.h"
    "include/lms/deprecated.h"
    "include/lms/internal/xml_parser.h"
    "include/lms/config_observer.h"
    "include/lms/internal/module_wrapper.h"
    "include/lms/internal/profiler.h"
    "include/lms/config.h"
    "include/lms/data_channel.h"
    "include/lms/inheritance.h"
    "include/lms/internal/data_channel_internal.h"
    "include/lms/internal/runtime.h"
    "include/lms/execution_type.h"
    "include/lms/service.h"
    "include/lms/internal/service_wrapper.h"
    "include/lms/service_handle.h"
    "include/lms/type_result.h"
    "include/lms/any.h"
    "include/lms/internal/file_monitor.h"
    "include/lms/internal/dag.h"
    "include/lms/internal/debug_server.h"
    "include/lms/internal/watch_dog.h"
    "include/lms/internal/wrapper.h"

    "include/lms/endian.h"
    "include/lms/unit.h"
    "include/lms/interface.h"

    "include/lms/logging/level.h"
    "include/lms/logging/logger.h"
    "include/lms/logging/event.h"
    "include/lms/logging/sink.h"
    "include/lms/logging/filter.h"
    "include/lms/logging/context.h"
    "include/lms/logging/console_sink.h"
    "include/lms/logging/file_sink.h"
    "include/lms/logging/threshold_filter.h"
    "include/lms/logging/debug_server_sink.h"

    "include/lms/extra/type.h"
    "include/lms/extra/string.h"
    "include/lms/time.h"
    "include/lms/internal/backtrace_formatter.h"
    "include/lms/extra/os.h"
    "include/lms/internal/dot_exporter.h"
)

set (SOURCE
    "main/module.cpp"
    "main/internal/datamanager.cpp"
    "main/internal/executionmanager.cpp"
    "main/internal/framework.cpp"
    "main/internal/signalhandler.cpp"
    "main/internal/argumenthandler.cpp"
    "main/internal/profiler.cpp"
    "main/messaging.cpp"
    "main/internal/clock.cpp"
    "main/internal/xml_parser.cpp"
    "main/config_observer.cpp"
    "main/config.cpp"
    "main/internal/execution_type.cpp"
    "main/internal/module_wrapper.cpp"
    "main/service.cpp"
    "main/internal/service_wrapper.cpp"
    "main/internal/runtime.cpp"
    "main/internal/file_monitor.cpp"
    "main/internal/debug_server.cpp"
    "main/internal/watch_dog.cpp"
    "main/internal/wrapper.cpp"
    "main/internal/loader.cpp"
    "main/endian.cpp"
    "main/unit.cpp"

    "main/logging/level.cpp"
    "main/logging/logger.cpp"
    "main/logging/context.cpp"
    "main/logging/event.cpp"
    "main/logging/console_sink.cpp"
    "main/logging/file_sink.cpp"
    "main/logging/threshold_filter.cpp"
    "main/logging/debug_server_sink.cpp"

    "main/extra/type.cpp"
    "main/extra/string.cpp"
    "main/time.cpp"
    "main/extra/os.cpp"
    "main/internal/dot_exporter.cpp"
)

# Add system-specific source
if(UNIX)
    message(STATUS "Use Unix specific sources")
    set (SOURCE ${SOURCE}
        "main/internal/backtrace_formatter_unix.cpp"
        "main/internal/signalhandler_unix.cpp"
        "main/internal/file_monitor_unix.cpp"
    )
elseif(WIN32)
    message(STATUS "Use Win32 specific sources")
    set (SOURCE ${SOURCE}
        "main/loader_win.cpp"
        "main/internal/backtrace_formatter_win.cpp"
        "main/signalhandler_win.cpp"
        "main/extra/file_monitor_win.cpp"
    )
else()
    message(FATAL_ERROR "OS is probably not supported")
endif()

if(POLICY CMP0042)
    # Set new MACOSX_RPATH policy on OS X
    cmake_policy(SET CMP0042 NEW)
endif()

# LMS core library
add_library (lmscore SHARED
    ${HEADERS}
    ${SOURCE}
)

# Compile xml-parser pugixml
add_library(pugixml STATIC         
    "pugixml-1.7/src/pugixml.cpp"
    "pugixml-1.7/src/pugixml.hpp"
    "pugixml-1.7/src/pugiconfig.hpp"
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
