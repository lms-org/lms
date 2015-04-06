cmake_minimum_required(VERSION 2.8)
project(LightWeightModularSystem)

#Gather all loadConfigs (used to load modules)
file(GLOB_RECURSE loadConfigs RELATIVE ${CMAKE_CURRENT_LIST_DIR}  external/loadConfig.xml)
#copy all module-configs (used to set values in modules)
file(GLOB_RECURSE moduleConfigs RELATIVE ${CMAKE_CURRENT_LIST_DIR}  *.lconf)
#copy the framework-configs
file( GLOB_RECURSE frameworkConfigs RELATIVE ${CMAKE_CURRENT_LIST_DIR} configs/*.xml)

#create one list
set(ConfigFiles
        ${frameworkConfigs}
        ${loadConfigs}
        ${moduleConfigs})

# Set source and binary dir in update_configs.sh and other scripts
configure_file("update_configs_template.sh" "update_configs.sh" @ONLY)
configure_file("pull_repos_template.sh" "pull_repos.sh" @ONLY)

#compile core
message("Compile core")
#set flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")
#Add include path so it's available for all
include_directories(core/include)
add_subdirectory(core)

#compile modules
message("Compile modules")
add_subdirectory(external)
