#pragma once

#include "WinHKMonLib/Types.h"
#include <filesystem>
#include <string>

/**
 * @file StateManager.h
 * @brief State persistence for delta calculations
 */

namespace WinHKMon {

/**
 * @brief Manages persistent state for delta calculations between runs
 * 
 * State is stored in a text file in the user's temp directory.
 * Format:
 *   VERSION 1.0
 *   TIMESTAMP <value>
 *   NETWORK_<interface>_IN <bytes>
 *   NETWORK_<interface>_OUT <bytes>
 *   DISK_<device>_READ <bytes>
 *   DISK_<device>_WRITE <bytes>
 */
class StateManager {
public:
    /**
     * @brief Construct StateManager with application name
     * @param appName Application name for state file (e.g., "WinHKMon")
     */
    explicit StateManager(const std::string& appName);

    /**
     * @brief Load previous state from file
     * 
     * @param[out] metrics Metrics structure to populate with previous counters
     * @param[out] timestamp Previous timestamp
     * @return true if state loaded successfully, false if no state or corrupted
     */
    bool load(SystemMetrics& metrics, uint64_t& timestamp);

    /**
     * @brief Save current state to file
     * 
     * @param metrics Current metrics to save
     * @return true if saved successfully, false on error
     */
    bool save(const SystemMetrics& metrics);

private:
    std::filesystem::path getStatePath() const;
    bool validateVersion(const std::string& version) const;
    std::string sanitizeKey(const std::string& key) const;

    std::string appName_;
    static constexpr const char* VERSION = "1.0";
};

}  // namespace WinHKMon

