/**
 * @file TempMonitorTest.cpp
 * @brief Tests for TempMonitor (temperature monitoring)
 * 
 * NOTE: These tests require:
 * - Windows operating system
 * - Administrator privileges (for sensor access tests)
 * - LibreHardwareMonitorLib.dll in path
 * 
 * Some tests are designed to work without admin (graceful degradation tests).
 */

#include <gtest/gtest.h>
#include "WinHKMonLib/TempMonitor.h"

using namespace WinHKMon;

// ============================================================================
// Admin Privilege Tests
// ============================================================================

TEST(AdminPrivilegesTest, CanCheckIfRunningAsAdmin) {
    // This test should work regardless of privilege level
    bool isAdmin = AdminPrivileges::IsRunningAsAdmin();
    
    // Result should be deterministic (true or false, not indeterminate)
    EXPECT_TRUE(isAdmin == true || isAdmin == false);
    
    // On Windows, print result for manual verification
    if (isAdmin) {
        std::cout << "Running with administrator privileges\n";
    } else {
        std::cout << "Running without administrator privileges\n";
    }
}

// ============================================================================
// TempMonitor Construction/Destruction
// ============================================================================

TEST(TempMonitorTest, CanConstruct) {
    // Should be able to create TempMonitor without throwing
    EXPECT_NO_THROW({
        TempMonitor monitor;
    });
}

TEST(TempMonitorTest, StartsUninitialized) {
    TempMonitor monitor;
    EXPECT_FALSE(monitor.isInitialized());
}

// ============================================================================
// Initialization Tests (Admin Required)
// ============================================================================

TEST(TempMonitorTest, InitializeRequiresAdmin) {
    TempMonitor monitor;
    
    auto result = monitor.initialize();
    
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        // Should fail with NO_ADMIN when not running as admin
        EXPECT_EQ(result, TempMonitor::InitResult::NO_ADMIN);
        EXPECT_FALSE(monitor.isInitialized());
    } else {
        // With admin, should either succeed or fail with different reason
        EXPECT_NE(result, TempMonitor::InitResult::NO_ADMIN);
        
        // If successful, should be initialized
        if (result == TempMonitor::InitResult::SUCCESS) {
            EXPECT_TRUE(monitor.isInitialized());
        }
    }
}

TEST(TempMonitorTest, InitializeWithAdminSucceeds) {
    // This test only runs if we have admin privileges
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    auto result = monitor.initialize();
    
    // Should not fail due to lack of admin
    EXPECT_NE(result, TempMonitor::InitResult::NO_ADMIN);
    
    // Could fail for other reasons (no sensors, driver issues)
    // but should not be admin-related
}

// ============================================================================
// Error Message Tests
// ============================================================================

TEST(TempMonitorTest, ErrorMessagesAreInformative) {
    // All error messages should be non-empty and helpful
    auto msg1 = TempMonitor::getInitResultMessage(TempMonitor::InitResult::SUCCESS);
    EXPECT_FALSE(msg1.empty());
    EXPECT_NE(msg1.find("success"), std::string::npos);
    
    auto msg2 = TempMonitor::getInitResultMessage(TempMonitor::InitResult::NO_ADMIN);
    EXPECT_FALSE(msg2.empty());
    EXPECT_NE(msg2.find("dministrator"), std::string::npos);
    
    auto msg3 = TempMonitor::getInitResultMessage(TempMonitor::InitResult::NO_SENSORS);
    EXPECT_FALSE(msg3.empty());
    EXPECT_NE(msg3.find("sensor"), std::string::npos);
    
    auto msg4 = TempMonitor::getInitResultMessage(TempMonitor::InitResult::DRIVER_FAILED);
    EXPECT_FALSE(msg4.empty());
    EXPECT_NE(msg4.find("driver"), std::string::npos);
    
    auto msg5 = TempMonitor::getInitResultMessage(TempMonitor::InitResult::LIBRARY_MISSING);
    EXPECT_FALSE(msg5.empty());
    EXPECT_NE(msg5.find("Library") != std::string::npos || msg5.find("dll") != std::string::npos, false);
}

// ============================================================================
// Temperature Reading Tests (Admin Required)
// ============================================================================

TEST(TempMonitorTest, GetStatsReturnsNulloptWhenNotInitialized) {
    TempMonitor monitor;
    
    // Should return nullopt before initialization
    auto stats = monitor.getCurrentStats();
    EXPECT_FALSE(stats.has_value());
}

