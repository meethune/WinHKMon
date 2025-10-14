#pragma once

#include "Types.h"
#include <windows.h>
#include <pdh.h>
#include <vector>

/**
 * @file CpuMonitor.h
 * @brief CPU usage and frequency monitoring component
 * 
 * Provides real-time CPU usage statistics using Windows Performance Data Helper (PDH) API
 * and CPU frequency information using CallNtPowerInformation.
 */

namespace WinHKMon {

/**
 * @brief Monitors CPU usage and frequency for overall system and per-core
 * 
 * This class provides access to CPU statistics including:
 * - Total CPU usage percentage
 * - Per-core usage percentages
 * - CPU frequency (current, per-core and average)
 * 
 * Implementation uses Performance Data Helper (PDH) API for usage percentages
 * and CallNtPowerInformation for frequency data.
 * 
 * @note This class maintains PDH query handles and requires initialization/cleanup
 * @note Thread-safe after initialization (read-only operations)
 * @note PDH requires two samples to calculate percentages (minimum 100ms between samples)
 */
class CpuMonitor {
public:
    /**
     * @brief Constructor
     * 
     * Creates a CpuMonitor instance. Call initialize() before using.
     */
    CpuMonitor();

    /**
     * @brief Destructor
     * 
     * Automatically calls cleanup() to release PDH resources.
     */
    ~CpuMonitor();

    // Disable copy and move (PDH handles are not copyable)
    CpuMonitor(const CpuMonitor&) = delete;
    CpuMonitor& operator=(const CpuMonitor&) = delete;
    CpuMonitor(CpuMonitor&&) = delete;
    CpuMonitor& operator=(CpuMonitor&&) = delete;

    /**
     * @brief Initialize PDH query and counters
     * 
     * Opens a PDH query and adds counters for:
     * - Total CPU usage (\\Processor(_Total)\\% Processor Time)
     * - Per-core CPU usage (\\Processor(N)\\% Processor Time)
     * 
     * @throws std::runtime_error if PDH initialization fails
     * 
     * @note Must be called before getCurrentStats()
     * @note Safe to call multiple times (subsequent calls are no-ops)
     */
    void initialize();

    /**
     * @brief Collect current CPU statistics
     * 
     * Collects PDH data (requires two samples, ~100ms minimum between calls)
     * and retrieves CPU frequency information.
     * 
     * @return CpuStats structure with all CPU metrics
     * @throws std::runtime_error if PDH query fails or not initialized
     * 
     * @note First call after initialize() may return 0% usage (needs baseline sample)
     * @note Execution time: ~1-5ms (PDH query collection)
     * @note For accurate results, wait at least 100ms between calls
     * 
     * @par Example:
     * @code
     * CpuMonitor monitor;
     * monitor.initialize();
     * std::this_thread::sleep_for(std::chrono::milliseconds(100));
     * CpuStats stats = monitor.getCurrentStats();
     * std::cout << "CPU: " << stats.totalUsagePercent << "%\n";
     * @endcode
     */
    CpuStats getCurrentStats();

    /**
     * @brief Release PDH resources
     * 
     * Closes the PDH query and releases all counter handles.
     * After cleanup(), initialize() must be called again before using.
     * 
     * @note Safe to call multiple times
     * @note Automatically called by destructor
     */
    void cleanup();

private:
    /**
     * @brief Get current CPU frequencies for all cores
     * 
     * Uses CallNtPowerInformation(ProcessorInformation) to retrieve
     * per-core frequency data.
     * 
     * @return Vector of frequencies in MHz (one per logical processor)
     * @throws std::runtime_error if API call fails
     */
    std::vector<uint64_t> getFrequencies();

    /**
     * @brief Calculate average frequency from per-core values
     * 
     * @param frequencies Vector of per-core frequencies
     * @return Average frequency in MHz
     */
    uint64_t calculateAverageFrequency(const std::vector<uint64_t>& frequencies);

    PDH_HQUERY hQuery_;              ///< PDH query handle
    PDH_HCOUNTER hCpuTotal_;         ///< Total CPU usage counter
    std::vector<PDH_HCOUNTER> hCpuCores_;  ///< Per-core CPU usage counters
    bool initialized_;               ///< Initialization state
    int coreCount_;                  ///< Number of logical processors
};

}  // namespace WinHKMon

