@echo off
REM X2Modern Windows GUI Build Script
REM Builds the complete Qt5 WebEngine version

echo Building X2Modern Qt5 GUI Version...
echo ====================================

REM Check if Qt5 is available
where qmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Qt5 not found in PATH
    echo.
    echo Please install Qt5 and add to PATH:
    echo Example: C:\Qt\5.15.2\msvc2019_64\bin
    echo.
    echo Download from: https://www.qt.io/download-qt-installer
    pause
    exit /b 1
)

REM Display Qt version
echo Qt version detected:
qmake -version
echo.

REM Check CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found
    echo Please install CMake from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring Qt5 build...
cmake -DCMAKE_BUILD_TYPE=Release ..
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed
    echo.
    echo Common solutions:
    echo 1. Ensure Qt5 is properly installed
    echo 2. Add Qt5 bin directory to PATH
    echo 3. Install Qt5 WebEngine module
    echo.
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building...
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed
    echo Check error messages above for details
    pause
    exit /b 1
)

echo.
echo ========================================
echo ✓ Qt5 GUI build completed successfully!
echo ========================================
echo.

REM Find the executable
if exist "Release\X2Modern.exe" (
    echo Executable: build\Release\X2Modern.exe
    set EXE_PATH=Release\X2Modern.exe
) else if exist "X2Modern.exe" (
    echo Executable: build\X2Modern.exe
    set EXE_PATH=X2Modern.exe
) else (
    echo Warning: Could not locate X2Modern.exe
    echo Check the build output above
    set EXE_PATH=
)

if not "%EXE_PATH%"=="" (
    echo.
    echo Deploying Qt dependencies...
    windeployqt "%EXE_PATH%"
    
    echo.
    echo Ready to run! Features include:
    echo • Qt5 WebEngine GUI interface
    echo • Twitter bot automation
    echo • Multi-account management
    echo • Real-time monitoring
    echo • AI integration support
)

echo.
pause
