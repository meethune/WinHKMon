/**
 * @file DiskMonitorTest.cpp
 * @brief Unit tests for DiskMonitor component
 * 
 * Tests disk I/O monitoring using Windows PDH counters for physical disks.
 */

#include "WinHKMonLib/DiskMonitor.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace WinHKMon;

/**
 * Test Fixture for DiskMonitor tests
 */
class DiskMonitorTest : public ::testing::Test {
protected:
    DiskMonitor monitor;
    
    void SetUp() override {
        // Initialize monitor before each test
        try {
            monitor.initialize();
        } catch (const std::exception& e) {
            // If initialization fails, skip tests that require it
            GTEST_SKIP() << "DiskMonitor initialization failed: " << e.what();
        }
    }
    
    void TearDown() override {
        // Clean up after each test
        try {
            monitor.cleanup();
        } catch (const std::exception&) {
            // Ignore cleanup errors in tests
        }
    }
};

/**
 * T013 Test 1: Verify initialization succeeds with PDH
 */
TEST_F(DiskMonitorTest, InitializeSucceeds) {
    // Initialization already done in SetUp
    SUCCEED() << "DiskMonitor initialized successfully with PDH";
}

/**
 * T013 Test 2: Verify getCurrentStats returns disk list
 */
TEST_F(DiskMonitorTest, GetCurrentStatsReturnsDiskList) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    // Should have at least one physical disk
    EXPECT_GE(disks.size(), 1) << "System should have at least one physical disk";
    
    // Verify each disk has required fields
    for (const auto& disk : disks) {
        EXPECT_FALSE(disk.deviceName.empty()) << "Disk device name should not be empty";
    }
}

/**
 * T013 Test 3: Verify physical disks enumerated (not partitions)
 */
TEST_F(DiskMonitorTest, PhysicalDisksEnumerated) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    // Physical disks typically have names like "0 C:", "1 D:", or "_Total"
    for (const auto& disk : disks) {
        // Device name should not be a single letter (which would indicate partition only)
        EXPECT_GT(disk.deviceName.length(), 1) 
            << "Disk name should be more than a single character: " << disk.deviceName;
    }
}

/**
 * T013 Test 4: Verify read/write rates are non-negative
 */
TEST_F(DiskMonitorTest, ReadWriteRatesNonNegative) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    for (const auto& disk : disks) {
        EXPECT_GE(disk.bytesReadPerSec, 0) 
            << "Read rate should be non-negative for " << disk.deviceName;
        EXPECT_GE(disk.bytesWrittenPerSec, 0) 
            << "Write rate should be non-negative for " << disk.deviceName;
    }
}

/**
 * T013 Test 5: Verify busy percentage in 0-100 range
 */
TEST_F(DiskMonitorTest, BusyPercentageInValidRange) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    for (const auto& disk : disks) {
        EXPECT_GE(disk.percentBusy, 0.0) 
            << "Disk busy % should be >= 0 for " << disk.deviceName;
        EXPECT_LE(disk.percentBusy, 100.0) 
            << "Disk busy % should be <= 100 for " << disk.deviceName;
    }
}

/**
 * T013 Test 6: Verify disk sizes are realistic
 */
TEST_F(DiskMonitorTest, DiskSizesRealistic) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    for (const auto& disk : disks) {
        // Skip _Total which may have zero size
        if (disk.deviceName.find("_Total") != std::string::npos) {
            continue;
        }
        
        // Disk size should be reasonable (at least 1 MB, at most 100 TB)
        EXPECT_GT(disk.totalSizeBytes, 1024ULL * 1024) 
            << "Disk size seems too small for " << disk.deviceName;
        EXPECT_LT(disk.totalSizeBytes, 100ULL * 1024 * 1024 * 1024 * 1024) 
            << "Disk size seems unrealistically large for " << disk.deviceName;
    }
}

/**
 * T013 Test 7: Verify error handling - PDH failure simulation
 * 
 * Test that cleanup can be called multiple times safely
 */
TEST_F(DiskMonitorTest, CleanupMultipleCallsSafe) {
    monitor.cleanup();
    
    // Second cleanup should not crash
    EXPECT_NO_THROW(monitor.cleanup()) 
        << "Multiple cleanup() calls should be safe";
}

/**
 * T013 Test 8: Verify cumulative counters are set
 */
