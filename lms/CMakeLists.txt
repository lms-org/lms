#Do we really need it?
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake; ${CMAKE_MODULE_PATH}")

set (CMAKE_CXX_FLAGS_MODULES "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0")
set (CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")

#Gather all config files
file(GLOB_RECURSE ConfigFiles1 RELATIVE ${CMAKE_SOURCE_DIR}  external/loadConfig.xml)
file(GLOB_RECURSE ConfigFiles2 RELATIVE ${CMAKE_SOURCE_DIR}  configs/*.xml)
#TODO add arrays
set(ConfigFiles
        ${ConfigFiles1}
        ${ConfigFiles2})
#message("Path:" ${CMAKE_SOURCE_DIR} " ConfigFiles FIRST: " ${Configfiles})



#compile core
message("Compile core")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_CORE}")
#Add include path so it's available for all
include_directories(core/include)
add_subdirectory(core)

#compile modules
message("Compile modules")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_MODULES}")
add_subdirectory(external)
