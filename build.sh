#!/bin/bash

# Exit if any errors were encountered, default bash behaviour is to continue.
set -e

# Create build directory if it does not exist
mkdir -p build

pushd build > /dev/null

COMPILER=clang++
COMPILER_FLAGS="-DDEBUG -std=c++17 -g -Wall -Werror"
LINKER_FLAGS="-framework SDL2 -framework SDL2_mixer -framework OpenGL -lfreetype"
INCLUDE_DIRECTORIES="-I/Users/jsanchez/Dropbox/Projects/Untitled/external/glad/include -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_mixer.framework/Versions/A/Headers -I/Users/jsanchez/Dropbox/Projects/Untitled/external/glm-0.9.9.6/glm-0.9.9.6 -I/usr/local/opt/freetype/include/freetype2"
WARNING_DISABLES="-Wno-writable-strings -Wno-char-subscripts -Wno-unused-value"

echo "------ Compiling ------"

$COMPILER ../main.cpp $INCLUDE_DIRECTORIES $COMPILER_FLAGS $WARNING_DISABLES $LINKER_FLAGS -o Untitled

popd > /dev/null
