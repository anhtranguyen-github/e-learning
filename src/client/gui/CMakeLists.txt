cmake_minimum_required(VERSION 3.15)

project(SockerClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Quick Widgets Network Multimedia)
find_package(Threads REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../include/client
    ${CMAKE_SOURCE_DIR}/../../../include/common
)

# Source files
set(SOURCES
    main.cpp
    NetworkManager.cpp
    ../../../src/client/network.cpp
    ../../../src/common/logger.cpp
    ../../../src/common/utils.cpp
    ../../../src/common/protocol.cpp
)

# Add executable
add_executable(socker_client ${SOURCES})

# Link libraries
target_link_libraries(socker_client
    PRIVATE
    Qt5::Quick
    Qt5::Widgets
    Qt5::Network
    Qt5::Multimedia
    Threads::Threads
)

# Copy QML files and qmldir to build directory
file(GLOB QML_FILES "*.qml")
file(COPY ${QML_FILES} "qmldir" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
