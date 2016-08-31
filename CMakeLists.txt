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

#if(NOT DEFINED LMS_STANDALONE)
    # LMS is standalone by default.
    # LMS_STANDALONE is set to FALSE in config repos.
    #set(LMS_STANDALONE TRUE)
#endif()

#LMS is standalone
set(LMS_DIR_CONFIGS "")
set(LMS_DIR_MODULES "")
set(LMS_DIR_EXTERNAL "")

if((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic")
endif()

add_subdirectory(external)

#create definitions.h
configure_file(include/lms/definitions.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/include/lms/definitions.h"
    ESCAPE_QUOTES)

set (HEADERS
    "${CMAKE_CURRENT_BINARY_DIR}/include/lms/definitions.h"

    "include/lms/logger.h"
    "include/lms/module.h"
    "include/lms/lms_exports.h"
    "include/lms/messaging.h"
    "include/lms/serializable.h"
    "include/lms/deprecated.h"
    "include/lms/config.h"
    "include/lms/inheritance.h"
    "include/lms/service_handle.h"
    "include/lms/service.h"
    "include/lms/endian.h"
    "include/lms/unit.h"
    "include/lms/interface.h"
    "include/lms/life_cycle.h"
    "include/lms/type.h"
    "include/lms/time.h"
    "include/lms/datamanager.h"
    "include/lms/exception.h"

    "src/internal/framework.h"
    "src/internal/signalhandler.h"
    "src/internal/colors.h"
    "src/internal/argumenthandler.h"
    "src/internal/executionmanager.h"
    "src/internal/loader.h"
    "src/internal/xml_parser.h"
    "src/internal/profiler.h"
    "src/internal/file_monitor.h"
    "src/internal/dag.h"
    "src/internal/debug_server.h"
    "src/internal/watch_dog.h"
    "src/internal/string.h"
    "src/internal/backtrace_formatter.h"
    "src/internal/dot_exporter.h"
    "src/internal/module_channel_graph.h"
    "src/internal/viz.h"
    "src/internal/line_separated_reader.h"
    "src/internal/daemon.h"
    "src/internal/master.h"
    "src/internal/clock.h"
    "src/internal/os.h"
    "src/internal/debug_server_sink.h"

    "include/lms/logging/level.h"
    "include/lms/logging/logger.h"
    "include/lms/logging/event.h"
    "include/lms/logging/sink.h"
    "include/lms/logging/filter.h"
    "include/lms/logging/context.h"
    "include/lms/logging/console_sink.h"
    "include/lms/logging/file_sink.h"
    "include/lms/logging/threshold_filter.h"
)

set (SOURCE
    "src/module.cpp"
    "src/messaging.cpp"
    "src/config.cpp"
    "src/service.cpp"
    "src/endian.cpp"
    "src/unit.cpp"
    "src/type.cpp"
    "src/time.cpp"
    "src/datamanager.cpp"

    "src/internal/executionmanager.cpp"
    "src/internal/framework.cpp"
    "src/internal/signalhandler.cpp"
    "src/internal/argumenthandler.cpp"
    "src/internal/profiler.cpp"
    "src/internal/file_monitor.cpp"
    "src/internal/debug_server.cpp"
    "src/internal/watch_dog.cpp"
    "src/internal/loader.cpp"
    "src/internal/backtrace_formatter.cpp"
    "src/internal/loader.cpp"
    "src/internal/signalhandler.cpp"
    "src/internal/clock.cpp"
    "src/internal/xml_parser.cpp"
    "src/internal/string.cpp"
    "src/internal/os.cpp"
    "src/internal/dot_exporter.cpp"
    "src/internal/viz.cpp"
    "src/internal/line_separated_reader.cpp"
    "src/internal/daemon.cpp"
    "src/internal/master.cpp"
    "src/internal/debug_server_sink.cpp"

    "src/logging/level.cpp"
    "src/logging/logger.cpp"
    "src/logging/context.cpp"
    "src/logging/event.cpp"
    "src/logging/console_sink.cpp"
    "src/logging/file_sink.cpp"
    "src/logging/threshold_filter.cpp"
)

if(POLICY CMP0042)
    # Set new MACOSX_RPATH policy on OS X
    cmake_policy(SET CMP0042 NEW)
endif()

# LMS core library
add_library (lmscore SHARED
    ${HEADERS}
    ${SOURCE}
)

if(COVERAGE)
    message(STATUS "Enable test coverage")
    set_target_properties(lmscore PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -O0")
    set_target_properties(lmscore PROPERTIES LINK_FLAGS "-fprofile-arcs")
endif()

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
#lpm handles it
#if(APPLE)
    # /bin suffix is needed on OS X
#    set_target_properties(lms PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#else()
#    set_target_properties(lms PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
#endif()

if(BUILD_TESTS)
    enable_testing()
    include_directories(external/googletest/googletest/include)
    add_subdirectory(test)
endif()

add_custom_target(format clang-format -i ${HEADERS} ${SOURCE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
