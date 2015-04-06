#!/bin/sh

SOURCE_DIR="@CMAKE_SOURCE_DIR@"
BINARY_DIR="@CMAKE_BINARY_DIR@"

copyFile() {
	cp -u -v "$SOURCE_DIR/$1" "$BINARY_DIR/$1"
}

copyFile "configs/framework_conf.xml"

for FILE in $(find $SOURCE_DIR -name "*.lconf" -printf "%P\n")
do
	copyFile $FILE
done

for FILE in $(find $SOURCE_DIR -name "loadConfig.xml" -printf "%P\n")
do
	copyFile $FILE
done
