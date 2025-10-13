#include "WinHKMonLib/StateManager.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace WinHKMon;

class StateManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use temporary test directory
        testStatePath = std::filesystem::temp_directory_path() / "WinHKMon_test.dat";
        stateManager = std::make_unique<StateManager>("WinHKMon_test");
    }

    void TearDown() override {
        // Clean up test file
        if (std::filesystem::exists(testStatePath)) {
            std::filesystem::remove(testStatePath);
        }
    }

    std::filesystem::path testStatePath;
    std::unique_ptr<StateManager> stateManager;
};

// Test save and load round-trip
TEST_F(StateManagerTest, SaveAndLoadRoundTrip) {
    SystemMetrics metrics;
    metrics.timestamp = 1234567890;
    
    // Setup network stats
    std::vector<InterfaceStats> network;
    InterfaceStats iface;
    iface.name = "Ethernet";
    iface.totalInOctets = 1000000000;
    iface.totalOutOctets = 500000000;
    network.push_back(iface);
    metrics.network = network;
    
    // Setup disk stats
    std::vector<DiskStats> disks;
    DiskStats disk;
    disk.deviceName = "0 C:";
    disk.totalBytesRead = 5000000000;
    disk.totalBytesWritten = 2500000000;
    disks.push_back(disk);
    metrics.disks = disks;
    
    // Save
    ASSERT_TRUE(stateManager->save(metrics));
    
    // Load
    SystemMetrics loadedMetrics;
    uint64_t loadedTimestamp;
    ASSERT_TRUE(stateManager->load(loadedMetrics, loadedTimestamp));
    
    // Verify
    EXPECT_EQ(loadedTimestamp, 1234567890);
    ASSERT_TRUE(loadedMetrics.network.has_value());
    EXPECT_EQ((*loadedMetrics.network)[0].totalInOctets, 1000000000);
    EXPECT_EQ((*loadedMetrics.network)[0].totalOutOctets, 500000000);
    ASSERT_TRUE(loadedMetrics.disks.has_value());
    EXPECT_EQ((*loadedMetrics.disks)[0].totalBytesRead, 5000000000);
    EXPECT_EQ((*loadedMetrics.disks)[0].totalBytesWritten, 2500000000);
}

// Test missing state file (first run)
TEST_F(StateManagerTest, HandMissingStateFile) {
    // Ensure no state file exists
    if (std::filesystem::exists(testStatePath)) {
        std::filesystem::remove(testStatePath);
    }
    
    SystemMetrics metrics;
    uint64_t timestamp;
    
    // Load should return false but not throw
    EXPECT_FALSE(stateManager->load(metrics, timestamp));
}

// Test corrupted state file handling
TEST_F(StateManagerTest, HandleCorruptedStateFile) {
    // Create corrupted file
    std::ofstream file(testStatePath);
    file << "CORRUPTED DATA\n";
    file << "INVALID FORMAT\n";
    file.close();
    
    SystemMetrics metrics;
    uint64_t timestamp;
    
    // Load should return false but not throw
    EXPECT_FALSE(stateManager->load(metrics, timestamp));
}

// Test version mismatch handling
TEST_F(StateManagerTest, HandleVersionMismatch) {
    // Create file with old version
    std::ofstream file(testStatePath);
    file << "VERSION 0.5\n";
    file << "TIMESTAMP 1234567890\n";
    file.close();
    
    SystemMetrics metrics;
    uint64_t timestamp;
    
    // Load should handle gracefully (might return false or migrate)
    // Implementation can choose to ignore old versions or migrate
    stateManager->load(metrics, timestamp);  // Should not throw
}

// Test empty metrics save
TEST_F(StateManagerTest, SaveEmptyMetrics) {
    SystemMetrics metrics;
    metrics.timestamp = 1234567890;
    
    // Save with no metrics
    ASSERT_TRUE(stateManager->save(metrics));
    
    // Load should work
    SystemMetrics loadedMetrics;
    uint64_t loadedTimestamp;
    ASSERT_TRUE(stateManager->load(loadedMetrics, loadedTimestamp));
    EXPECT_EQ(loadedTimestamp, 1234567890);
}

