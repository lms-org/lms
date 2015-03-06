#include shared headers
include_directories(include)
#include pugixml headers (hpp-files)
include_directories(xml/pugixml-1.5/src)

#TODO Rename, not public
set (PUBLIC_HEADERS
        "include/core/configurationloader.h"
        "include/core/framework.h"
        "include/core/signalhandler.h"
        "include/core/extra/colors.h"
        "include/core/datamanager.h"
        "include/core/argumentparser.h"
        "include/core/logger.h"
        "include/core/executionmanager.h"
        "include/core/module.h"
        "include/core/loader.h"
        "include/core/type/static_image.h"
        "include/core/lms_exports.h"
        "include/core/type/module_config.h"
#private
        "main/backtrace_formatter.h"
)

set (SOURCE
        "main/main.cpp"
        "main/loader.cpp"
        "main/module.cpp"
        "main/datamanager.cpp"
        "main/configurationloader.cpp"
        "main/executionmanager.cpp"
        "main/visualizer.cpp"
        "main/framework.cpp"
        "main/signalhandler.cpp"
        "main/argumentparser.cpp"
        "main/logger.cpp"
        "main/backtrace_formatter.cpp"
        "main/type/static_image.cpp"
        "main/type/module_config.cpp"
)
#Add system-specific source
if(UNIX)
    message("OS is UNIX!")
    set (SOURCE ${SOURCE}
        "main/loader_unix.cpp"
        "main/framework_unix.cpp"
        "main/backtrace_formatter_unix.cpp"
        "main/signalhandler_unix.cpp"
    )
elseif(WIN32)
    message("OS is WIN32!")
    set (SOURCE ${SOURCE}
        "main/loader_win.cpp"
        "main/framework_win.cpp"
        "main/backtrace_formatter_win.cpp"
        "main/signalhandler_win.cpp"
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
    target_link_libraries(lms PRIVATE "dl")
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
