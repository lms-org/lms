#!/bin/sh

SOURCE_DIR="@CMAKE_SOURCE_DIR@"
BINARY_DIR="@CMAKE_BINARY_DIR@"

copyFile() {
    mkdir -p -v $(dirname "$BINARY_DIR/$1")
    cp -u -v "$SOURCE_DIR/$1" "$BINARY_DIR/$1"
}

for FILE in $(find "$SOURCE_DIR/configs" -name "*.xml" -printf "%P\n")
do
    copyFile "configs/$FILE"
done

for FILE in $(find $SOURCE_DIR -name "*.lconf" -printf "%P\n")
do
    copyFile $FILE
done

for FILE in $(find $SOURCE_DIR -name "loadConfig.xml" -printf "%P\n")
do
    copyFile $FILE
done
