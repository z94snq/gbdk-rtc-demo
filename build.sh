#!/bin/bash

# Parse arguments
BUILD_TYPE="Release"
CLEAN=false

for arg in "$@"; do
    case $arg in
        --clean)
            CLEAN=true
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    rm -rf build
fi

# Build the project
mkdir -p build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
ninja
cd ..
