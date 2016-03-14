cmake_minimum_required(VERSION 2.8)
project(LMS C CXX)

option(BUILD_TESTS "Build tests" OFF)
option(COVERAGE "Enable test coverage" OFF)

find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(include)
include_directories(external/pugixml-1.7/src)
include_directories(external/tclap-1.2.1/include)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(TestCpp11)

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
        if(COVERAGE)
            message(STATUS "Enable test coverage")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -coverage -O0")
        endif()
    endif()
endif()

add_subdirectory(external)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/lms")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/CMakeSharedHeaders.txt" "")
configure_file(include/lms/definitions.h.in
    include/lms/definitions.h
    ESCAPE_QUOTES)

set (HEADERS
    "${CMAKE_CURRENT_BINARY_DIR}/include/lms/definitions.h"

    "include/lms/internal/framework.h"
    "include/lms/internal/signalhandler.h"
    "include/lms/internal/colors.h"
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
    "include/lms/life_cycle.h"

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

    "include/lms/type.h"
    "include/lms/internal/string.h"
    "include/lms/time.h"
    "include/lms/internal/backtrace_formatter.h"
    "include/lms/internal/os.h"
    "include/lms/internal/dot_exporter.h"
)

set (SOURCE
    "src/module.cpp"
    "src/internal/datamanager.cpp"
    "src/internal/executionmanager.cpp"
    "src/internal/framework.cpp"
    "src/internal/signalhandler.cpp"
    "src/internal/argumenthandler.cpp"
    "src/internal/profiler.cpp"
    "src/messaging.cpp"
    "src/internal/clock.cpp"
    "src/internal/xml_parser.cpp"
    "src/config.cpp"
    "src/internal/execution_type.cpp"
    "src/internal/module_wrapper.cpp"
    "src/service.cpp"
    "src/internal/service_wrapper.cpp"
    "src/internal/runtime.cpp"
    "src/internal/file_monitor.cpp"
    "src/internal/debug_server.cpp"
    "src/internal/watch_dog.cpp"
    "src/internal/wrapper.cpp"
    "src/internal/loader.cpp"
    "src/endian.cpp"
    "src/unit.cpp"

    "src/logging/level.cpp"
    "src/logging/logger.cpp"
    "src/logging/context.cpp"
    "src/logging/event.cpp"
    "src/logging/console_sink.cpp"
    "src/logging/file_sink.cpp"
    "src/logging/threshold_filter.cpp"
    "src/logging/debug_server_sink.cpp"

    "src/type.cpp"
    "src/internal/string.cpp"
    "src/time.cpp"
    "src/internal/os.cpp"
    "src/internal/dot_exporter.cpp"
)

# Add system-specific source
if(UNIX)
    message(STATUS "Use Unix specific sources")
    set (SOURCE ${SOURCE}
        "src/internal/backtrace_formatter_unix.cpp"
        "src/internal/signalhandler_unix.cpp"
        "src/internal/file_monitor_unix.cpp"
    )
elseif(WIN32)
    message(STATUS "Use Win32 specific sources")
    set (SOURCE ${SOURCE}
        "src/loader_win.cpp"
        "src/internal/backtrace_formatter_win.cpp"
        "src/signalhandler_win.cpp"
        "src/extra/file_monitor_win.cpp"
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

target_link_libraries(lmscore PRIVATE pugixml)

# System specific link
# http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
if(UNIX)
    target_link_libraries(lmscore PRIVATE dl)
endif()
target_link_libraries(lmscore PRIVATE ${CMAKE_THREAD_LIBS_INIT})

# LMS executable
add_executable(lms
    src/main.cpp
)
target_link_libraries(lms PRIVATE lmscore)
if(APPLE)
    # /bin suffix is needed on OS X
    set_target_properties(lms PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else()
    set_target_properties(lms PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

if(BUILD_TESTS)
    enable_testing()
    include_directories(external/googletest/googletest/include)
    add_subdirectory(test)
endif()
