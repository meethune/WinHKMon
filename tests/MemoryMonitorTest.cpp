#include "WinHKMonLib/MemoryMonitor.h"
#include <gtest/gtest.h>

using namespace WinHKMon;

/**
 * Test Suite: MemoryMonitor
 * 
 * Tests for the MemoryMonitor component that collects RAM and page file statistics
 * using the GlobalMemoryStatusEx() Windows API.
 * 
 * Coverage:
 * - Valid data retrieval
 * - Calculated fields (usedPhysicalBytes, percentages)
 * - Invariant validation (total >= available)
 * - Range validation (percentages 0-100)
 * - Error handling (API failure simulation - difficult on real systems)
 */

// Test 1: getCurrentStats() returns valid data
TEST(MemoryMonitorTest, ReturnsValidMemoryStats) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    // Basic sanity checks
    EXPECT_GT(stats.totalPhysicalBytes, 0);
    EXPECT_LE(stats.availablePhysicalBytes, stats.totalPhysicalBytes);
    EXPECT_GT(stats.totalPageFileBytes, 0);
}

// Test 2: Total >= Available invariant
TEST(MemoryMonitorTest, TotalGreaterThanOrEqualToAvailable) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    EXPECT_GE(stats.totalPhysicalBytes, stats.availablePhysicalBytes);
    EXPECT_GE(stats.totalPageFileBytes, stats.availablePageFileBytes);
}

// Test 3: Usage percentages in 0-100 range
TEST(MemoryMonitorTest, PercentagesInValidRange) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    EXPECT_GE(stats.usagePercent, 0.0);
    EXPECT_LE(stats.usagePercent, 100.0);
    EXPECT_GE(stats.pageFilePercent, 0.0);
    EXPECT_LE(stats.pageFilePercent, 100.0);
}

// Test 4: Calculated fields (usedPhysicalBytes) are correct
TEST(MemoryMonitorTest, CalculatedFieldsCorrect) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    // usedPhysicalBytes should equal total - available
    uint64_t expectedUsed = stats.totalPhysicalBytes - stats.availablePhysicalBytes;
    EXPECT_EQ(stats.usedPhysicalBytes, expectedUsed);

    // usedPageFileBytes should equal total - available
    uint64_t expectedPageFileUsed = stats.totalPageFileBytes - stats.availablePageFileBytes;
    EXPECT_EQ(stats.usedPageFileBytes, expectedPageFileUsed);
}

// Test 5: Usage percentage calculation is accurate
TEST(MemoryMonitorTest, UsagePercentageCalculationAccurate) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    double expectedPercent = (static_cast<double>(stats.usedPhysicalBytes) / 
                             stats.totalPhysicalBytes) * 100.0;
    EXPECT_NEAR(stats.usagePercent, expectedPercent, 0.01);
}

// Test 6: Page file percentage calculation is accurate
TEST(MemoryMonitorTest, PageFilePercentageCalculationAccurate) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    if (stats.totalPageFileBytes > 0) {
        double expectedPercent = (static_cast<double>(stats.usedPageFileBytes) / 
                                 stats.totalPageFileBytes) * 100.0;
        EXPECT_NEAR(stats.pageFilePercent, expectedPercent, 0.01);
    } else {
        // If no page file, percentage should be 0
        EXPECT_EQ(stats.pageFilePercent, 0.0);
    }
}

// Test 7: Multiple calls return consistent data (within reasonable variance)
TEST(MemoryMonitorTest, MultipleCallsReturnConsistentData) {
    MemoryMonitor monitor;
    
    MemoryStats stats1 = monitor.getCurrentStats();
    MemoryStats stats2 = monitor.getCurrentStats();
    
    // Total memory should be identical
    EXPECT_EQ(stats1.totalPhysicalBytes, stats2.totalPhysicalBytes);
    EXPECT_EQ(stats1.totalPageFileBytes, stats2.totalPageFileBytes);
    
    // Available memory might vary slightly, but should be close
    // Allow up to 10% variance (system might allocate/free memory between calls)
    double variance = static_cast<double>(std::abs(static_cast<int64_t>(
        stats1.availablePhysicalBytes - stats2.availablePhysicalBytes))) / 
        stats1.totalPhysicalBytes;
    EXPECT_LT(variance, 0.10);  // Less than 10% variance
}

// Test 8: Memory values are realistic (not corrupted)
TEST(MemoryMonitorTest, ValuesAreRealistic) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();
    
    // Total physical RAM should be at least 512 MB (0.5 GB)
    // Modern systems typically have 4GB+ but we allow for minimal systems
    EXPECT_GE(stats.totalPhysicalBytes, 512ULL * 1024 * 1024);
    
    // Total RAM should not exceed 1 TB (sanity check for corruption)
    EXPECT_LE(stats.totalPhysicalBytes, 1024ULL * 1024 * 1024 * 1024);
    
    // Page file should be reasonable (not larger than 10x physical RAM)
    EXPECT_LE(stats.totalPageFileBytes, stats.totalPhysicalBytes * 10);
}

// Test 9: Zero page file handling
TEST(MemoryMonitorTest, HandlesZeroPageFile) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();
    
    // Some systems might have page file disabled
    // If totalPageFileBytes is 0, all page file fields should be 0
    if (stats.totalPageFileBytes == 0) {
        EXPECT_EQ(stats.availablePageFileBytes, 0);
        EXPECT_EQ(stats.usedPageFileBytes, 0);
        EXPECT_EQ(stats.pageFilePercent, 0.0);
    }
}

// Test 10: Optional fields are not set (for MemoryStats, cachedBytes and committedBytes)
TEST(MemoryMonitorTest, OptionalFieldsHandled) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();
    
    // For v1.0, optional fields might not be populated
    // This test just verifies the structure is correct
    // (No assertion needed, just ensure compilation and access works)
    
    if (stats.cachedBytes.has_value()) {
        EXPECT_GT(stats.cachedBytes.value(), 0);
    }
    
    if (stats.committedBytes.has_value()) {
        EXPECT_GT(stats.committedBytes.value(), 0);
    }
}

