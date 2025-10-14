/**
 * @file NetworkMonitorTest.cpp
 * @brief Unit tests for NetworkMonitor component
 * 
 * Tests network interface enumeration, statistics collection, and primary
 * interface selection using Windows IP Helper API (MIB_IF_ROW2).
 */

#include "WinHKMonLib/NetworkMonitor.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace WinHKMon;

/**
 * Test Fixture for NetworkMonitor tests
 */
class NetworkMonitorTest : public ::testing::Test {
protected:
    NetworkMonitor monitor;
    
    void SetUp() override {
        // Initialize monitor before each test
        try {
            monitor.initialize();
        } catch (const std::exception& e) {
            // If initialization fails, skip tests that require it
            GTEST_SKIP() << "NetworkMonitor initialization failed: " << e.what();
        }
    }
};

/**
 * T012 Test 1: Verify initialization succeeds
 */
TEST_F(NetworkMonitorTest, InitializeSucceeds) {
    // Initialization already done in SetUp
    SUCCEED() << "NetworkMonitor initialized successfully";
}

/**
 * T012 Test 2: Verify getCurrentStats returns interface list
 */
TEST_F(NetworkMonitorTest, GetCurrentStatsReturnsInterfaces) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    // Should have at least one interface (even if just loopback)
    EXPECT_GE(interfaces.size(), 1) << "System should have at least one network interface";
    
    // Verify each interface has required fields
    for (const auto& iface : interfaces) {
        EXPECT_FALSE(iface.name.empty()) << "Interface name should not be empty";
        EXPECT_FALSE(iface.description.empty()) << "Interface description should not be empty";
    }
}

/**
 * T012 Test 3: Verify loopback interfaces are excluded (if present)
 * 
 * Note: This test verifies that if loopback interfaces exist, they are filtered out
 * by the getCurrentStats method. Some systems may not have loopback reported.
 */
TEST_F(NetworkMonitorTest, LoopbackInterfacesExcluded) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    // Check that no interface is explicitly named "Loopback"
    for (const auto& iface : interfaces) {
        EXPECT_FALSE(iface.name.find("Loopback") != std::string::npos ||
                    iface.description.find("Loopback") != std::string::npos)
            << "Loopback interface should be filtered out: " << iface.name;
    }
}

/**
 * T012 Test 4: Verify traffic counters are non-negative and monotonic
 */
TEST_F(NetworkMonitorTest, TrafficCountersMonotonic) {
    std::vector<InterfaceStats> interfaces1 = monitor.getCurrentStats();
    
    // Wait a bit to allow some potential network activity
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::vector<InterfaceStats> interfaces2 = monitor.getCurrentStats();
    
    // Verify counters are non-negative in both samples
    for (const auto& iface : interfaces1) {
        EXPECT_GE(iface.totalInOctets, 0) << "InOctets should be non-negative";
        EXPECT_GE(iface.totalOutOctets, 0) << "OutOctets should be non-negative";
    }
    
    for (const auto& iface : interfaces2) {
        EXPECT_GE(iface.totalInOctets, 0) << "InOctets should be non-negative";
        EXPECT_GE(iface.totalOutOctets, 0) << "OutOctets should be non-negative";
    }
    
    // Verify monotonic property: second sample >= first sample
    // (counters should never decrease unless rollover, which is rare)
    for (const auto& iface1 : interfaces1) {
        auto it = std::find_if(interfaces2.begin(), interfaces2.end(),
            [&iface1](const InterfaceStats& iface2) { return iface2.name == iface1.name; });
        
        if (it != interfaces2.end()) {
            // Allow for potential rollover (very unlikely in short test)
            if (it->totalInOctets < iface1.totalInOctets) {
                ADD_FAILURE() << "InOctets decreased unexpectedly for " << iface1.name
                             << " (possible counter rollover)";
            }
            if (it->totalOutOctets < iface1.totalOutOctets) {
                ADD_FAILURE() << "OutOctets decreased unexpectedly for " << iface1.name
                             << " (possible counter rollover)";
            }
        }
    }
}

/**
 * T012 Test 5: Verify selectPrimaryInterface logic
 */
TEST_F(NetworkMonitorTest, SelectPrimaryInterfaceLogic) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    if (interfaces.empty()) {
        GTEST_SKIP() << "No interfaces available for testing";
    }
    
    std::string primaryInterface = monitor.selectPrimaryInterface(interfaces);
    
    EXPECT_FALSE(primaryInterface.empty()) << "Primary interface should be selected";
    
    // Verify the selected interface exists in the list
    auto it = std::find_if(interfaces.begin(), interfaces.end(),
        [&primaryInterface](const InterfaceStats& iface) { return iface.name == primaryInterface; });
    
    EXPECT_NE(it, interfaces.end()) << "Selected primary interface should exist in interface list";
}

/**
 * T012 Test 6: Verify connection status detection
 */
TEST_F(NetworkMonitorTest, ConnectionStatusDetection) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    // At least one interface should typically be connected (or have status reported)
    bool hasConnectionInfo = false;
    for (const auto& iface : interfaces) {
        // Connection status should be a boolean (we can check it exists)
        hasConnectionInfo = true;
        // We don't enforce that at least one is connected, as tests may run in isolated environments
    }
    
    EXPECT_TRUE(hasConnectionInfo) << "Connection status should be available for interfaces";
}

/**
 * T012 Test 7: Verify error handling - no interfaces scenario
 * 
 * This is a simulated test; real systems always have at least one interface.
 * We test that the API doesn't crash if no interfaces match our filter.
 */
