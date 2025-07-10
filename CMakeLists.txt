cmake_minimum_required(VERSION 3.16)

project(Spark-godot-translation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt5 components
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Concurrent
)

# Set Qt5 properties
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    translationworker.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    translationworker.h
)

# UI files
set(UI_FILES
    mainwindow.ui
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# Link Qt5 libraries
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Concurrent
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)