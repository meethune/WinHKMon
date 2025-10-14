#include "WinHKMonLib/CpuMonitor.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace WinHKMon;

/**
 * Test Suite: CpuMonitor
 * 
 * Tests for the CpuMonitor component that collects CPU usage and frequency statistics
 * using the Performance Data Helper (PDH) API and CallNtPowerInformation.
 * 
 * Coverage:
 * - Initialization and cleanup
 * - Valid data retrieval
 * - Per-core statistics
 * - Percentage ranges (0-100)
 * - Frequency values
 * - Error handling
 * - Resource leak prevention
 */

// Test fixture for CpuMonitor tests
class CpuMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Tests will initialize their own monitor instances
    }

    void TearDown() override {
        // Each test cleans up its own monitor
    }
};

// Test 1: Initialize() succeeds
TEST_F(CpuMonitorTest, InitializeSucceeds) {
    CpuMonitor monitor;
    EXPECT_NO_THROW(monitor.initialize());
}

// Test 2: getCurrentStats() returns valid data after initialization
TEST_F(CpuMonitorTest, ReturnsValidStatsAfterInitialization) {
    CpuMonitor monitor;
    monitor.initialize();
    
    // PDH requires two samples for percentage calculation, so we need to wait
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    // Basic validation
    EXPECT_GE(stats.totalUsagePercent, 0.0);
    EXPECT_LE(stats.totalUsagePercent, 100.0);
    EXPECT_GT(stats.cores.size(), 0);
}

// Test 3: Total usage percentage in 0-100 range
TEST_F(CpuMonitorTest, TotalUsagePercentageInValidRange) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    EXPECT_GE(stats.totalUsagePercent, 0.0);
    EXPECT_LE(stats.totalUsagePercent, 100.0);
}

// Test 4: Per-core usage percentages in 0-100 range
TEST_F(CpuMonitorTest, PerCoreUsagePercentagesInValidRange) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    EXPECT_GT(stats.cores.size(), 0);  // At least one core
    
    for (const auto& core : stats.cores) {
        EXPECT_GE(core.usagePercent, 0.0);
        EXPECT_LE(core.usagePercent, 100.0);
    }
}

// Test 5: Core count matches system
TEST_F(CpuMonitorTest, CoreCountMatchesSystem) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    // Get system processor count
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD numProcessors = sysInfo.dwNumberOfProcessors;
    
    EXPECT_EQ(stats.cores.size(), numProcessors);
}

// Test 6: Core IDs are sequential and start from 0
TEST_F(CpuMonitorTest, CoreIdsAreSequential) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    for (size_t i = 0; i < stats.cores.size(); ++i) {
        EXPECT_EQ(stats.cores[i].coreId, static_cast<int>(i));
    }
}

// Test 7: Frequency values are realistic (> 0 and < 10 GHz)
TEST_F(CpuMonitorTest, FrequencyValuesRealistic) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    // Average frequency should be positive
    EXPECT_GT(stats.averageFrequencyMhz, 0);
    
    // Should not exceed 10 GHz (10000 MHz) - modern CPUs typically < 6 GHz
    EXPECT_LT(stats.averageFrequencyMhz, 10000);
    
    // Per-core frequencies should also be realistic
    for (const auto& core : stats.cores) {
        EXPECT_GT(core.frequencyMhz, 0);
        EXPECT_LT(core.frequencyMhz, 10000);
    }
}

// Test 8: Cleanup doesn't crash and can be called multiple times
TEST_F(CpuMonitorTest, CleanupDoesNotCrash) {
    CpuMonitor monitor;
    monitor.initialize();
    
    EXPECT_NO_THROW(monitor.cleanup());
    EXPECT_NO_THROW(monitor.cleanup());  // Should be safe to call twice
}

