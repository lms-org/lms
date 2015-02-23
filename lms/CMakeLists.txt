cmake_minimum_required(VERSION 2.6)

project ("LightWeightModularSystem")

#Der ist noch falsch, für was bracht man den?
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake; ${CMAKE_MODULE_PATH}")


set (CMAKE_CXX_FLAGS_MODULES "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0")
set (CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_CORE}")
message("Compile core")
add_subdirectory(core)

#Nicht sicher was das tut
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_MODULES}")
message("Compile modules")
add_subdirectory(external)