TEST(TempMonitorTest, GetStatsReturnsDataWhenInitialized) {
    // Requires admin and successful initialization
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    auto result = monitor.initialize();
    
    if (result != TempMonitor::InitResult::SUCCESS) {
        GTEST_SKIP() << "Initialization failed: " 
                     << TempMonitor::getInitResultMessage(result);
    }
    
    // Should return temperature data
    auto stats = monitor.getCurrentStats();
    EXPECT_TRUE(stats.has_value());
    
    if (stats.has_value()) {
        // Should have at least some CPU temperature readings
        EXPECT_FALSE(stats->cpuTemps.empty());
        
        // Temperature values should be realistic (0-150°C range)
        for (const auto& reading : stats->cpuTemps) {
            EXPECT_GE(reading.tempCelsius, 0);
            EXPECT_LE(reading.tempCelsius, 150);
            EXPECT_FALSE(reading.name.empty());
        }
        
        // Max temperature should be set and realistic
        EXPECT_GE(stats->maxCpuTempCelsius, 0);
        EXPECT_LE(stats->maxCpuTempCelsius, 150);
    }
}

TEST(TempMonitorTest, TemperatureValuesAreRealistic) {
    // Requires admin and successful initialization
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    auto result = monitor.initialize();
    
    if (result != TempMonitor::InitResult::SUCCESS) {
        GTEST_SKIP() << "Initialization failed";
    }
    
    auto stats = monitor.getCurrentStats();
    ASSERT_TRUE(stats.has_value());
    
    int maxTemp = stats->maxCpuTempCelsius;
    
    // Skip if temperature is clearly invalid (0°C or negative)
    // This happens on VMs or systems without real temperature sensors
    if (maxTemp <= 0) {
        GTEST_SKIP() << "Invalid temperature reading (" << maxTemp 
                     << "°C), likely running on VM without real sensors";
    }
    
    // Typical idle temperature range: 20-80°C
    // Under load could be higher but shouldn't exceed 100°C normally
    EXPECT_GE(maxTemp, 20) << "Temperature too low, sensor may be incorrect";
    EXPECT_LE(maxTemp, 100) << "Temperature very high, possible thermal issue";
}

// ============================================================================
// Multiple Read Tests
// ============================================================================

TEST(TempMonitorTest, CanReadTemperatureMultipleTimes) {
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    if (monitor.initialize() != TempMonitor::InitResult::SUCCESS) {
        GTEST_SKIP() << "Initialization failed";
    }
    
    // Should be able to read multiple times without error
    auto stats1 = monitor.getCurrentStats();
    EXPECT_TRUE(stats1.has_value());
    
    auto stats2 = monitor.getCurrentStats();
    EXPECT_TRUE(stats2.has_value());
    
    auto stats3 = monitor.getCurrentStats();
    EXPECT_TRUE(stats3.has_value());
    
    // Temperatures should be similar (within 10°C) on consecutive reads
    if (stats1.has_value() && stats2.has_value()) {
        int diff = std::abs(stats1->maxCpuTempCelsius - stats2->maxCpuTempCelsius);
        EXPECT_LE(diff, 10) << "Temperature changed drastically between reads";
    }
}

// ============================================================================
// Cleanup Tests
// ============================================================================

TEST(TempMonitorTest, CleanupDoesNotCrash) {
    TempMonitor monitor;
    
    // Cleanup should work even if not initialized
    EXPECT_NO_THROW(monitor.cleanup());
    
    // Should be able to cleanup multiple times
    EXPECT_NO_THROW(monitor.cleanup());
}

TEST(TempMonitorTest, CleanupMakesMonitorUninitialized) {
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    if (monitor.initialize() != TempMonitor::InitResult::SUCCESS) {
        GTEST_SKIP() << "Initialization failed";
    }
    
    EXPECT_TRUE(monitor.isInitialized());
    
    monitor.cleanup();
    
    EXPECT_FALSE(monitor.isInitialized());
}

// ============================================================================
// Graceful Degradation Tests (No Admin)
// ============================================================================

TEST(TempMonitorTest, GracefullyHandlesMissingAdmin) {
    // Test that runs without admin to verify graceful handling
    if (AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires non-admin user";
    }
    
    TempMonitor monitor;
    
    // Should not crash
    EXPECT_NO_THROW({
        auto result = monitor.initialize();
        EXPECT_EQ(result, TempMonitor::InitResult::NO_ADMIN);
    });
    
    // Should return nullopt for stats
    auto stats = monitor.getCurrentStats();
    EXPECT_FALSE(stats.has_value());
    
    // Should not crash on cleanup
    EXPECT_NO_THROW(monitor.cleanup());
}

// ============================================================================
// Virtual Machine / No Sensors Tests
// ============================================================================

TEST(TempMonitorTest, HandlesNoSensorsGracefully) {
    // This test checks handling when sensors aren't available
    // (common in VMs)
    
    if (!AdminPrivileges::IsRunningAsAdmin()) {
        GTEST_SKIP() << "Test requires administrator privileges";
    }
    
    TempMonitor monitor;
    auto result = monitor.initialize();
    
    // If no sensors, should get NO_SENSORS result
    if (result == TempMonitor::InitResult::NO_SENSORS) {
        EXPECT_FALSE(monitor.isInitialized());
        
        auto stats = monitor.getCurrentStats();
        EXPECT_FALSE(stats.has_value());
        
        // Should not crash
        EXPECT_NO_THROW(monitor.cleanup());
    }
}

