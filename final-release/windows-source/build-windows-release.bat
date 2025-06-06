@echo off
REM X2Modern Windows Release Builder
REM This script creates a complete Windows release package

echo.
echo X2Modern Windows Release Builder
echo ===============================
echo.

REM Check for Qt5
where qmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Qt5 not found in PATH
    echo.
    echo Please install Qt5 for Windows and add it to your PATH
    echo Download from: https://download.qt.io/official_releases/qt/5.15/
    echo.
    echo Required Qt5 components:
    echo - Qt5 Core
    echo - Qt5 Widgets  
    echo - Qt5 Network
    echo - Qt5 SQL
    echo - Qt5 WebEngine
    echo - Qt5 WebEngineWidgets
    echo.
    pause
    exit /b 1
)

echo Qt5 found. Checking version...
qmake -version
echo.

REM Create build directory
if exist "build-windows" rmdir /s /q "build-windows"
mkdir build-windows
cd build-windows

REM Configure build
echo Configuring Windows build...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -f ..\CMakeLists-windows.txt ..
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    echo.
    echo Make sure you have:
    echo 1. Qt5 properly installed
    echo 2. CMake in your PATH
    echo 3. MinGW or Visual Studio compiler
    echo.
    pause
    exit /b 1
)

REM Build
echo.
echo Building X2Modern...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

REM Create release package
echo.
echo Creating Windows release package...
if not exist "release" mkdir release
copy bin\X2Modern.exe release\
copy ..\resources release\ /s /q

REM Deploy Qt dependencies
echo.
echo Deploying Qt dependencies...
cd release
windeployqt X2Modern.exe --webenginewidgets --sql --concurrent
cd ..

REM Copy additional files
copy ..\README.md release\
copy ..\RELEASE_INFO.txt release\

echo.
echo ========================================
echo Windows build completed successfully!
echo.
echo Release package location: build-windows\release\
echo Executable: build-windows\release\X2Modern.exe
echo.
echo The release package includes:
echo - X2Modern.exe (main executable)
echo - All required Qt5 DLLs
echo - Resources directory
echo - Documentation files
echo.
echo Ready for distribution!
echo ========================================
echo.
pause
