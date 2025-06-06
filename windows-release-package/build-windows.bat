@echo off
echo Building X2Modern for Windows...
echo.

REM Check if Qt5 is available
where qmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Qt5 not found in PATH
    echo Please ensure Qt5 is installed and qmake is in your PATH
    echo Example: C:\Qt\5.15.2\msvc2019_64\bin
    pause
    exit /b 1
)

REM Display Qt version
echo Qt version:
qmake -version
echo.

REM Create build directory
if not exist "build-windows" mkdir build-windows
cd build-windows

REM Configure with CMake
echo Configuring build...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -f ..\CMakeLists-windows.txt ..
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

REM Build the project
echo Building...
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable: build-windows\bin\X2Modern.exe
echo.

REM Copy Qt DLLs (this is a simplified approach)
echo Copying Qt dependencies...
windeployqt bin\X2Modern.exe

echo.
echo Windows build ready in: build-windows\bin\
pause
