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

# Windows下自动复制SSL库文件到输出目录
if(WIN32)
    # 设置输出目录
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    )
    
    # 确保bin目录存在
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    
    # 复制SSL库文件到输出目录（构建时立即复制）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ssl/libcrypto-1_1.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libcrypto-1_1.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ssl/libssl-1_1.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libssl-1_1.dll"
        COMMENT "Copying SSL libraries to output directory"
        VERBATIM
    )
    
    # 额外确保：在配置时也复制一次（防止首次构建时遗漏）
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ssl/libcrypto-1_1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/libcrypto-1_1.dll"
        COPYONLY
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ssl/libssl-1_1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/libssl-1_1.dll"
        COPYONLY
    )
endif()