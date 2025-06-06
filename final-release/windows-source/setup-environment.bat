@echo off
REM X2Modern Windows Installer Script
REM This script helps set up the Windows build environment

echo.
echo X2Modern Windows Environment Setup
echo ==================================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running with administrator privileges... ✓
) else (
    echo Warning: Not running as administrator
    echo Some installations may require admin rights
    echo.
)

REM Check for existing installations
echo Checking for existing tools...
echo.

REM Check Qt5
where qmake >nul 2>nul
if %errorlevel% == 0 (
    echo ✓ Qt5 found
    qmake -version
) else (
    echo ✗ Qt5 not found
    echo.
    echo Please download and install Qt5:
    echo https://download.qt.io/official_releases/qt/5.15/
    echo.
    echo Make sure to install these components:
    echo - Qt5 Core, Widgets, Network, SQL
    echo - Qt5 WebEngine and WebEngineWidgets
    echo - MinGW or MSVC compiler
    echo.
)

echo.

REM Check CMake
where cmake >nul 2>nul
if %errorlevel% == 0 (
    echo ✓ CMake found
    cmake --version | findstr "cmake version"
) else (
    echo ✗ CMake not found
    echo.
    echo Please download and install CMake:
    echo https://cmake.org/download/
    echo.
)

echo.

REM Check MinGW
where mingw32-make >nul 2>nul
if %errorlevel% == 0 (
    echo ✓ MinGW found
    mingw32-make --version | findstr "GNU Make"
) else (
    where nmake >nul 2>nul
    if %errorlevel% == 0 (
        echo ✓ MSVC build tools found
    ) else (
        echo ✗ No build tools found
        echo.
        echo Please install either:
        echo 1. MinGW (comes with Qt5)
        echo 2. Visual Studio with C++ tools
        echo.
    )
)

echo.
echo Environment Check Complete
echo ==========================
echo.

REM Summary
echo Summary:
if exist "%ProgramFiles%\CMake" echo ✓ CMake installed
if exist "C:\Qt" echo ✓ Qt likely installed
echo.

echo Next steps:
echo 1. Ensure all tools are in your PATH
echo 2. Run build-windows-release.bat to build
echo 3. Find executable in build-windows\release\
echo.

echo For detailed instructions, see WINDOWS_BUILD.md
echo.
pause
