#!/bin/sh

SOURCE_DIR="@CMAKE_SOURCE_DIR@"

pullAllRepos() {
	for MOD in $(find "$SOURCE_DIR/$1" -maxdepth 1 -mindepth 1 -type d -printf "%P\n")
	do
		cd "$SOURCE_DIR/$1/$MOD"
		echo $MOD
		git pull
	done
}

pullAllRepos "external/modules"
pullAllRepos "external/libraries"

