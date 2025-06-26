cmake_minimum_required(VERSION 3.16)

project(ChessGUI VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt configuration
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific optimizations
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /D_DEBUG /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /OPT:REF /OPT:ICF")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -D_DEBUG")
endif()

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

# Find OpenCV
find_package(OpenCV REQUIRED CONFIG)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Set OpenCV_DIR to the folder containing OpenCVConfig.cmake")
endif()

# Project source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    boardwidget.h
    boardwidget.cpp
    arrowoverlay.h
    arrowoverlay.cpp
    settingsdialog.h
    settingsdialog.cpp
    regionselector.h
    regionselector.cpp
    chessboard_detector.h
    chessboard_detector.cpp
    globalhotkeymanager.h
    globalhotkeymanager.cpp
    telemetrymanager.h
    telemetrymanager.cpp
    telemetrydashboardv2.h
    telemetrydashboardv2.cpp
    stealthmoveselector.h
    stealthmoveselector.cpp
)

# Create executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ChessGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ChessGUI SHARED ${PROJECT_SOURCES})
    else()
        add_executable(ChessGUI ${PROJECT_SOURCES})
    endif()
endif()

# Link libraries
target_link_libraries(ChessGUI PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Svg
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(ChessGUI PRIVATE ${OpenCV_INCLUDE_DIRS})

# Platform-specific settings
if(WIN32)
    set_target_properties(ChessGUI PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE FALSE
    )
elseif(APPLE)
    set_target_properties(ChessGUI PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.fenginelive.chessgui
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# Installation configuration
include(GNUInstallDirs)
install(TARGETS ChessGUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets 
    DESTINATION ${CMAKE_INSTALL_DATADIR}/chessgui
    FILES_MATCHING PATTERN "*.svg" PATTERN "*.ttf" PATTERN "*.qss"
)



# Bundle Python scripts
install(DIRECTORY ${CMAKE_SOURCE_DIR}/external/python/fen_tracker/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/chessgui/python
        PATTERN "__pycache__" EXCLUDE
        PATTERN "*.pyc" EXCLUDE)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/external/python/sable/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/chessgui/python/sable
        PATTERN "__pycache__" EXCLUDE
        PATTERN "*.pyc" EXCLUDE)

# Install runtime libraries from Qt deployment
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ChessGUI)
endif()

# Post-build copy commands
add_custom_command(TARGET ChessGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:ChessGUI>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/external/python/fen_tracker
            $<TARGET_FILE_DIR:ChessGUI>/python
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/external/python/sable
            $<TARGET_FILE_DIR:ChessGUI>/python/sable
    COMMENT "Copying runtime folders: assets/, python/"
)

# CPack configuration for installer
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "FENgineLive")
set(CPACK_PACKAGE_VENDOR "William Samiri")
set(CPACK_PACKAGE_CONTACT "williamsamiri011@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Real-time chess analysis with AI assistance")
set(CPACK_PACKAGE_DESCRIPTION "FENgineLive is a hybrid Qt + Python application that provides real-time chess analysis using computer vision and AI engines.")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "FENgineLive")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "ChessGUI.exe")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "FENgineLive")
    set(CPACK_PACKAGE_EXECUTABLES "ChessGUI" "FENgineLive")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "FENgineLive")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
