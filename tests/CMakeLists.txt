# Google Test setup
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include Google Test's CMake utilities
include(GoogleTest)

# Test executable
add_executable(WinHKMonTests
    SampleTest.cpp
    CliParserTest.cpp
    OutputFormatterTest.cpp
    StateManagerTest.cpp
    MemoryMonitorTest.cpp
    CpuMonitorTest.cpp
    DeltaCalculatorTest.cpp
    NetworkMonitorTest.cpp
    DiskMonitorTest.cpp
    TempMonitorTest.cpp
)

target_link_libraries(WinHKMonTests
    PRIVATE
        WinHKMonLib
        GTest::gtest_main
)

# WinHKMonTests needs CLR support because it links against WinHKMonLib which contains
# C++/CLI code (TempMonitor.cpp). When linking a native executable against a static
# library with C++/CLI, the executable becomes a mixed-mode assembly.
if(MSVC)
    # Enable CLR for the test executable by setting /clr flag on link
    set_target_properties(WinHKMonTests PROPERTIES
        COMMON_LANGUAGE_RUNTIME ""
    )
endif()

# Copy LibreHardwareMonitorLib.dll to test output directory
add_custom_command(TARGET WinHKMonTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../lib/LibreHardwareMonitorLib.dll"
        "$<TARGET_FILE_DIR:WinHKMonTests>"
    COMMENT "Copying LibreHardwareMonitorLib.dll to test directory"
)

# Discover and register tests
gtest_discover_tests(WinHKMonTests)

# Add test command
add_test(
    NAME WinHKMonTests
    COMMAND WinHKMonTests
)

