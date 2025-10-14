#pragma once

#include "Types.h"
#include <string>
#include <vector>

/**
 * @file NetworkMonitor.h
 * @brief Network interface statistics monitoring
 * 
 * Provides network interface enumeration and statistics collection using
 * Windows IP Helper API (GetIfTable2, MIB_IF_ROW2).
 */

namespace WinHKMon {

/**
 * @brief Network interface monitor using IP Helper API
 * 
 * Collects network interface statistics including traffic counters,
 * connection status, and link speeds. Uses GetIfTable2() for modern
 * Windows versions (Vista+).
 * 
 * @note Loopback interfaces are automatically filtered out
 * @note Rate calculations require DeltaCalculator and previous state
 */
class NetworkMonitor {
public:
    /**
     * @brief Construct NetworkMonitor (no initialization needed)
     */
    NetworkMonitor() = default;
    
    /**
     * @brief Destructor
     */
    ~NetworkMonitor() = default;
    
    /**
     * @brief Initialize the network monitor
     * 
     * Verifies IP Helper API availability. This is a lightweight operation
     * as GetIfTable2 doesn't require persistent handles.
     * 
     * @throws std::runtime_error if IP Helper API is unavailable
     */
    void initialize();
    
    /**
     * @brief Get current statistics for all network interfaces
     * 
     * Enumerates all network interfaces and retrieves statistics including:
     * - Interface name and description
     * - Connection status and link speed
     * - Cumulative traffic counters (in/out octets)
     * - Rate calculations (set to 0 on first call, updated by caller)
     * 
     * @return Vector of InterfaceStats for all non-loopback interfaces
     * @throws std::runtime_error if GetIfTable2() fails
     * 
     * @note Loopback interfaces are filtered out automatically
     * @note Rate calculations (inBytesPerSec, outBytesPerSec) are set to 0;
     *       caller must use DeltaCalculator to compute rates from cumulative counters
     */
    std::vector<InterfaceStats> getCurrentStats();
    
    /**
     * @brief Select primary network interface for monitoring
     * 
     * Selection algorithm:
     * 1. Exclude loopback interfaces
     * 2. Select interface with highest total traffic (inOctets + outOctets)
     * 3. If tie, prefer Ethernet over Wi-Fi
     * 4. Fallback: first non-loopback interface
     * 
     * @param interfaces List of available interfaces
     * @return Name of selected primary interface (empty if no interfaces)
     */
    std::string selectPrimaryInterface(const std::vector<InterfaceStats>& interfaces);

private:
    /**
     * @brief Check if interface is loopback
     * 
     * @param ifType Interface type from MIB_IF_ROW2
     * @return true if loopback interface
     */
    bool isLoopback(unsigned long ifType) const;
    
    /**
     * @brief Convert wide string to UTF-8
     * 
     * @param wstr Wide string (e.g., interface alias)
     * @return UTF-8 encoded string
     */
    std::string wideToUtf8(const wchar_t* wstr) const;
};

}  // namespace WinHKMon

