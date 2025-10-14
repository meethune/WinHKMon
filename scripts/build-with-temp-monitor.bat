@echo off
REM Build WinHKMon with Temperature Monitoring Support
REM 
REM Requirements:
REM   - Visual Studio 2022 with .NET Desktop Development workload
REM   - LibreHardwareMonitorLib.dll in lib/ directory
REM   - Run from Developer Command Prompt for VS 2022
REM
REM Usage:
REM   .\scripts\build-with-temp-monitor.bat [Debug|Release]

setlocal enabledelayedexpansion

REM Configuration
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo ================================================================
echo WinHKMon Build Script with Temperature Monitoring
echo ================================================================
echo.
echo Build Type: %BUILD_TYPE%
echo.

REM Check if we're in Developer Command Prompt
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo ERROR: MSVC compiler not found!
    echo Please run this script from "Developer Command Prompt for VS 2022"
    echo or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    exit /b 1
)

echo [1/5] Checking prerequisites...
echo.

REM Check for CMake
where cmake.exe >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    exit /b 1
)
echo   - CMake: OK

REM Check for LibreHardwareMonitor
if not exist "lib\LibreHardwareMonitorLib.dll" (
    echo   - LibreHardwareMonitorLib.dll: MISSING
    echo.
    echo ERROR: LibreHardwareMonitorLib.dll not found in lib\ directory
    echo.
    echo Download from: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases
    echo Or install via NuGet in Visual Studio:
    echo   Install-Package LibreHardwareMonitorLib
    echo.
    echo Place the DLL in: %CD%\lib\
    exit /b 1
) else (
    echo   - LibreHardwareMonitorLib.dll: OK
)

echo.
echo [2/5] Creating build directory...
if not exist "build" mkdir build
cd build

echo.
echo [3/5] Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DENABLE_TEMP_MONITOR=ON
    
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
echo [4/5] Building project...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

echo.
echo [5/5] Running tests...
ctest -C %BUILD_TYPE% --output-on-failure

if errorlevel 1 (
    echo WARNING: Some tests failed
    echo This is expected if not running as administrator
    cd ..
    exit /b 1
)

cd ..

echo.
echo ================================================================
echo Build Complete!
echo ================================================================
echo.
echo Executable: build\%BUILD_TYPE%\WinHKMon.exe
echo Library DLL: build\%BUILD_TYPE%\LibreHardwareMonitorLib.dll
echo.
echo To test temperature monitoring:
echo   1. Right-click Command Prompt and "Run as administrator"
echo   2. cd %CD%
echo   3. build\%BUILD_TYPE%\WinHKMon.exe CPU TEMP
echo.
echo Without admin privileges, TEMP will gracefully skip.
echo.

endlocal

