#!/bin/bash
# X2Modern Twitter Bot - Startup Script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/X2Modern"

# Check if executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: X2Modern executable not found in $SCRIPT_DIR"
    exit 1
fi

# Check for Qt5 dependencies
if ! ldconfig -p | grep -q "libQt5Core"; then
    echo "Warning: Qt5 libraries may not be installed"
    echo "Please install Qt5 development libraries:"
    echo "  Ubuntu/Debian: sudo apt install qtbase5-dev qtwebengine5-dev"
    echo "  CentOS/RHEL: sudo yum install qt5-qtbase-devel qt5-qtwebengine-devel"
fi

# Set environment variables for better compatibility
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_SCALE_FACTOR=1

# Check if we have a display
if [ -z "$DISPLAY" ] && [ "$QT_QPA_PLATFORM" != "offscreen" ]; then
    echo "No display detected. Running in offscreen mode..."
    export QT_QPA_PLATFORM=offscreen
fi

echo "Starting X2Modern Twitter Bot..."
echo "Executable: $EXECUTABLE"
echo "Platform: $QT_QPA_PLATFORM"
echo "Working directory: $SCRIPT_DIR"
echo "----------------------------------------"

# Run the application
"$EXECUTABLE" "$@"
