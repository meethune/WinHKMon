#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

/**
 * @file Types.h
 * @brief Core data structures for WinHKMon
 * 
 * This file defines all data structures used throughout the WinHKMon system,
 * based on the data model specification.
 */

namespace WinHKMon {

/**
 * @brief Per-core CPU statistics
 */
struct CoreStats {
    int coreId;                    ///< Logical processor ID (0-based)
    double usagePercent;           ///< Core usage percentage (0.0 - 100.0)
    uint64_t frequencyMhz;         ///< Current frequency in MHz
};

/**
 * @brief CPU usage and frequency information
 */
struct CpuStats {
    double totalUsagePercent;                ///< Overall CPU usage (0.0 - 100.0)
    std::vector<CoreStats> cores;            ///< Per-core statistics
    uint64_t averageFrequencyMhz;            ///< Average frequency across all cores
    
    // Optional breakdown (if available from API)
    std::optional<double> userPercent;       ///< User-mode CPU time
    std::optional<double> systemPercent;     ///< Kernel-mode CPU time
    std::optional<double> idlePercent;       ///< Idle time percentage
};

/**
 * @brief Physical and virtual memory statistics
 */
struct MemoryStats {
    uint64_t totalPhysicalBytes;             ///< Total RAM installed
    uint64_t availablePhysicalBytes;         ///< RAM available for allocation
    uint64_t usedPhysicalBytes;              ///< RAM currently in use (calculated)
    double usagePercent;                     ///< RAM usage percentage (0.0 - 100.0)
    
    uint64_t totalPageFileBytes;             ///< Total page file size
    uint64_t availablePageFileBytes;         ///< Page file available
    uint64_t usedPageFileBytes;              ///< Page file in use (calculated)
    double pageFilePercent;                  ///< Page file usage percentage (0.0 - 100.0)
    
    // Optional: cached memory breakdown
    std::optional<uint64_t> cachedBytes;     ///< File cache size
    std::optional<uint64_t> committedBytes;  ///< Committed memory
};

/**
 * @brief Disk statistics per physical drive (space + I/O)
 */
struct DiskStats {
    std::string deviceName;                  ///< Physical disk identifier (e.g., "C:", "D:")
    
    // Disk space (for DISK metric)
    uint64_t totalSizeBytes;                 ///< Disk capacity
    uint64_t usedBytes;                      ///< Used space
    uint64_t freeBytes;                      ///< Available space
    
    // Real-time I/O rates (for IO metric)
    uint64_t bytesReadPerSec;                ///< Current read rate
    uint64_t bytesWrittenPerSec;             ///< Current write rate
    double percentBusy;                      ///< Disk active time percentage (0.0 - 100.0)
    
    // Cumulative counters (since boot or monitor start)
    uint64_t totalBytesRead;                 ///< Cumulative reads
    uint64_t totalBytesWritten;              ///< Cumulative writes
    
    // Optional: IOPS
    std::optional<uint64_t> readsPerSec;     ///< Read operations per second
    std::optional<uint64_t> writesPerSec;    ///< Write operations per second
};

/**
 * @brief Network interface statistics
 */
struct InterfaceStats {
    std::string name;                        ///< Interface alias (e.g., "Ethernet", "Wi-Fi")
    std::string description;                 ///< Hardware description
    
    // Connection state
    bool isConnected;                        ///< Media connection state
    uint64_t linkSpeedBitsPerSec;            ///< Max negotiated link speed
    
    // Real-time rates
    uint64_t inBytesPerSec;                  ///< Current download rate
    uint64_t outBytesPerSec;                 ///< Current upload rate
    
    // Cumulative counters
    uint64_t totalInOctets;                  ///< Total bytes received
    uint64_t totalOutOctets;                 ///< Total bytes sent
    
