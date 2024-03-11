#!/bin/bash

# Define a function for displaying usage information
usage() {
    echo "Usage: $0 [init|format|build|lint|test|install|all]" \
         "\n  init:    Initialize the repository by fetching submodules." \
         "\n  format:  Run clang-format on all C++ files in the repository." \
         "\n  build:   Build the project. Options: -d|--debug, -c|--ccache." \
         "\n  lint:    Run clang-tidy on all C++ files in the repository." \
         "\n  test:    Run the unit tests." \
         "\n  all:     Run all of the above." \
         "\n  install: Install the project."
}

do_init() {
    echo "Ensuring that submodules are up to date."
    git submodule update --init --recursive || { echo "Failed to initialize submodules."; return 1; }
}

get_cpp_files() {
    find lib/core/ test/ -type f \( -name '*.cc' -o -name '*.h' \) -print0
}

get_cc_files() {
    find lib/core/ test/ -type f -name '*.cc' -print0
}

do_format() {
    echo "Running clang-format."
    get_cpp_files | xargs -0 clang-format-16 -style='file:.clang-format' -i || { echo "clang-format failed."; return 1; }
}

do_build() {
    echo "Building project."
    export CC=clang-16
    export CXX=clang++-16

    local debug=false
    local optimize=false
    local ccache=false
    local ccache_flags=""

    while [[ $# -gt 0 ]]; do
        case "$1" in
            -d | --debug)
                debug=true
                ;;
            -c | --ccache)
                ccache=true
                ;;
            *)
                echo "Unknown option: $1"
                return 1
                ;;
        esac
        shift
    done

    local build_type=$([ "$debug" = true ] && echo "Debug" || echo "Release")
    [ "$ccache" = true ] && ccache_flags="-DCMAKE_C_COMPILER_LAUNCHER=ccache \
                                          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"

    mkdir -p build
    cmake -S . -B build -G "Ninja" \
          -DZ3_BUILD_LIBZ3_SHARED=FALSE \
          -DBUILD_SHARED_LIBS=FALSE \
          -DCMAKE_BUILD_TYPE="$build_type" \
          $ccache_flags || { echo "CMake configuration failed."; return 1; }
    cmake --build build --config "$build_type" \
    || { echo "Build failed."; return 1; }
}

do_lint() {
    echo "Running clang-tidy."
    get_cc_files | xargs -0 clang-tidy-16 --config-file=.clang-tidy -p ./build \
    || { echo "clang-tidy found errors or warnings. Please fix before committing."; return 1; }
}

do_test() {
    echo "Running unit tests."
    if ! (cd build/test && ./dbufTests); then
        echo "Tests failed."
        return 1
    fi
}

do_install() {
    echo "Installing project."
    sudo cmake --install build || { echo "Installation failed."; return 1; }
}

do_all() {
    do_init || return 1
    do_format || return 1
    shift
    do_build "$@" || return 1
    do_lint || return 1
    do_test || return 1
}

if [[ $# -eq 0 ]]; then
    do_all
else
    case "$1" in
        init)
            do_init
            ;;
        format)
            do_format
            ;;
        build)
            shift
            do_build "$@"
            ;;
        lint)
            do_lint
            ;;
        test)
            do_test
            ;;
        install)
            do_install
            ;;
        all)
            do_all
            ;;
        *)
            echo "Unknown command: $1"
            usage
            ;;
    esac
fi
