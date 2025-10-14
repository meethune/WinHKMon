#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <pdh.h>

/**
 * @file DiskMonitor.h
 * @brief Disk I/O statistics monitoring
 * 
 * Provides disk I/O monitoring using Windows Performance Data Helper (PDH) API
 * for physical disk counters.
 */

namespace WinHKMon {

/**
 * @brief Disk I/O monitor using PDH counters
 * 
 * Collects disk I/O statistics including read/write rates, busy percentage,
 * and cumulative byte counts using PDH counters for physical disks.
 * 
 * @note Requires two PDH samples for rate calculations
 * @note First call to getCurrentStats() initializes counters, subsequent calls return rates
 */
class DiskMonitor {
public:
    /**
     * @brief Construct DiskMonitor (no initialization)
     */
    DiskMonitor();
    
    /**
     * @brief Destructor - cleans up PDH resources
     */
    ~DiskMonitor();
    
    /**
     * @brief Initialize the disk monitor
     * 
     * Opens PDH query and adds physical disk counters:
     * - \\PhysicalDisk(*)\\Disk Read Bytes/sec
     * - \\PhysicalDisk(*)\\Disk Write Bytes/sec
     * - \\PhysicalDisk(*)\\% Disk Time
     * 
     * @throws std::runtime_error if PDH initialization fails
     */
    void initialize();
    
    /**
     * @brief Get current disk I/O statistics
     * 
     * Collects PDH data and returns statistics for all physical disks including:
     * - Device name (e.g., "0 C:", "1 D:", "_Total")
     * - Read/write rates (bytes per second)
     * - Disk busy percentage (0-100)
     * - Total disk size
     * - Cumulative byte counters
     * 
     * @return Vector of DiskStats for all physical disks
     * @throws std::runtime_error if PDH query fails
     * 
     * @note First call initializes counters; rates may be zero or inaccurate
     * @note Subsequent calls (after ~1 second) provide accurate rates
     */
    std::vector<DiskStats> getCurrentStats();
    
    /**
     * @brief Clean up PDH resources
     * 
     * Closes PDH query and counter handles. Safe to call multiple times.
     */
    void cleanup();

private:
    PDH_HQUERY hQuery_;           ///< PDH query handle
    bool initialized_;             ///< Initialization state
    
    /**
     * @brief Counter handles for each disk instance
     */
    struct DiskCounters {
        PDH_HCOUNTER bytesRead;    ///< Read bytes/sec counter
        PDH_HCOUNTER bytesWritten; ///< Write bytes/sec counter
        PDH_HCOUNTER percentBusy;  ///< Disk time percentage counter
    };
    
    std::map<std::string, DiskCounters> counters_;  ///< Counter handles by disk name
    
    /**
     * @brief Add PDH counters for a specific disk instance
     * 
     * @param diskInstance PDH instance name (e.g., "0 C:", "_Total")
     * @throws std::runtime_error if counter addition fails
     */
    void addDiskCounters(const std::string& diskInstance);
    
    /**
     * @brief Get disk size for a drive letter
     * 
     * @param driveLetter Drive letter (e.g., "C:")
     * @return Total disk size in bytes (0 if unavailable)
     */
    uint64_t getDiskSize(const std::string& driveLetter);
    
    /**
     * @brief Extract drive letter from disk instance name
     * 
     * @param diskInstance Instance name like "0 C:" or "1 D:"
     * @return Drive letter like "C:" or empty if not found
     */
    std::string extractDriveLetter(const std::string& diskInstance);
    
    /**
     * @brief Convert wide string to UTF-8
     * 
     * @param wstr Wide string
     * @return UTF-8 encoded string
     */
    std::string wideToUtf8(const wchar_t* wstr);
};

}  // namespace WinHKMon

