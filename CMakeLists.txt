cmake_minimum_required(VERSION 3.16)

project(ChessGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Svg)

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
        telemetrydashboard.h
        telemetrydashboard.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ChessGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ChessGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ChessGUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ChessGUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ChessGUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ChessGUI)
endif()
set_target_properties(ChessGUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ChessGUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION .
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION .)

# Bundle Lc0 engine
set(LC0_DIR ${CMAKE_SOURCE_DIR}/external/lc0)
if(EXISTS "${LC0_DIR}/lc0.exe")
    install(DIRECTORY ${LC0_DIR}/ DESTINATION lc0)
else()
    message(WARNING "Lc0 engine not found: ${LC0_DIR}")
endif()

# Bundle Python scripts
install(DIRECTORY ${CMAKE_SOURCE_DIR}/external/python/
        DESTINATION python
        PATTERN "__pycache__" EXCLUDE)

# Install runtime libraries from Qt deployment
install(DIRECTORY "${CMAKE_BINARY_DIR}/staging/"
        DESTINATION .
        FILES_MATCHING PATTERN "*.dll")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ChessGUI)
endif()

# Add post-build copy of runtime folders
add_custom_command(TARGET ChessGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:ChessGUI>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${LC0_DIR}
            $<TARGET_FILE_DIR:ChessGUI>/lc0
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/external/python
            $<TARGET_FILE_DIR:ChessGUI>/python
    COMMENT "Copying runtime folders: assets/, lc0/, python/"
)

# Find OpenCV - assumes OpenCV_DIR is passed at configure time
find_package(OpenCV REQUIRED CONFIG)

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_INCLUDE_DIRS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(ChessGUI PRIVATE ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found. Set OpenCV_DIR to the folder containing OpenCVConfig.cmake")
endif()

# Include MSVC runtime and configure NSIS installer
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "FENgineLive")
set(CPACK_PACKAGE_VENDOR "William Samiri")
set(CPACK_PACKAGE_CONTACT "williamsamiri011@gmail.com")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_DISPLAY_NAME "FENgineLive")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FENgineLive")

# Fix Start Menu shortcut target
set(CPACK_PACKAGE_EXECUTABLES "ChessGUI" "FENgineLive")
set(CPACK_NSIS_DISPLAY_NAME "FENgineLive")
set(CPACK_NSIS_INSTALLED_ICON_NAME "ChessGUI.exe")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "FENgineLive")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
