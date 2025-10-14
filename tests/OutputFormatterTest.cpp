#include "WinHKMonLib/OutputFormatter.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace WinHKMon;

// Helper to create default CLI options for testing
CliOptions createDefaultOptions() {
    CliOptions opts;
    opts.showCpu = true;
    opts.showMemory = true;
    opts.showDiskSpace = true;  // Show disk space by default in tests
    opts.showDiskIO = true;     // Show disk I/O by default in tests
    opts.showNetwork = true;
    return opts;
}

// Helper to create sample metrics
SystemMetrics createSampleMetrics() {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    
    // CPU stats
    CpuStats cpu;
    cpu.totalUsagePercent = 23.5;
    cpu.averageFrequencyMhz = 2400;
    cpu.cores.push_back({0, 45.0, 2800});
    cpu.cores.push_back({1, 12.0, 2100});
    metrics.cpu = cpu;
    
    // Memory stats
    MemoryStats mem;
    mem.totalPhysicalBytes = 16ULL * 1024 * 1024 * 1024;  // 16 GB
    mem.availablePhysicalBytes = 8ULL * 1024 * 1024 * 1024;  // 8 GB
    mem.usedPhysicalBytes = 8ULL * 1024 * 1024 * 1024;  // 8 GB
    mem.usagePercent = 50.0;
    mem.totalPageFileBytes = 8ULL * 1024 * 1024 * 1024;  // 8 GB
    mem.usedPageFileBytes = 2ULL * 1024 * 1024 * 1024;  // 2 GB
    mem.availablePageFileBytes = 6ULL * 1024 * 1024 * 1024;  // 6 GB
    mem.pageFilePercent = 25.0;
    metrics.memory = mem;
    
    return metrics;
}

// Test text format generation
TEST(OutputFormatterTest, FormatTextCompact) {
    SystemMetrics metrics = createSampleMetrics();
    std::string output = formatText(metrics, false, createDefaultOptions());
    
    EXPECT_NE(output.find("CPU:"), std::string::npos);
    EXPECT_NE(output.find("23.5"), std::string::npos);
    EXPECT_NE(output.find("2.4"), std::string::npos);  // 2400 MHz -> 2.4 GHz
    EXPECT_NE(output.find("RAM:"), std::string::npos);
    EXPECT_NE(output.find("8192"), std::string::npos);  // 8 GB in MB
}

TEST(OutputFormatterTest, FormatTextSingleLine) {
    SystemMetrics metrics = createSampleMetrics();
    std::string output = formatText(metrics, true, createDefaultOptions());
    
    // Single line should not contain newlines (except trailing)
    size_t newlineCount = 0;
    for (char c : output) {
        if (c == '\n') newlineCount++;
    }
    EXPECT_LE(newlineCount, 1);  // At most one trailing newline
}

TEST(OutputFormatterTest, HandlesOptionalMetrics) {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    // Only CPU, no memory
    CpuStats cpu;
    cpu.totalUsagePercent = 23.5;
    cpu.averageFrequencyMhz = 2400;
    metrics.cpu = cpu;
    
    std::string output = formatText(metrics, false, createDefaultOptions());
    
    EXPECT_NE(output.find("CPU:"), std::string::npos);
    EXPECT_EQ(output.find("RAM:"), std::string::npos);  // RAM should not appear
}

// Test JSON format generation
TEST(OutputFormatterTest, FormatJsonValidStructure) {
    SystemMetrics metrics = createSampleMetrics();
    std::string json = formatJson(metrics, createDefaultOptions());
    
    EXPECT_NE(json.find("\"schemaVersion\""), std::string::npos);
    EXPECT_NE(json.find("\"timestamp\""), std::string::npos);
    EXPECT_NE(json.find("\"cpu\""), std::string::npos);
    EXPECT_NE(json.find("\"memory\""), std::string::npos);
    EXPECT_NE(json.find("\"totalUsagePercent\""), std::string::npos);
}

TEST(OutputFormatterTest, FormatJsonValidJson) {
    SystemMetrics metrics = createSampleMetrics();
    std::string json = formatJson(metrics, createDefaultOptions());
    
    // Basic JSON validation: balanced braces
    int braceCount = 0;
    for (char c : json) {
        if (c == '{') braceCount++;
        if (c == '}') braceCount--;
    }
    EXPECT_EQ(braceCount, 0);
    
    // Should start and end with braces
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');
}

TEST(OutputFormatterTest, FormatJsonOptionalMetrics) {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    CpuStats cpu;
    cpu.totalUsagePercent = 23.5;
    cpu.averageFrequencyMhz = 2400;
    metrics.cpu = cpu;
    
    std::string json = formatJson(metrics, createDefaultOptions());
    
    EXPECT_NE(json.find("\"cpu\""), std::string::npos);
    EXPECT_EQ(json.find("\"memory\""), std::string::npos);  // No memory field
}

