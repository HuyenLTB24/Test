#!/bin/bash

# X2 Modern C++ Build Script
# This script builds the C++ Twitter bot application using CMake and Qt6

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== X2 Modern C++ Build Script ===${NC}"
echo

# Check if Qt6 is available
if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo -e "${RED}Error: Qt6 not found. Please install Qt6 development packages.${NC}"
    echo "On Ubuntu/Debian: sudo apt install qt6-base-dev qt6-webengine-dev libqt6sql6-sqlite"
    echo "On Fedora: sudo dnf install qt6-qtbase-devel qt6-qtwebengine-devel qt6-qtsql"
    exit 1
fi

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake not found. Please install CMake.${NC}"
    echo "On Ubuntu/Debian: sudo apt install cmake"
    echo "On Fedora: sudo dnf install cmake"
    exit 1
fi

# Project directory
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo -e "${YELLOW}Project directory: $PROJECT_DIR${NC}"
echo -e "${YELLOW}Build directory: $BUILD_DIR${NC}"
echo

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
INSTALL=false
RUN_AFTER_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -r|--run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug    Build in Debug mode (default: Release)"
            echo "  -c, --clean    Clean build directory before building"
            echo "  -i, --install  Install after building"
            echo "  -r, --run      Run application after building"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}Build configuration:${NC}"
echo -e "  Build type: ${GREEN}$BUILD_TYPE${NC}"
echo -e "  Clean build: ${GREEN}$CLEAN_BUILD${NC}"
echo -e "  Install: ${GREEN}$INSTALL${NC}"
echo -e "  Run after build: ${GREEN}$RUN_AFTER_BUILD${NC}"
echo

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      "$PROJECT_DIR"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . --config "$BUILD_TYPE" -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build completed successfully!${NC}"

# Install if requested
if [ "$INSTALL" = true ]; then
    echo -e "${YELLOW}Installing...${NC}"
    cmake --install . --config "$BUILD_TYPE"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Installation failed!${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Installation completed!${NC}"
fi

# Run the application if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo -e "${YELLOW}Running X2 Modern...${NC}"
    if [ -f "./X2Modern" ]; then
        ./X2Modern
    elif [ -f "./x2modern" ]; then
        ./x2modern
    else
        echo -e "${RED}Executable not found in build directory!${NC}"
        exit 1
    fi
fi

echo
echo -e "${GREEN}=== Build process completed ===${NC}"
echo -e "Executable location: ${BLUE}$BUILD_DIR/X2Modern${NC}"
echo
echo "To run the application:"
echo "  cd $BUILD_DIR && ./X2Modern"
echo
echo "To install system-wide:"
echo "  $0 --install"