// Test multiple interfaces and disks
TEST_F(StateManagerTest, SaveMultipleDevices) {
    SystemMetrics metrics;
    metrics.timestamp = 1234567890;
    
    // Multiple network interfaces
    std::vector<InterfaceStats> network;
    InterfaceStats iface1;
    iface1.name = "Ethernet";
    iface1.totalInOctets = 1000000000;
    iface1.totalOutOctets = 500000000;
    network.push_back(iface1);
    
    InterfaceStats iface2;
    iface2.name = "Wi-Fi";
    iface2.totalInOctets = 2000000000;
    iface2.totalOutOctets = 1000000000;
    network.push_back(iface2);
    metrics.network = network;
    
    // Multiple disks
    std::vector<DiskStats> disks;
    DiskStats disk1;
    disk1.deviceName = "0 C:";
    disk1.totalBytesRead = 5000000000;
    disk1.totalBytesWritten = 2500000000;
    disks.push_back(disk1);
    
    DiskStats disk2;
    disk2.deviceName = "1 D:";
    disk2.totalBytesRead = 3000000000;
    disk2.totalBytesWritten = 1500000000;
    disks.push_back(disk2);
    metrics.disks = disks;
    
    // Save and load
    ASSERT_TRUE(stateManager->save(metrics));
    
    SystemMetrics loadedMetrics;
    uint64_t loadedTimestamp;
    ASSERT_TRUE(stateManager->load(loadedMetrics, loadedTimestamp));
    
    // Verify all devices loaded
    ASSERT_TRUE(loadedMetrics.network.has_value());
    EXPECT_EQ(loadedMetrics.network->size(), 2);
    ASSERT_TRUE(loadedMetrics.disks.has_value());
    EXPECT_EQ(loadedMetrics.disks->size(), 2);
}

// Test overwrite existing file
TEST_F(StateManagerTest, OverwriteExistingFile) {
    SystemMetrics metrics1;
    metrics1.timestamp = 1111111111;
    stateManager->save(metrics1);
    
    SystemMetrics metrics2;
    metrics2.timestamp = 2222222222;
    stateManager->save(metrics2);
    
    // Load should get the latest
    SystemMetrics loadedMetrics;
    uint64_t loadedTimestamp;
    ASSERT_TRUE(stateManager->load(loadedMetrics, loadedTimestamp));
    EXPECT_EQ(loadedTimestamp, 2222222222);
}

// Test interface names with special characters
TEST_F(StateManagerTest, HandleSpecialCharactersInNames) {
    SystemMetrics metrics;
    metrics.timestamp = 1234567890;
    
    std::vector<InterfaceStats> network;
    InterfaceStats iface;
    iface.name = "Ethernet \"Test\" 2";
    iface.totalInOctets = 1000000000;
    iface.totalOutOctets = 500000000;
    network.push_back(iface);
    metrics.network = network;
    
    std::vector<DiskStats> disks;
    DiskStats disk;
    disk.deviceName = "0 C: System";
    disk.totalBytesRead = 5000000000;
    disk.totalBytesWritten = 2500000000;
    disks.push_back(disk);
    metrics.disks = disks;
    
    // Save and load
    ASSERT_TRUE(stateManager->save(metrics));
    
    SystemMetrics loadedMetrics;
    uint64_t loadedTimestamp;
    ASSERT_TRUE(stateManager->load(loadedMetrics, loadedTimestamp));
    
    // Verify names with sanitized spaces (space->underscore for file format)
    ASSERT_TRUE(loadedMetrics.network.has_value());
    EXPECT_EQ((*loadedMetrics.network)[0].name, "Ethernet_\"Test\"_2");
    ASSERT_TRUE(loadedMetrics.disks.has_value());
    EXPECT_EQ((*loadedMetrics.disks)[0].deviceName, "0_C:_System");
}

// Test state file location
TEST_F(StateManagerTest, StateFileInTempDirectory) {
    SystemMetrics metrics;
    metrics.timestamp = 1234567890;
    
    stateManager->save(metrics);
    
    // Verify file exists in temp directory
    EXPECT_TRUE(std::filesystem::exists(testStatePath));
    
    // Verify it's in the temp directory (compare string representations)
    auto parentPath = testStatePath.parent_path().string();
    auto tempPath = std::filesystem::temp_directory_path().string();
    
    // Remove trailing slashes for comparison
    if (!parentPath.empty() && (parentPath.back() == '/' || parentPath.back() == '\\')) {
        parentPath.pop_back();
    }
    if (!tempPath.empty() && (tempPath.back() == '/' || tempPath.back() == '\\')) {
        tempPath.pop_back();
    }
    
    EXPECT_EQ(parentPath, tempPath);
}

