X2Modern Windows Build Instructions
===================================

This document provides instructions for building X2Modern on Windows.

Prerequisites
-------------
1. Windows 10/11 (64-bit recommended)
2. Qt5 Framework (5.15.x recommended)
   - Download from: https://download.qt.io/official_releases/qt/5.15/
   - Required components:
     * Qt5 Core
     * Qt5 Widgets
     * Qt5 Network
     * Qt5 SQL
     * Qt5 WebEngine
     * Qt5 WebEngineWidgets
     * Qt5 Concurrent

3. CMake 3.16 or later
   - Download from: https://cmake.org/download/

4. Build tools (choose one):
   - MinGW-w64 (recommended for open source)
   - Visual Studio 2019/2022 with C++ tools
   - Qt Creator with bundled compiler

Quick Start
-----------
1. Install Qt5 for Windows
2. Add Qt5 bin directory to your PATH (e.g., C:\Qt\5.15.2\msvc2019_64\bin)
3. Install CMake and add to PATH
4. Open Command Prompt or PowerShell
5. Navigate to the X2Modern project directory
6. Run: build-windows-release.bat

Manual Build Steps
------------------
If the automated script doesn't work:

1. Open Command Prompt/PowerShell
2. cd to project directory
3. mkdir build-windows && cd build-windows
4. cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -f ..\CMakeLists-windows.txt ..
5. cmake --build . --config Release
6. mkdir release
7. copy bin\X2Modern.exe release\
8. copy ..\resources release\ /s
9. cd release
10. windeployqt X2Modern.exe --webenginewidgets --sql --concurrent

Alternative: Qt Creator
-----------------------
1. Open Qt Creator
2. File > Open File or Project
3. Select CMakeLists-windows.txt
4. Configure project with your Qt5 kit
5. Build > Build Project
6. Use windeployqt to package dependencies

Building with Visual Studio
----------------------------
1. Install Visual Studio with C++ tools
2. Install Qt5 for MSVC
3. Use cmake with Visual Studio generator:
   cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release ..
4. Build with: cmake --build . --config Release

Troubleshooting
---------------
Error: "Qt5 not found"
- Ensure qmake is in your PATH
- Set Qt5_DIR environment variable to Qt5 installation

Error: "WebEngine not found"
- Install Qt5 WebEngine component
- Some Qt installations exclude WebEngine by default

Error: "MinGW not found"
- Install MinGW-w64 or use Qt's bundled MinGW
- Add MinGW bin directory to PATH

Dependencies
------------
The Windows build requires these Qt5 modules:
- Qt5Core
- Qt5Widgets
- Qt5Network
- Qt5Sql
- Qt5WebEngine
- Qt5WebEngineWidgets
- Qt5Concurrent

Additional runtime requirements:
- Microsoft Visual C++ Redistributable (if using MSVC build)
- Windows 10 or later for full WebEngine support

Cross-compilation from Linux
-----------------------------
For advanced users, cross-compilation is possible using:
1. MXE (M cross environment): https://mxe.cc/
2. Qt5 compiled for MinGW
3. Wine for testing (optional)

See build-windows.sh for cross-compilation setup.

Output
------
Successful build produces:
- X2Modern.exe (main executable)
- All required Qt5 DLLs
- Resources directory
- Ready-to-distribute package

The final package can be zipped and distributed to other Windows systems
without requiring Qt5 installation on the target machine.

Support
-------
For build issues, check:
1. Qt5 installation is complete
2. PATH environment variables
3. CMake version compatibility
4. Compiler installation

For Windows-specific WebEngine issues, ensure:
- Windows 10/11 (WebEngine requires modern Windows)
- Hardware acceleration support
- Proper display drivers
