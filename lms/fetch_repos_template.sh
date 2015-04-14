#!/bin/sh

SOURCE_DIR="@CMAKE_SOURCE_DIR@"

fetchAllRepos() {
    for MOD in $(find "$SOURCE_DIR/$1" -maxdepth 1 -mindepth 1 -type d -printf "%P\n")
    do
        cd "$SOURCE_DIR/$1/$MOD"
        echo $MOD
        git fetch
    done
}

fetchAllRepos "external/modules"
fetchAllRepos "external/libraries"

