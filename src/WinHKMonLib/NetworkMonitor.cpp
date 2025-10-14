/**
 * @file NetworkMonitor.cpp
 * @brief Network interface statistics monitoring implementation
 * 
 * Uses Windows IP Helper API (GetIfTable2) to enumerate network interfaces
 * and collect traffic statistics.
 */

// Define Windows version for Vista+ APIs (GetIfTable2, MIB_IF_ROW2)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Windows Vista
#endif

#include "WinHKMonLib/NetworkMonitor.h"
#include <windows.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <stdexcept>
#include <algorithm>
#include <cstring>

// Link against IP Helper API
#pragma comment(lib, "iphlpapi.lib")

namespace WinHKMon {

void NetworkMonitor::initialize() {
    // IP Helper API doesn't require initialization, but we verify it's available
    // by attempting to get the interface table
    PMIB_IF_TABLE2 pIfTable = nullptr;
    DWORD result = GetIfTable2(&pIfTable);
    
    if (result != NO_ERROR) {
        throw std::runtime_error("Failed to initialize NetworkMonitor: GetIfTable2 error " + 
                                std::to_string(result));
    }
    
    // Clean up test allocation
    if (pIfTable != nullptr) {
        FreeMibTable(pIfTable);
    }
}

std::vector<InterfaceStats> NetworkMonitor::getCurrentStats() {
    std::vector<InterfaceStats> interfaces;
    
    // Get network interface table
    PMIB_IF_TABLE2 pIfTable = nullptr;
    DWORD result = GetIfTable2(&pIfTable);
    
    if (result != NO_ERROR) {
        throw std::runtime_error("GetIfTable2 failed with error " + std::to_string(result));
    }
    
    // Ensure cleanup on all exit paths
    struct TableGuard {
        PMIB_IF_TABLE2 table;
        ~TableGuard() { if (table) FreeMibTable(table); }
    } guard{pIfTable};
    
    // Enumerate all interfaces
    for (ULONG i = 0; i < pIfTable->NumEntries; i++) {
        MIB_IF_ROW2 ifaceRow = pIfTable->Table[i];
        
        // Skip loopback interfaces
        if (isLoopback(ifaceRow.Type)) {
            continue;
        }
        
        // Create InterfaceStats entry
        InterfaceStats stats;
        
        // Interface identification
        stats.name = wideToUtf8(ifaceRow.Alias);  // User-friendly name (e.g., "Ethernet", "Wi-Fi")
        stats.description = wideToUtf8(ifaceRow.Description);  // Hardware description
        
        // Connection state
        stats.isConnected = (ifaceRow.MediaConnectState == MediaConnectStateConnected);
        
        // Link speed (bits per second)
        stats.linkSpeedBitsPerSec = ifaceRow.TransmitLinkSpeed;  // or ReceiveLinkSpeed (typically same)
        
        // Cumulative traffic counters (octets = bytes)
        stats.totalInOctets = ifaceRow.InOctets;
        stats.totalOutOctets = ifaceRow.OutOctets;
        
        // Rate calculations (set to 0 initially, caller will use DeltaCalculator)
        stats.inBytesPerSec = 0;
        stats.outBytesPerSec = 0;
        
        // Optional packet-level stats (if available)
        if (ifaceRow.InUcastPkts != 0 || ifaceRow.InNUcastPkts != 0) {
            stats.inPacketsPerSec = 0;  // Will be calculated by caller
        }
        if (ifaceRow.OutUcastPkts != 0 || ifaceRow.OutNUcastPkts != 0) {
            stats.outPacketsPerSec = 0;  // Will be calculated by caller
        }
        
        // Error counters
        if (ifaceRow.InErrors != 0) {
            stats.inErrors = ifaceRow.InErrors;
        }
        if (ifaceRow.OutErrors != 0) {
            stats.outErrors = ifaceRow.OutErrors;
        }
        
        interfaces.push_back(stats);
    }
    
    return interfaces;
}

std::string NetworkMonitor::selectPrimaryInterface(const std::vector<InterfaceStats>& interfaces) {
    if (interfaces.empty()) {
        return "";
    }
    
    // Find interface with highest total traffic
    auto maxTrafficIface = std::max_element(interfaces.begin(), interfaces.end(),
        [](const InterfaceStats& a, const InterfaceStats& b) {
            uint64_t totalA = a.totalInOctets + a.totalOutOctets;
            uint64_t totalB = b.totalInOctets + b.totalOutOctets;
            
            // If traffic is equal, prefer Ethernet over Wi-Fi
            if (totalA == totalB) {
                // Check if 'a' is Ethernet-like and 'b' is Wi-Fi-like
                bool aIsEthernet = (a.name.find("Ethernet") != std::string::npos ||
                                   a.description.find("Ethernet") != std::string::npos);
                bool bIsEthernet = (b.name.find("Ethernet") != std::string::npos ||
                                   b.description.find("Ethernet") != std::string::npos);
                bool aIsWifi = (a.name.find("Wi-Fi") != std::string::npos ||
                               a.name.find("WiFi") != std::string::npos ||
                               a.name.find("Wireless") != std::string::npos);
                bool bIsWifi = (b.name.find("Wi-Fi") != std::string::npos ||
                               b.name.find("WiFi") != std::string::npos ||
                               b.name.find("Wireless") != std::string::npos);
                
                // Prefer Ethernet over Wi-Fi
                if (aIsEthernet && bIsWifi) return false;  // a > b
                if (bIsEthernet && aIsWifi) return true;   // b > a
            }
            
            return totalA < totalB;  // Normal comparison
        });
    
    return maxTrafficIface->name;
}

bool NetworkMonitor::isLoopback(unsigned long ifType) const {
    // IF_TYPE_SOFTWARE_LOOPBACK = 24
    return ifType == 24;  // IF_TYPE_SOFTWARE_LOOPBACK
}

std::string NetworkMonitor::wideToUtf8(const wchar_t* wstr) const {
    if (wstr == nullptr || wstr[0] == L'\0') {
        return "";
    }
    
    // Get required buffer size
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        return "";
    }
    
    // Convert to UTF-8
    std::string utf8Str(sizeNeeded - 1, '\0');  // -1 to exclude null terminator
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &utf8Str[0], sizeNeeded, nullptr, nullptr);
    
    return utf8Str;
}

}  // namespace WinHKMon