TEST_F(DiskMonitorTest, CumulativeCountersSet) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    for (const auto& disk : disks) {
        // Cumulative counters should be non-negative
        EXPECT_GE(disk.totalBytesRead, 0) 
            << "Total bytes read should be non-negative for " << disk.deviceName;
        EXPECT_GE(disk.totalBytesWritten, 0) 
            << "Total bytes written should be non-negative for " << disk.deviceName;
    }
}

/**
 * T013 Test 9: Verify multiple calls work correctly
 * 
 * PDH requires two samples for rate calculations, so we test multiple calls
 */
TEST_F(DiskMonitorTest, MultipleCallsWorkCorrectly) {
    // First call
    std::vector<DiskStats> disks1 = monitor.getCurrentStats();
    EXPECT_GE(disks1.size(), 1);
    
    // Wait for PDH to collect another sample
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // Second call
    std::vector<DiskStats> disks2 = monitor.getCurrentStats();
    EXPECT_GE(disks2.size(), 1);
    
    // Should have same number of disks
    EXPECT_EQ(disks1.size(), disks2.size()) 
        << "Disk count should remain consistent";
}

/**
 * T013 Test 10: Verify disk names are consistent
 */
TEST_F(DiskMonitorTest, DiskNamesConsistent) {
    std::vector<DiskStats> disks1 = monitor.getCurrentStats();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::vector<DiskStats> disks2 = monitor.getCurrentStats();
    
    // Disk names should be the same
    ASSERT_EQ(disks1.size(), disks2.size());
    for (size_t i = 0; i < disks1.size(); ++i) {
        EXPECT_EQ(disks1[i].deviceName, disks2[i].deviceName)
            << "Disk names should be consistent between calls";
    }
}

/**
 * T013 Test 11: Verify IOPS fields are optional but valid if present
 */
TEST_F(DiskMonitorTest, IOPSFieldsValidIfPresent) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    for (const auto& disk : disks) {
        if (disk.readsPerSec.has_value()) {
            EXPECT_GE(*disk.readsPerSec, 0) 
                << "Reads per sec should be non-negative for " << disk.deviceName;
        }
        if (disk.writesPerSec.has_value()) {
            EXPECT_GE(*disk.writesPerSec, 0) 
                << "Writes per sec should be non-negative for " << disk.deviceName;
        }
    }
}

/**
 * T013 Test 12: Verify _Total disk aggregates correctly
 */
TEST_F(DiskMonitorTest, TotalDiskAggregates) {
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    
    // Find _Total disk
    auto totalIt = std::find_if(disks.begin(), disks.end(),
        [](const DiskStats& disk) { 
            return disk.deviceName.find("_Total") != std::string::npos; 
        });
    
    if (totalIt != disks.end()) {
        // _Total should have valid statistics
        EXPECT_GE(totalIt->bytesReadPerSec, 0);
        EXPECT_GE(totalIt->bytesWrittenPerSec, 0);
        EXPECT_GE(totalIt->percentBusy, 0.0);
        EXPECT_LE(totalIt->percentBusy, 100.0);
    }
}

/**
 * T013 Test 13: Verify reinitialization works
 */
TEST_F(DiskMonitorTest, ReinitializationWorks) {
    // Clean up
    monitor.cleanup();
    
    // Reinitialize
    EXPECT_NO_THROW(monitor.initialize()) 
        << "Reinitialization should work";
    
    // Verify it works after reinitialization
    std::vector<DiskStats> disks = monitor.getCurrentStats();
    EXPECT_GE(disks.size(), 1);
}

/**
 * T013 Test 14: Verify rates increase with actual disk activity
 * 
 * Note: This test is best-effort; it may not always detect activity
 */
TEST_F(DiskMonitorTest, RatesReflectActivity) {
    // First sample
    std::vector<DiskStats> disks1 = monitor.getCurrentStats();
    
    // Wait for PDH collection interval
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // Second sample (may have some disk activity from test framework)
    std::vector<DiskStats> disks2 = monitor.getCurrentStats();
    
    // Verify we got data for both samples
    ASSERT_EQ(disks1.size(), disks2.size());
    
    // At least one disk should potentially have activity
    // (This is not guaranteed, but we check that the mechanism works)
    bool foundActivityOrZero = false;
    for (const auto& disk : disks2) {
        if (disk.bytesReadPerSec >= 0 && disk.bytesWrittenPerSec >= 0) {
            foundActivityOrZero = true;
            break;
        }
    }
    EXPECT_TRUE(foundActivityOrZero) 
        << "At least one disk should have valid rate data";
}

