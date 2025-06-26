@echo off
setlocal enabledelayedexpansion

echo ========================================
echo FENgineLive Build Script
echo ========================================

:: Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

:: Set build type
set BUILD_TYPE=Release
if "%1"=="debug" set BUILD_TYPE=Debug
if "%1"=="relwithdebinfo" set BUILD_TYPE=RelWithDebInfo

echo Building in %BUILD_TYPE% mode...

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

:: Build the project
echo Building project...
cmake --build . --config %BUILD_TYPE% --parallel
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

:: Copy runtime dependencies
echo Copying runtime dependencies...
if exist "..\external\lc0" (
    if not exist "lc0" mkdir lc0
    xcopy "..\external\lc0\*" "lc0\" /Y /Q
)

if exist "..\external\python\fen_tracker" (
    if not exist "python" mkdir python
    xcopy "..\external\python\fen_tracker\*" "python\" /Y /Q
)

if exist "..\external\python\sable" (
    if not exist "python" mkdir python
    if not exist "python\sable" mkdir python\sable
    xcopy "..\external\python\sable\*" "python\sable\" /Y /Q
)

if exist "..\assets" (
    if not exist "assets" mkdir assets
    xcopy "..\assets\*" "assets\" /Y /Q /E
)

echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable location: build\%BUILD_TYPE%\ChessGUI.exe
echo.
echo To run the application:
echo   cd build\%BUILD_TYPE%
echo   ChessGUI.exe
echo.

pause 
