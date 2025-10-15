cmake_minimum_required(VERSION 3.20)
project(WinHKMon VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows version requirement (Windows 10+ for GetIfTable2, MIB_IF_ROW2 APIs)
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)  # Windows 10
endif()

# MSVC-specific flags
if(MSVC)
    # Warnings as errors, high warning level
    add_compile_options(/W4 /WX)
    
    # Release optimizations
    add_compile_options("$<$<CONFIG:Release>:/O2>")
    add_compile_options("$<$<CONFIG:Release>:/GL>")  # Whole program optimization
    
    # Debug symbols
    add_compile_options("$<$<CONFIG:Debug>:/Zi>")
    add_compile_options("$<$<CONFIG:Debug>:/Od>")
endif()

# Core Library (WinHKMonLib)
add_library(WinHKMonLib STATIC
    src/WinHKMonLib/CliParser.cpp
    src/WinHKMonLib/OutputFormatter.cpp
    src/WinHKMonLib/StateManager.cpp
    src/WinHKMonLib/MemoryMonitor.cpp
    src/WinHKMonLib/CpuMonitor.cpp
    src/WinHKMonLib/DeltaCalculator.cpp
    src/WinHKMonLib/NetworkMonitor.cpp
    src/WinHKMonLib/DiskMonitor.cpp
    src/WinHKMonLib/TempMonitor.cpp
)

target_include_directories(WinHKMonLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Configure TempMonitor.cpp for C++/CLI compilation
# /clr requires /EHa (async exception handling) instead of /EHs
if(MSVC)
    # Find .NET Framework reference assemblies
    # Typical locations for .NET Framework 4.x
    set(DOTNET_FRAMEWORK_PATH "C:/Program Files (x86)/Reference Assemblies/Microsoft/Framework/.NETFramework/v4.7.2")
    if(NOT EXISTS "${DOTNET_FRAMEWORK_PATH}")
        set(DOTNET_FRAMEWORK_PATH "C:/Program Files (x86)/Reference Assemblies/Microsoft/Framework/.NETFramework/v4.8")
    endif()
    if(NOT EXISTS "${DOTNET_FRAMEWORK_PATH}")
        set(DOTNET_FRAMEWORK_PATH "C:/Windows/Microsoft.NET/Framework64/v4.0.30319")
    endif()
    
    set_source_files_properties(src/WinHKMonLib/TempMonitor.cpp 
        PROPERTIES 
        COMPILE_FLAGS "/clr /EHa /AI\"${DOTNET_FRAMEWORK_PATH}\" /AI\"${CMAKE_CURRENT_SOURCE_DIR}/lib\" /FU\"${CMAKE_CURRENT_SOURCE_DIR}/lib/LibreHardwareMonitorLib.dll\""
        COMPILE_DEFINITIONS "UNICODE;_UNICODE"
    )
    
    message(STATUS "C++/CLI .NET Framework path: ${DOTNET_FRAMEWORK_PATH}")
endif()

# Windows API libraries
target_link_libraries(WinHKMonLib
    PUBLIC
        pdh        # Performance Data Helper
        iphlpapi   # IP Helper API (network)
        powrprof   # Power management (CPU frequency)
)

# CLI Executable (WinHKMon.exe)
add_executable(WinHKMon
    src/WinHKMon/main.cpp
)

target_link_libraries(WinHKMon
    PRIVATE
        WinHKMonLib
)

# Copy LibreHardwareMonitorLib.dll to output directory
add_custom_command(TARGET WinHKMon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/LibreHardwareMonitorLib.dll"
        "$<TARGET_FILE_DIR:WinHKMon>"
    COMMENT "Copying LibreHardwareMonitorLib.dll to output directory"
)

# Testing
enable_testing()
add_subdirectory(tests)

# Print configuration summary
message(STATUS "====================================")
message(STATUS "WinHKMon Configuration Summary")
message(STATUS "====================================")
message(STATUS "Version:           ${PROJECT_VERSION}")
message(STATUS "Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "====================================")

