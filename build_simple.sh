#!/bin/bash

# Simple Windows build script
# This attempts to build a static executable if possible

echo "=== Simple Windows Build Attempt ==="

# Check for cross compiler
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "❌ MinGW cross-compiler not found"
    echo "Install with: sudo apt install mingw-w64 mingw-w64-tools"
    exit 1
fi

# Create simple build
mkdir -p build-simple
cd build-simple

echo "Attempting simple cross-compile..."

# Try to build without Qt first (won't work fully but let's see)
x86_64-w64-mingw32-g++ --version

echo ""
echo "❌ Cross-compilation needs Qt5 for MinGW"
echo ""
echo "📋 **SOLUTIONS FOR WINDOWS EXE:**"
echo ""
echo "🏆 **EASIEST: Native Windows Build**"
echo "   1. Use Windows machine with Visual Studio"
echo "   2. Install Qt5 (5.15.2)"
echo "   3. Follow BUILD_WINDOWS_GUIDE.md"
echo ""
echo "🤖 **AUTOMATED: GitHub Actions**"
echo "   1. Push to GitHub"
echo "   2. Add workflow in .github/workflows/"
echo "   3. Download built exe from Actions"
echo ""
echo "🐧 **LINUX: Install MXE**"
echo "   1. sudo apt install mxe-*"
echo "   2. Build Qt5 for MinGW (takes 1-2 hours)"
echo "   3. Cross-compile"
echo ""
echo "📦 **CURRENT OPTIONS:**"
echo "   - Linux binary: build/MedianXLOfflineTools"
echo "   - Use Wine to run on Windows"
echo "   - Build natively on Windows (recommended)"