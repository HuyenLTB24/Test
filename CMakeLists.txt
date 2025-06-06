cmake_minimum_required(VERSION 3.16.0)

project(X2Modern VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network
    Sql
    WebEngine
    WebEngineWidgets
    Concurrent
)

# Find additional dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/twitter_bot_gui.cpp
    src/twitter_bot.cpp
    src/database_manager.cpp
    src/account_manager.cpp
    src/bot_worker.cpp
)

# Header files
set(HEADERS
    include/twitter_bot_gui.h
    include/twitter_bot.h
    include/database_manager.h
    include/account_manager.h
    include/bot_worker.h
    include/api_manager.h
    include/settings_manager.h
    include/log_manager.h
    include/constants.h
)

# Create executable
add_executable(X2Modern ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(X2Modern
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::WebEngine
    Qt6::WebEngineWidgets
    Qt6::Concurrent
    ${CURL_LIBRARIES}
)

# Compiler flags
target_compile_options(X2Modern PRIVATE ${CURL_CFLAGS_OTHER})

# Copy resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Installation
install(TARGETS X2Modern
    RUNTIME DESTINATION bin
)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
