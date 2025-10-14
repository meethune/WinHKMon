/**
 * @file TempMonitor.h
 * @brief Temperature monitoring using LibreHardwareMonitor
 * 
 * This component requires administrator privileges due to kernel driver access.
 * It gracefully degrades when run without admin rights.
 * 
 * @note Requires LibreHardwareMonitor library (MPL 2.0 license)
 * @note Requires C++/CLI compilation for .NET interop
 */

#pragma once

#include "Types.h"
#include <optional>
#include <string>

namespace WinHKMon {

/**
 * @brief Admin privilege checker utility
 */
namespace AdminPrivileges {
    /**
     * @brief Check if current process has administrator privileges
     * @return true if running as administrator, false otherwise
     */
    bool IsRunningAsAdmin();
}

/**
 * @brief Temperature monitoring component
 * 
 * Uses LibreHardwareMonitor to access hardware temperature sensors.
 * Requires administrator privileges to load kernel drivers.
 * 
 * @note This class uses C++/CLI for .NET interop
 * @note Compile with /clr flag in MSVC
 */
class TempMonitor {
public:
    /**
     * @brief Initialization result codes
     */
    enum class InitResult {
        SUCCESS,              ///< Initialization successful
        NO_ADMIN,            ///< Admin privileges required
        NO_SENSORS,          ///< No temperature sensors detected
        DRIVER_FAILED,       ///< Driver failed to load
        LIBRARY_MISSING      ///< LibreHardwareMonitor.dll not found
    };

    TempMonitor();
    ~TempMonitor();

    /**
     * @brief Initialize temperature monitoring
     * 
     * Checks admin privileges, loads LibreHardwareMonitor library,
     * initializes hardware detection, and enables CPU sensors.
     * 
     * @return InitResult indicating success or reason for failure
     */
    InitResult initialize();

    /**
     * @brief Get current temperature statistics
     * 
     * Queries all temperature sensors and returns aggregated data.
     * 
     * @return std::optional<TempStats> containing temperature data,
     *         or std::nullopt if monitoring unavailable
     */
    std::optional<TempStats> getCurrentStats();

    /**
     * @brief Clean up resources and unload library
     */
    void cleanup();

    /**
     * @brief Check if temperature monitoring is initialized
     * @return true if initialized and ready, false otherwise
     */
    bool isInitialized() const { return isInitialized_; }

    /**
     * @brief Get human-readable error message for InitResult
     * @param result The initialization result code
     * @return String describing the error
     */
    static std::string getInitResultMessage(InitResult result);

private:
    bool isInitialized_;
    
    // Implementation details hidden (uses managed C++ types)
    // Actual implementation in TempMonitor.cpp with C++/CLI
    class Impl;
    Impl* pImpl_;  // Pointer to implementation (PIMPL pattern)
};

} // namespace WinHKMon

