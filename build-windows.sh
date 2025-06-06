#!/bin/bash

# X2Modern Windows Build Script
# This script helps build X2Modern for Windows using cross-compilation

echo "X2Modern Windows Cross-Compilation Setup"
echo "========================================"
echo

# Check if MinGW is installed
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "ERROR: MinGW cross-compiler not found"
    echo "Install with: sudo apt-get install mingw-w64"
    exit 1
fi

echo "✓ MinGW cross-compiler found"

# For a full Windows build, you would need:
echo
echo "To complete Windows build, you need:"
echo "1. Qt5 compiled for MinGW (cross-compilation)"
echo "2. Windows versions of dependencies"
echo
echo "Recommended approaches:"
echo "1. Use Qt Online Installer to get Qt5 for MinGW"
echo "2. Use MXE (M cross environment) for dependencies"
echo "3. Build on actual Windows system with Qt5 installed"
echo

# Create a simple demo to show cross-compilation works
echo "Creating simple Windows test executable..."
cat > test_windows.cpp << 'EOF'
#include <iostream>
#include <windows.h>

int main() {
    std::cout << "Hello from X2Modern Windows build!" << std::endl;
    std::cout << "This is a test executable." << std::endl;
    return 0;
}
EOF

# Compile test
echo "Compiling test Windows executable..."
x86_64-w64-mingw32-g++ -static -static-libgcc -static-libstdc++ -o test_windows.exe test_windows.cpp

if [ -f "test_windows.exe" ]; then
    echo "✓ Windows test executable created: test_windows.exe"
    ls -la test_windows.exe
    echo
    echo "This demonstrates that cross-compilation is working."
    echo "To build the full Qt application for Windows:"
    echo "1. Copy this project to a Windows machine with Qt5"
    echo "2. Run build-windows.bat"
    echo "3. Or use the CMakeLists-windows.txt with proper Qt5 setup"
else
    echo "✗ Failed to create test executable"
    exit 1
fi

echo
echo "Windows build setup complete!"
echo "Files created:"
echo "- CMakeLists-windows.txt (Windows-specific CMake config)"
echo "- build-windows.bat (Windows build script)"
echo "- test_windows.exe (Cross-compilation test)"
