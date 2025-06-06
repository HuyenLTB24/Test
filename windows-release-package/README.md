X2Modern for Windows - Release Package
=====================================

This package contains everything needed to build X2Modern on Windows.

Contents:
---------
src/                     - Source code files (.cpp)
include/                 - Header files (.h)
resources/               - Application resources (styles, etc.)
CMakeLists-windows.txt   - Windows-specific CMake configuration
build-windows.bat        - Simple build script
build-windows-release.bat - Complete release build script
WINDOWS_BUILD.md         - Detailed build instructions
README.md               - This file

Quick Start:
------------
1. Install Qt5 for Windows (5.15.x recommended)
2. Install CMake
3. Run: build-windows-release.bat
4. Find your executable in: build-windows\release\X2Modern.exe

Requirements:
-------------
- Windows 10/11 (64-bit)
- Qt5 with WebEngine support
- CMake 3.16+
- MinGW or Visual Studio compiler

For detailed instructions, see WINDOWS_BUILD.md

Features:
---------
- Twitter Bot GUI application
- Qt5 WebEngine integration
- Database management
- API integration (Gemini/ChatGPT)
- Comprehensive logging
- Account management

The built application will be self-contained with all Qt5 dependencies
included, making it ready for distribution.

Build Output:
-------------
After successful build:
- X2Modern.exe (main application)
- Qt5 DLLs (automatically included)
- Resources directory
- Complete Windows installer-ready package

For support and issues, refer to WINDOWS_BUILD.md
