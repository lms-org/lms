cmake_minimum_required(VERSION 2.6)

project ("LightWeightModularSystem")

#Do we really need it?
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake; ${CMAKE_MODULE_PATH}")

set (CMAKE_CXX_FLAGS_MODULES "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0")
set (CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")

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
