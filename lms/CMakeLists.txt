#add version if you use that cmake as your
#cmake_minimum_required(VERSION 2.8)
project(LightWeightModularSystem)
#create one list
set(ConfigFiles
        ${frameworkConfigs}
        ${loadConfigs}
        ${moduleConfigs})

# Set source and binary dir in update_configs.sh and other scripts
#configure_file("update_configs_template.sh" "update_configs.sh" @ONLY)
#configure_file("pull_repos_template.sh" "pull_repos.sh" @ONLY)
#configure_file("fetch_repos_template.sh" "fetch_repos.sh" @ONLY)

#compile core
message("Compile core")
#set flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x -Wall -Wextra -pedantic -O0 ")
#Add include path so it's available for all
add_subdirectory(core)
