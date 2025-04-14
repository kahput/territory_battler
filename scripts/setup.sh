#!/bin/bash

# Resolve the directory this script is in and move one level up to get the project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Function to build the project using CMake with C99
build() {
    echo "[INFO] Building project..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || exit 1
    cmake -DCMAKE_C_STANDARD=99 "$PROJECT_ROOT"
    make || exit 1
}

# Function to run the main executable
client() {
    local BIN_DIR="$BUILD_DIR/bin"

    # Run the found executable
    echo "[INFO] Running client..."
    "$BIN_DIR/client"
}

# Function to run the main executable
server() {
    local BIN_DIR="$BUILD_DIR/bin"

    # Run the found executable
    echo "[INFO] Running server..."
    "$BIN_DIR/server"
}

# Function to clean the build directory
clean() {
    echo "[INFO] Cleaning build directory..."
    rm -rf "$BUILD_DIR"
}

# Main logic
case "$1" in
    build)
        build
        ;;
    client)
        build
        client
        ;;
    server)
        build
        server
        ;;
    clean)
        clean
        ;;
    "")
        build && client
        ;;
    *)
        echo "Usage: $0 {build|client|server|clean}"
        ;;
esac

