cmake_minimum_required(VERSION 2.6)

project ("LightWeightModularSystem")

#Braucht man das wirklich?
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake; ${CMAKE_MODULE_PATH}")

set (CMAKE_CXX_FLAGS_MODULES "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0")
set (CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")

#Gather all config files
file(GLOB_RECURSE ConfigFiles ${CMAKE_CURRENT_LIST_FILE} **/loadConfig.xml)
message("ConfigFiles: " ${ConfigFiles})

#TODO doesnt work yet!
#Adds copy commands to the core (core compiled is called lms)
foreach(ConfigFile ${ConfigFiles})
  add_custom_command(TARGET lms PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E
                         copy ${ConfigFile} "$<TARGET_FILE_DIR:lms>/..")
endforeach()


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