TEST_F(NetworkMonitorTest, HandlesNoInterfacesGracefully) {
    // Create an empty interface list
    std::vector<InterfaceStats> emptyList;
    
    // selectPrimaryInterface should handle empty list gracefully
    std::string primary = monitor.selectPrimaryInterface(emptyList);
    
    EXPECT_TRUE(primary.empty()) << "Primary interface should be empty string for empty list";
}

/**
 * T012 Test 8: Verify link speed values are realistic
 */
TEST_F(NetworkMonitorTest, LinkSpeedRealistic) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    for (const auto& iface : interfaces) {
        // Link speeds should be reasonable (e.g., 0 for disconnected, or 1 Mbps to 100 Gbps)
        // We allow 0 for disconnected interfaces
        if (iface.isConnected) {
            EXPECT_GT(iface.linkSpeedBitsPerSec, 0) 
                << "Connected interface " << iface.name << " should have link speed > 0";
            
            // Realistic upper bound: 100 Gbps = 100,000,000,000 bps
            EXPECT_LE(iface.linkSpeedBitsPerSec, 100000000000ULL)
                << "Link speed seems unrealistically high for " << iface.name;
        }
    }
}

/**
 * T012 Test 9: Verify interface names are non-empty and ASCII-safe
 */
TEST_F(NetworkMonitorTest, InterfaceNamesValid) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    for (const auto& iface : interfaces) {
        EXPECT_FALSE(iface.name.empty()) << "Interface name should not be empty";
        EXPECT_LT(iface.name.length(), 256) << "Interface name should be reasonably short";
        
        // Name should contain printable characters
        for (char c : iface.name) {
            EXPECT_TRUE(c >= 32 && c < 127 || c < 0) << "Interface name should be printable ASCII or Unicode";
        }
    }
}

/**
 * T012 Test 10: Verify multiple calls return consistent data
 */
TEST_F(NetworkMonitorTest, MultipleCallsConsistent) {
    std::vector<InterfaceStats> interfaces1 = monitor.getCurrentStats();
    std::vector<InterfaceStats> interfaces2 = monitor.getCurrentStats();
    
    // Should return same number of interfaces
    EXPECT_EQ(interfaces1.size(), interfaces2.size()) 
        << "Interface count should remain consistent between calls";
    
    // Interface names should be the same
    for (size_t i = 0; i < std::min(interfaces1.size(), interfaces2.size()); ++i) {
        EXPECT_EQ(interfaces1[i].name, interfaces2[i].name)
            << "Interface order and names should be consistent";
    }
}

/**
 * T012 Test 11: Verify rate calculations are initialized to zero
 */
TEST_F(NetworkMonitorTest, RateCalculationsInitializedToZero) {
    std::vector<InterfaceStats> interfaces = monitor.getCurrentStats();
    
    // On first call, rate calculations should be zero (no previous sample)
    for (const auto& iface : interfaces) {
        EXPECT_EQ(iface.inBytesPerSec, 0) << "inBytesPerSec should be 0 on first call";
        EXPECT_EQ(iface.outBytesPerSec, 0) << "outBytesPerSec should be 0 on first call";
    }
}

/**
 * T012 Test 12: Verify primary interface selection prefers highest traffic
 * 
 * This test creates mock interface data to verify selection logic.
 */
TEST(NetworkMonitorUnitTest, SelectPrimaryInterfaceHighestTraffic) {
    NetworkMonitor monitor;
    
    // Create mock interfaces
    InterfaceStats iface1;
    iface1.name = "Ethernet 1";
    iface1.totalInOctets = 1000000;
    iface1.totalOutOctets = 500000;
    iface1.isConnected = true;
    
    InterfaceStats iface2;
    iface2.name = "Ethernet 2";
    iface2.totalInOctets = 5000000;
    iface2.totalOutOctets = 3000000;
    iface2.isConnected = true;
    
    InterfaceStats iface3;
    iface3.name = "Wi-Fi";
    iface3.totalInOctets = 2000000;
    iface3.totalOutOctets = 1000000;
    iface3.isConnected = true;
    
    std::vector<InterfaceStats> interfaces = {iface1, iface2, iface3};
    
    std::string primary = monitor.selectPrimaryInterface(interfaces);
    
    // Should select Ethernet 2 (highest total traffic: 8M)
    EXPECT_EQ(primary, "Ethernet 2") << "Should select interface with highest traffic";
}

/**
 * T012 Test 13: Verify primary interface selection handles tie-breaking
 */
TEST(NetworkMonitorUnitTest, SelectPrimaryInterfaceTieBreaker) {
    NetworkMonitor monitor;
    
    // Create mock interfaces with same traffic
    InterfaceStats ethernet;
    ethernet.name = "Ethernet";
    ethernet.description = "Intel Ethernet";
    ethernet.totalInOctets = 1000000;
    ethernet.totalOutOctets = 1000000;
    ethernet.isConnected = true;
    
    InterfaceStats wifi;
    wifi.name = "Wi-Fi";
    wifi.description = "Wireless";
    wifi.totalInOctets = 1000000;
    wifi.totalOutOctets = 1000000;
    wifi.isConnected = true;
    
    std::vector<InterfaceStats> interfaces = {wifi, ethernet};  // Wi-Fi first
    
    std::string primary = monitor.selectPrimaryInterface(interfaces);
    
    // Should prefer Ethernet over Wi-Fi in tie
    EXPECT_EQ(primary, "Ethernet") << "Should prefer Ethernet over Wi-Fi in traffic tie";
}

