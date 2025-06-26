#!/bin/bash

# FENgineLive Build Script for Linux/macOS
# Usage: ./build.sh [debug|release|relwithdebinfo]

set -e  # Exit on any error

echo "========================================"
echo "FENgineLive Build Script"
echo "========================================"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Set build type
BUILD_TYPE="Release"
if [ "$1" = "debug" ]; then
    BUILD_TYPE="Debug"
elif [ "$1" = "relwithdebinfo" ]; then
    BUILD_TYPE="RelWithDebInfo"
fi

echo "Building in $BUILD_TYPE mode..."

# Detect OS and set appropriate generator
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    GENERATOR="Unix Makefiles"
    echo "Detected macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    GENERATOR="Unix Makefiles"
    echo "Detected Linux"
else
    echo "Warning: Unknown OS type: $OSTYPE"
    GENERATOR="Unix Makefiles"
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G "$GENERATOR"
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo "Building project..."
# Use all available CPU cores
if command -v nproc &> /dev/null; then
    CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

cmake --build . --config $BUILD_TYPE --parallel $CORES
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Copy runtime dependencies
echo "Copying runtime dependencies..."
if [ -d "../external/lc0" ]; then
    mkdir -p lc0
    cp -r ../external/lc0/* lc0/ 2>/dev/null || true
fi

if [ -d "../external/python/fen_tracker" ]; then
    mkdir -p python
    cp -r ../external/python/fen_tracker/* python/ 2>/dev/null || true
fi

if [ -d "../external/python/sable" ]; then
    mkdir -p python/sable
    cp -r ../external/python/sable/* python/sable/ 2>/dev/null || true
fi

if [ -d "../assets" ]; then
    mkdir -p assets
    cp -r ../assets/* assets/ 2>/dev/null || true
fi

echo "========================================"
echo "Build completed successfully!"
echo "========================================"
echo ""
echo "Executable location: build/ChessGUI"
echo ""
echo "To run the application:"
echo "  cd build"
echo "  ./ChessGUI"
echo ""

# Make executable executable
chmod +x ChessGUI 2>/dev/null || true 