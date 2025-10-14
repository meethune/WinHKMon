#pragma once

#include "Types.h"

/**
 * @file MemoryMonitor.h
 * @brief Memory (RAM and page file) monitoring component
 * 
 * Provides real-time memory usage statistics using Windows GlobalMemoryStatusEx API.
 * This is the simplest monitor component with a single API call.
 */

namespace WinHKMon {

/**
 * @brief Monitors physical and virtual memory (RAM and page file) usage
 * 
 * This class provides access to system memory statistics including:
 * - Physical RAM (total, available, used)
 * - Page file (total, available, used)
 * - Usage percentages
 * 
 * Implementation uses GlobalMemoryStatusEx() which provides all needed data
 * in a single API call.
 * 
 * @note This class is stateless and thread-safe.
 * @note No initialization or cleanup required.
 */
class MemoryMonitor {
public:
    /**
     * @brief Collect current memory usage statistics
     * 
     * Calls GlobalMemoryStatusEx() to retrieve current memory information
     * and calculates derived fields (used bytes, percentages).
     * 
     * @return MemoryStats structure with all memory metrics
     * @throws std::runtime_error if GlobalMemoryStatusEx fails
     * 
     * @note Execution time: < 1ms (single API call)
     * @note Thread-safe: Can be called from multiple threads
     * 
     * @par Example:
     * @code
     * MemoryMonitor monitor;
     * MemoryStats stats = monitor.getCurrentStats();
     * std::cout << "RAM: " << stats.usagePercent << "% used\n";
     * @endcode
     */
    MemoryStats getCurrentStats();
};

}  // namespace WinHKMon

