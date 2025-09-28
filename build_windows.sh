#!/bin/bash

# Build script for Windows executable using MinGW cross-compiler
# Usage: ./build_windows.sh

set -e  # Exit on error

echo "=== Building MedianXL Offline Tools for Windows ==="

# Check if MinGW is installed
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Error: MinGW not found. Please install with:"
    echo "sudo apt install mingw-w64 mingw-w64-tools"
    exit 1
fi

# Create Windows build directory
BUILD_DIR="build-windows"
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing Windows build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring CMake for Windows build..."

# Try to configure with different Qt5 options
if command -v qt5-qmake-x86_64-w64-mingw32 &> /dev/null; then
    # If cross-compiled Qt5 is available
    echo "Using cross-compiled Qt5..."
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake \
             -DCMAKE_BUILD_TYPE=Release \
             -DQt5_DIR=/usr/x86_64-w64-mingw32/lib/qt5/lib/cmake/Qt5
elif [ -d "/usr/x86_64-w64-mingw32" ]; then
    # Try generic cross-compile
    echo "Using basic MinGW cross-compile..."
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake \
             -DCMAKE_BUILD_TYPE=Release
else
    echo "Error: No suitable Qt5 cross-compile environment found."
    echo ""
    echo "Options to get Windows Qt5:"
    echo "1. Use MXE (M cross environment): https://mxe.cc/"
    echo "2. Download precompiled Qt5 for MinGW"
    echo "3. Build on native Windows with Visual Studio or MinGW"
    exit 1
fi

if [ $? -eq 0 ]; then
    echo "Building Windows executable..."
    make -j$(nproc)
    
    if [ -f "MedianXLOfflineTools.exe" ]; then
        echo "✅ Success! Windows executable created: $BUILD_DIR/MedianXLOfflineTools.exe"
        
        # Show file info
        echo ""
        echo "File information:"
        ls -la MedianXLOfflineTools.exe
        file MedianXLOfflineTools.exe
        
        # Copy to easy location
        cp MedianXLOfflineTools.exe ../MedianXLOfflineTools-Windows.exe
        echo "✅ Copied to: MedianXLOfflineTools-Windows.exe"
    else
        echo "❌ Build completed but executable not found"
        exit 1
    fi
else
    echo "❌ CMake configuration failed"
    echo ""
    echo "This likely means Qt5 for MinGW is not available."
    echo "You may need to:"
    echo "1. Install MXE (M cross environment)"
    echo "2. Or build on native Windows"
    exit 1
fi