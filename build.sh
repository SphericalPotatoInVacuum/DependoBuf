#!/bin/bash

# Name of the script
SCRIPT_NAME=$(basename $0)

# Define a function for displaying usage information
usage() {
    echo "Usage: $0 [-t] [-i] [-j N] [-h]"
    echo "    -t: Run tests after building"
    echo "    -i: Install after building"
    echo "    -j N: Use N jobs for make (default 1)"
    echo "    -h: Show this help message"
    exit 1
}

# Parse command line options
RUN_TESTS=0
INSTALL=0
JOBS=1
DEBUG=0
while getopts "tij:hd" opt; do
    case ${opt} in
        t)
            RUN_TESTS=1
            ;;
        i)
            INSTALL=1
            ;;
        j)
            JOBS=${OPTARG}
            ;;
        d)
            DEBUG=1
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND -1))

# Ensuring that submodules are initialized
echo "Ensuring that submodules are up to date. Running 'git submodule update --init --recursive'"
git submodule update --init --recursive

BUILD_TYPE="Release"

if [ $DEBUG -eq 1 ]; then
    BUILD_TYPE="Debug"
fi

# Build the project
echo "Building the project"
mkdir -p build
echo "Generating build files"
cmake -S . -B build -G "Ninja" -DZ3_BUILD_LIBZ3_SHARED=FALSE -DBUILD_SHARED_LIBS=FALSE -DCMAKE_BUILD_TYPE=$BUILD_TYPE
echo "Building the project"
cmake --build build --parallel $JOBS --config $BUILD_TYPE

# Conditionally run tests
if [ $RUN_TESTS -eq 1 ]; then
    echo "Running tests..."
    cd test
    ./dbufTests
    cd ..
fi

# Conditionally install the project system-wide
if [ $INSTALL -eq 1 ]; then
    echo "Installing the project system-wide..."
    sudo cmake --install build --config $BUILD_TYPE
fi

# Finish
echo "Done."
