#include shared headers
include_directories(include)
#include pugixml headers (hpp-files)
include_directories(xml/pugixml-1.5/src)

#Gather all config files
file(GLOB_RECURSE ConfigFiles  ../external/loadConfig.xml)
message("ConfigFiles: " ${ConfigFiles})

#Adds copy commands to the core (core compiled is called lms)
#TODO doesnt work yet!
foreach(ConfigFile ${ConfigFiles})
    add_custom_command(TARGET lms PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E
                            copy ${ConfigFile} $<TARGET_FILE_DIR:lms>
    )
endforeach()

#Set public headers so the ide finds it
set (PUBLIC_HEADERS
        "include/core/configfile.h"
        "include/core/configurationmanager.h"
        "include/core/framework.h"
        "include/core/handle.h"
        "include/core/signalhandler.h"
        "include/core/extra/colors.h"
        "include/core/datamanager.h"
        "include/core/argumentparser.h"
        "include/core/logger.h"
        "include/core/executionmanager.h"
)
#Compile config file, may be moved to add_executable or removed as we might dont need it anymore
add_library(configfile STATIC
        ${PUBLIC_HEADERS}
        "configfile/configfile.cpp"
)
#Compile xml-parser pugixml
add_library(pugixml STATIC         
	"xml/pugixml-1.5/src/pugixml.cpp"
	"xml/pugixml-1.5/src/pugixml.hpp"
	"xml/pugixml-1.5/src/pugiconfig.hpp"
)

#link libraries
link_libraries("dl" configfile pugixml)

set (SOURCE
        "main/main.cpp"
        "main/loader.cpp"
        "main/shared_base.cpp"
        "main/datamanager.cpp"
        "main/configurationmanager.cpp"
        "main/executionmanager.cpp"
        "main/visualizer.cpp"
        "main/handle.cpp"
        "main/framework.cpp"
        "main/signalhandler.cpp"
        "main/argumentparser.cpp"
        "main/logger.cpp"
)

add_executable (lms
        ${PUBLIC_HEADERS}
        ${SOURCE}
)

