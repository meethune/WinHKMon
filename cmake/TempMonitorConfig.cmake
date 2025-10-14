# TempMonitor CMake Configuration
#
# This file contains configuration for building TempMonitor with LibreHardwareMonitor.
# 
# REQUIREMENTS:
# - Windows operating system
# - Visual Studio 2022 with .NET Desktop Development workload
# - LibreHardwareMonitorLib.dll (from NuGet or manual download)
# - C++/CLI support (/clr compiler flag)
#
# USAGE:
#   include(cmake/TempMonitorConfig.cmake)
#   configure_temp_monitor(WinHKMonLib)

# Check if we're on Windows
if(NOT WIN32)
    message(STATUS "TempMonitor: Skipping (requires Windows)")
    set(TEMP_MONITOR_AVAILABLE FALSE CACHE BOOL "Temperature monitoring available")
    return()
endif()

# Check if we have MSVC compiler
if(NOT MSVC)
    message(STATUS "TempMonitor: Skipping (requires MSVC compiler)")
    set(TEMP_MONITOR_AVAILABLE FALSE CACHE BOOL "Temperature monitoring available")
    return()
endif()

# Find LibreHardwareMonitor library
find_file(LHM_DLL
    NAMES LibreHardwareMonitorLib.dll
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_BINARY_DIR}
    DOC "LibreHardwareMonitor library DLL"
)

if(NOT LHM_DLL)
    message(WARNING "TempMonitor: LibreHardwareMonitorLib.dll not found")
    message(STATUS "  Download from: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases")
    message(STATUS "  Or install via NuGet: Install-Package LibreHardwareMonitorLib")
    message(STATUS "  Place in: ${CMAKE_CURRENT_SOURCE_DIR}/lib/")
    set(TEMP_MONITOR_AVAILABLE FALSE CACHE BOOL "Temperature monitoring available")
    return()
endif()

message(STATUS "TempMonitor: Found LibreHardwareMonitorLib.dll at ${LHM_DLL}")
set(TEMP_MONITOR_AVAILABLE TRUE CACHE BOOL "Temperature monitoring available")

# Function to configure TempMonitor for a target
function(configure_temp_monitor TARGET_NAME)
    if(NOT TEMP_MONITOR_AVAILABLE)
        message(STATUS "TempMonitor: Not configured for ${TARGET_NAME} (unavailable)")
        return()
    endif()
    
    message(STATUS "TempMonitor: Configuring for ${TARGET_NAME}")
    
    # Set C++/CLI compilation flag for TempMonitor.cpp
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/WinHKMonLib/TempMonitor.cpp
        PROPERTIES
        COMPILE_FLAGS "/clr"
    )
    
    # Add reference to LibreHardwareMonitorLib.dll
    # Note: This needs to be done in Visual Studio project or via #using directive
    
    # Add post-build command to copy DLL to output directory
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LHM_DLL}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMENT "Copying LibreHardwareMonitorLib.dll to output directory"
    )
    
    message(STATUS "TempMonitor: Configured successfully")
    message(STATUS "  - C++/CLI flag enabled for TempMonitor.cpp")
    message(STATUS "  - LibreHardwareMonitorLib.dll will be copied to output")
endfunction()

# Optional: Function to download LibreHardwareMonitor if not present
function(download_librehardwaremonitor)
    set(LHM_VERSION "0.9.3" CACHE STRING "LibreHardwareMonitor version to download")
    set(LHM_URL "https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases/download/v${LHM_VERSION}/LibreHardwareMonitor-net472.zip")
    
    message(STATUS "Downloading LibreHardwareMonitor v${LHM_VERSION}...")
    
    file(DOWNLOAD
        ${LHM_URL}
        ${CMAKE_CURRENT_BINARY_DIR}/lhm.zip
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
    )
    
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        message(WARNING "Failed to download LibreHardwareMonitor")
        return()
    endif()
    
    message(STATUS "Extracting LibreHardwareMonitor...")
    file(ARCHIVE_EXTRACT
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/lhm.zip
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/lhm
    )
    
    message(STATUS "LibreHardwareMonitor downloaded successfully")
endfunction()