    // Optional: packet-level stats
    std::optional<uint64_t> inPacketsPerSec;  ///< Packets received per second
    std::optional<uint64_t> outPacketsPerSec; ///< Packets sent per second
    std::optional<uint64_t> inErrors;         ///< Cumulative receive errors
    std::optional<uint64_t> outErrors;        ///< Cumulative transmit errors
};

/**
 * @brief Individual temperature sensor reading
 */
struct SensorReading {
    std::string name;                        ///< Sensor identifier (e.g., "CPU Core #0")
    int tempCelsius;                         ///< Temperature in Celsius
    std::string hardwareType;                ///< Hardware category ("CPU", "GPU", etc.)
};

/**
 * @brief Temperature sensor statistics
 */
struct TempStats {
    std::vector<SensorReading> cpuTemps;     ///< CPU sensor readings
    std::vector<SensorReading> gpuTemps;     ///< GPU sensor readings
    std::vector<SensorReading> otherTemps;   ///< Other sensor readings
    
    int maxCpuTempCelsius;                   ///< Maximum CPU temperature
    std::optional<int> minCpuTempCelsius;    ///< Minimum CPU temperature
    std::optional<int> avgCpuTempCelsius;    ///< Average CPU temperature
};

/**
 * @brief Central container for all collected metrics at a specific point in time
 */
struct SystemMetrics {
    std::optional<CpuStats> cpu;                          ///< CPU metrics (optional)
    std::optional<MemoryStats> memory;                    ///< Memory metrics (optional)
    std::optional<std::vector<DiskStats>> disks;          ///< Disk I/O metrics (optional)
    std::optional<std::vector<InterfaceStats>> network;   ///< Network metrics (optional)
    std::optional<TempStats> temperature;                 ///< Temperature metrics (optional)
    
    uint64_t timestamp;  ///< Monotonic timestamp (QueryPerformanceCounter)
};

/**
 * @brief Persistent state for delta calculations between runs
 */
struct MonitorState {
    std::string version;                     ///< State file format version (e.g., "1.0")
    uint64_t timestamp;                      ///< Previous sample timestamp
    
    // Previous counters for rate calculations (key = device/interface name)
    std::map<std::string, uint64_t> networkInOctets;   ///< Previous inbound bytes by interface
    std::map<std::string, uint64_t> networkOutOctets;  ///< Previous outbound bytes by interface
    std::map<std::string, uint64_t> diskBytesRead;     ///< Previous read bytes by disk
    std::map<std::string, uint64_t> diskBytesWritten;  ///< Previous write bytes by disk
};

/**
 * @brief Output format options
 */
enum class OutputFormat {
    TEXT,  ///< Human-readable multi-line text
    JSON,  ///< Structured JSON
    CSV    ///< Comma-separated values
};

/**
 * @brief Network speed unit preference
 */
enum class NetworkUnit {
    BITS,   ///< Display in bits/sec (Mbps, Gbps)
    BYTES   ///< Display in bytes/sec (MB/s, GB/s)
};

/**
 * @brief Parsed command-line options
 */
struct CliOptions {
    // Metrics to monitor
    bool showCpu = false;                    ///< Monitor CPU
    bool showMemory = false;                 ///< Monitor memory
    bool showDiskSpace = false;              ///< Monitor disk space (capacity/used/free)
    bool showDiskIO = false;                 ///< Monitor disk I/O (read/write rates)
    bool showNetwork = false;                ///< Monitor network
    bool showTemp = false;                   ///< Monitor temperature
    
    std::string networkInterface;            ///< Specific interface (empty = auto-select)
    
    // Output options
    OutputFormat format = OutputFormat::TEXT; ///< Output format
    bool singleLine = false;                 ///< Single-line compact output
    
    // Monitoring mode
    bool continuous = false;                 ///< Continuous monitoring mode
    double intervalSeconds = 1.0;            ///< Update interval (0.1 - 3600)
    
    // Units
    NetworkUnit networkUnit = NetworkUnit::BITS; ///< Network speed unit
    
    // Help/version
    bool showHelp = false;                   ///< Display help
    bool showVersion = false;                ///< Display version
};

}  // namespace WinHKMon