TEST(OutputFormatterTest, FormatJsonEscapesStrings) {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    
    // Network interface with quotes in name
    std::vector<InterfaceStats> interfaces;
    InterfaceStats iface;
    iface.name = "Ethernet \"Test\"";
    iface.description = "Test\\Device";
    iface.isConnected = true;
    iface.linkSpeedBitsPerSec = 1000000000;
    iface.inBytesPerSec = 1000000;
    iface.outBytesPerSec = 500000;
    iface.totalInOctets = 1000000000;
    iface.totalOutOctets = 500000000;
    interfaces.push_back(iface);
    metrics.network = interfaces;
    
    std::string json = formatJson(metrics, createDefaultOptions());
    
    // Should escape quotes and backslashes
    EXPECT_NE(json.find("\\\"Test\\\""), std::string::npos);
    EXPECT_NE(json.find("Test\\\\Device"), std::string::npos);
}

// Test CSV format generation
TEST(OutputFormatterTest, FormatCsvWithHeader) {
    SystemMetrics metrics = createSampleMetrics();
    std::string csv = formatCsv(metrics, true, createDefaultOptions());
    
    EXPECT_NE(csv.find("timestamp"), std::string::npos);
    EXPECT_NE(csv.find("cpu_percent"), std::string::npos);
    EXPECT_NE(csv.find("cpu_mhz"), std::string::npos);
    EXPECT_NE(csv.find("ram_available_mb"), std::string::npos);
}

TEST(OutputFormatterTest, FormatCsvWithoutHeader) {
    SystemMetrics metrics = createSampleMetrics();
    std::string csv = formatCsv(metrics, false, createDefaultOptions());
    
    // Should not contain header field names
    EXPECT_EQ(csv.find("timestamp"), std::string::npos);
    EXPECT_EQ(csv.find("cpu_percent"), std::string::npos);
    
    // But should contain data values
    EXPECT_NE(csv.find("23.5"), std::string::npos);
    EXPECT_NE(csv.find("2400"), std::string::npos);
}

TEST(OutputFormatterTest, FormatCsvHandlesCommas) {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    
    // Disk with comma in name
    std::vector<DiskStats> disks;
    DiskStats disk;
    disk.deviceName = "0 C:, System";
    disk.totalSizeBytes = 1000000000000;
    disk.usedBytes = 600000000000;   // 600 GB used
    disk.freeBytes = 400000000000;   // 400 GB free
    disk.bytesReadPerSec = 1000000;
    disk.bytesWrittenPerSec = 500000;
    disk.percentBusy = 25.5;
    disk.totalBytesRead = 1000000000;
    disk.totalBytesWritten = 500000000;
    disks.push_back(disk);
    metrics.disks = disks;
    
    std::string csv = formatCsv(metrics, true, createDefaultOptions());
    
    // Name with comma should be quoted
    EXPECT_NE(csv.find("\"0 C:, System\""), std::string::npos);
}

TEST(OutputFormatterTest, FormatCsvMultipleRows) {
    SystemMetrics metrics = createSampleMetrics();
    
    std::string row1 = formatCsv(metrics, true, createDefaultOptions());   // With header
    std::string row2 = formatCsv(metrics, false, createDefaultOptions());  // Without header
    
    // Row 1 should have more lines than row 2
    size_t lines1 = std::count(row1.begin(), row1.end(), '\n');
    size_t lines2 = std::count(row2.begin(), row2.end(), '\n');
    EXPECT_GT(lines1, lines2);
}

// Test Unicode symbol rendering
TEST(OutputFormatterTest, TextUsesUnicodeSymbols) {
    SystemMetrics metrics = createSampleMetrics();
    
    // Add network stats
    std::vector<InterfaceStats> interfaces;
    InterfaceStats iface;
    iface.name = "Ethernet";
    iface.isConnected = true;
    iface.linkSpeedBitsPerSec = 1000000000;
    iface.inBytesPerSec = 1000000;
    iface.outBytesPerSec = 500000;
    iface.totalInOctets = 1000000000;
    iface.totalOutOctets = 500000000;
    interfaces.push_back(iface);
    metrics.network = interfaces;
    
    std::string output = formatText(metrics, false, createDefaultOptions());
    
    // Should contain < and > direction indicators (ASCII-safe for Windows console)
    bool hasDirectionIndicators = (output.find("<") != std::string::npos && 
                                   output.find(">") != std::string::npos);
    
    EXPECT_TRUE(hasDirectionIndicators) << "Output should contain < and > direction indicators";
}

// Test handling of missing optional fields
TEST(OutputFormatterTest, HandlesEmptyMetrics) {
    SystemMetrics metrics;
    metrics.timestamp = 1000000;
    
    std::string text = formatText(metrics, false, createDefaultOptions());
    std::string json = formatJson(metrics, createDefaultOptions());
    std::string csv = formatCsv(metrics, true, createDefaultOptions());
    
    // Should not crash and should produce valid (though minimal) output
    EXPECT_FALSE(text.empty());
    EXPECT_FALSE(json.empty());
    EXPECT_FALSE(csv.empty());
}