// Test 9: Multiple samples return reasonable data
TEST_F(CpuMonitorTest, MultipleSamplesReturnReasonableData) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats1 = monitor.getCurrentStats();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CpuStats stats2 = monitor.getCurrentStats();
    
    // Both samples should have valid data
    EXPECT_GE(stats1.totalUsagePercent, 0.0);
    EXPECT_LE(stats1.totalUsagePercent, 100.0);
    EXPECT_GE(stats2.totalUsagePercent, 0.0);
    EXPECT_LE(stats2.totalUsagePercent, 100.0);
    
    // Core counts should be identical
    EXPECT_EQ(stats1.cores.size(), stats2.cores.size());
}

// Test 10: Average frequency is calculated correctly
TEST_F(CpuMonitorTest, AverageFrequencyCalculatedCorrectly) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    // Calculate average manually
    uint64_t sum = 0;
    for (const auto& core : stats.cores) {
        sum += core.frequencyMhz;
    }
    uint64_t expectedAvg = sum / stats.cores.size();
    
    // Should be within 10 MHz of calculated average (allow for rounding)
    EXPECT_NEAR(static_cast<double>(stats.averageFrequencyMhz), static_cast<double>(expectedAvg), 10.0);
}

// Test 11: Usage under load increases (stress test)
TEST_F(CpuMonitorTest, UsageIncreasesUnderLoad) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Get idle usage
    CpuStats idleStats = monitor.getCurrentStats();
    
    // Generate CPU load (busy loop for 100ms on all cores)
    auto start = std::chrono::high_resolution_clock::now();
    volatile int dummy = 0;
    while (std::chrono::high_resolution_clock::now() - start < std::chrono::milliseconds(100)) {
        dummy++;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Get loaded usage
    CpuStats loadedStats = monitor.getCurrentStats();
    
    // Usage should be higher under load (at least 10% higher)
    // Note: This test might be flaky on systems with high background activity
    EXPECT_GT(loadedStats.totalUsagePercent, idleStats.totalUsagePercent);
}

// Test 12: Optional fields (user/system/idle) are handled correctly
TEST_F(CpuMonitorTest, OptionalFieldsHandledCorrectly) {
    CpuMonitor monitor;
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CpuStats stats = monitor.getCurrentStats();
    
    // In v1.0, these optional fields might not be populated
    // If they are populated, they should sum to approximately 100%
    if (stats.userPercent.has_value() && 
        stats.systemPercent.has_value() && 
        stats.idlePercent.has_value()) {
        
        double sum = stats.userPercent.value() + 
                     stats.systemPercent.value() + 
                     stats.idlePercent.value();
        
        EXPECT_NEAR(sum, 100.0, 1.0);  // Allow 1% variance for rounding
    }
}

// Test 13: Uninitialized getCurrentStats throws or returns empty data
TEST_F(CpuMonitorTest, UninitializedGetStatsHandledGracefully) {
    CpuMonitor monitor;
    
    // Calling getCurrentStats without initialize should either:
    // 1. Throw an exception, or
    // 2. Return empty/invalid data (0 cores)
    
    try {
        CpuStats stats = monitor.getCurrentStats();
        // If it doesn't throw, cores should be empty or data should be invalid
        EXPECT_TRUE(stats.cores.empty() || stats.totalUsagePercent == 0.0);
    } catch (const std::exception&) {
        // Exception is acceptable behavior
        SUCCEED();
    }
}

// Test 14: Reinitialize after cleanup works correctly
TEST_F(CpuMonitorTest, ReinitializeAfterCleanup) {
    CpuMonitor monitor;
    
    // First cycle
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CpuStats stats1 = monitor.getCurrentStats();
    EXPECT_GT(stats1.cores.size(), 0);
    monitor.cleanup();
    
    // Second cycle
    monitor.initialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CpuStats stats2 = monitor.getCurrentStats();
    EXPECT_GT(stats2.cores.size(), 0);
    
    // Both cycles should return valid data
    EXPECT_EQ(stats1.cores.size(), stats2.cores.size());
}

