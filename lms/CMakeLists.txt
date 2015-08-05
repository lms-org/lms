project(LightWeightModularSystem)

message("Compile core")

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

#TODO Rename, not public
set (PUBLIC_HEADERS
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

        "include/lms/type/module_config.h"
        "include/lms/type/framework_info.h"

        "include/lms/logging/log_level.h"
        "include/lms/logging/logger.h"
        "include/lms/logging/log_message.h"
        "include/lms/logging/sink.h"
        "include/lms/logging/logging_filter.h"
        "include/lms/logging/root_logger.h"
        "include/lms/logging/child_logger.h"
        "include/lms/logging/console_sink.h"
        "include/lms/logging/file_sink.h"
        "include/lms/logging/multi_sink.h"
        "include/lms/logging/prefix_and_level_filter.h"

        "include/lms/extra/type.h"
        "include/lms/extra/string.h"
        "include/lms/extra/time.h"
        "include/lms/extra/file_monitor.h"
        "include/lms/extra/backtrace_formatter.h"
        "include/lms/extra/os.h"
)

set (SOURCE
        "main/main.cpp"
        "main/loader.cpp"
        "main/module.cpp"
        "main/datamanager.cpp"
        "main/executionmanager.cpp"
        "main/framework.cpp"
        "main/signalhandler.cpp"
        "main/argumenthandler.cpp"
        "main/type/module_config.cpp"
        "main/type/framework_info.cpp"
        "main/messaging.cpp"
        "main/clock.cpp"
        "main/xml_parser.cpp"
        "main/config_observer.cpp"

        "main/logging/log_level.cpp"
        "main/logging/logger.cpp"
        "main/logging/root_logger.cpp"
        "main/logging/child_logger.cpp"
        "main/logging/log_message.cpp"
        "main/logging/console_sink.cpp"
        "main/logging/file_sink.cpp"
        "main/logging/multi_sink.cpp"
        "main/logging/prefix_and_level_filter.cpp"

        "main/extra/type.cpp"
        "main/extra/string.cpp"
        "main/extra/time.cpp"
        "main/extra/file_monitor.cpp"
        "main/extra/os.cpp"
)
#Add system-specific source
if(UNIX)
    message("OS is UNIX!")
    set (SOURCE ${SOURCE}
        "main/loader_unix.cpp"
        "main/framework_unix.cpp"
        "main/extra/backtrace_formatter_unix.cpp"
        "main/signalhandler_unix.cpp"
        "main/extra/file_monitor_unix.cpp"
    )
elseif(WIN32)
    message("OS is WIN32!")
    set (SOURCE ${SOURCE}
        "main/loader_win.cpp"
        "main/framework_win.cpp"
        "main/extra/backtrace_formatter_win.cpp"
        "main/signalhandler_win.cpp"
        "main/extra/file_monitor_win.cpp"
    )
else()
    message("NO OS found!")
endif()

add_executable (lms
        ${PUBLIC_HEADERS}
        ${SOURCE}
)

set_target_properties(lms PROPERTIES ENABLE_EXPORTS 1)

#Compile xml-parser pugixml
add_library(pugixml STATIC         
	"xml/pugixml-1.5/src/pugixml.cpp"
	"xml/pugixml-1.5/src/pugixml.hpp"
	"xml/pugixml-1.5/src/pugiconfig.hpp"
)

#link libraries
#link_libraries(pugixml)
target_link_libraries(lms PRIVATE pugixml)

#System specific source
#http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
if(UNIX)
    target_link_libraries(lms PRIVATE dl pthread)
elseif(WIN32)
else()
    message("NO OS found!")
endif()


#Adds copy commands for configs to the core (core compiled is called lms)
foreach(ConfigFile ${ConfigFiles})
    add_custom_command(TARGET lms PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E
                            copy "${CMAKE_SOURCE_DIR}/${ConfigFile}" "$<TARGET_FILE_DIR:lms>/../${ConfigFile}"
    )
endforeach()
