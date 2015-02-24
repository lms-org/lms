cmake_minimum_required(VERSION 2.6)

project ("LightWeightModularSystem")

set (CMAKE_CXX_FLAGS_MODULES "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0")
set (CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")

#Gather all config files
file(GLOB_RECURSE ConfigFiles **/loadConfig.xml)
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
