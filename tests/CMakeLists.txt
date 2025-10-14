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
)

target_link_libraries(WinHKMonTests
    PRIVATE
        WinHKMonLib
        GTest::gtest_main
)

# Discover and register tests
gtest_discover_tests(WinHKMonTests)

# Add test command
add_test(
    NAME WinHKMonTests
    COMMAND WinHKMonTests
)

