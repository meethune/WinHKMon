#include "WinHKMonLib/MemoryMonitor.h"
#include <windows.h>
#include <stdexcept>

namespace WinHKMon {

MemoryStats MemoryMonitor::getCurrentStats() {
    // Initialize structure for GlobalMemoryStatusEx
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    // Call Windows API to get memory information
    if (!GlobalMemoryStatusEx(&memStatus)) {
        throw std::runtime_error("GlobalMemoryStatusEx failed");
    }

    // Populate MemoryStats structure
    MemoryStats stats;
    
    // Physical memory
    stats.totalPhysicalBytes = memStatus.ullTotalPhys;
    stats.availablePhysicalBytes = memStatus.ullAvailPhys;
    stats.usedPhysicalBytes = stats.totalPhysicalBytes - stats.availablePhysicalBytes;
    
    // Calculate physical memory usage percentage
    if (stats.totalPhysicalBytes > 0) {
        stats.usagePercent = (static_cast<double>(stats.usedPhysicalBytes) / 
                             stats.totalPhysicalBytes) * 100.0;
    } else {
        stats.usagePercent = 0.0;
    }

    // Page file (virtual memory)
    stats.totalPageFileBytes = memStatus.ullTotalPageFile;
    stats.availablePageFileBytes = memStatus.ullAvailPageFile;
    stats.usedPageFileBytes = stats.totalPageFileBytes - stats.availablePageFileBytes;
    
    // Calculate page file usage percentage
    if (stats.totalPageFileBytes > 0) {
        stats.pageFilePercent = (static_cast<double>(stats.usedPageFileBytes) / 
                                stats.totalPageFileBytes) * 100.0;
    } else {
        stats.pageFilePercent = 0.0;
    }

    // Optional fields: Not populated in v1.0
    // stats.cachedBytes - Would require additional API calls (GetPerformanceInfo)
    // stats.committedBytes - Would require additional API calls (GetPerformanceInfo)

    return stats;
}

}  // namespace WinHKMon

