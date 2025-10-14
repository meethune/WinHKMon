/**
 * @file DiskMonitor.cpp
 * @brief Disk I/O statistics monitoring implementation
 * 
 * Uses Windows Performance Data Helper (PDH) API to collect physical disk
 * I/O statistics including read/write rates and busy percentage.
 */

#include "WinHKMonLib/DiskMonitor.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <chrono>

// Link against PDH library
#pragma comment(lib, "pdh.lib")

namespace WinHKMon {

/**
 * @brief Disk space information
 */
struct DiskSpaceInfo {
    uint64_t totalBytes;
    uint64_t freeBytes;
    uint64_t usedBytes;
};

}  // namespace WinHKMon

namespace {
    /**
     * Extract user-friendly disk name from PDH disk name
     * PDH format: "0 C:", "1 D:", "_Total"
     * Extract to: "C:", "D:", "_Total"
     */
    std::string extractFriendlyDiskName(const std::string& pdhDiskName) {
        // Keep "_Total" as is
        if (pdhDiskName == "_Total") {
            return pdhDiskName;
        }
        
        // Find the first letter after a space (the drive letter)
        size_t spacePos = pdhDiskName.find(' ');
        if (spacePos != std::string::npos && spacePos + 1 < pdhDiskName.length()) {
            // Return everything after the space (drive letter and colon)
            return pdhDiskName.substr(spacePos + 1);
        }
        
        // If format is unexpected, return as-is
        return pdhDiskName;
    }
}

namespace WinHKMon {

DiskMonitor::DiskMonitor() 
    : hQuery_(nullptr), initialized_(false) {
}

DiskMonitor::~DiskMonitor() {
    cleanup();
}

void DiskMonitor::initialize() {
    if (initialized_) {
        return;  // Already initialized
    }
    
    // Open PDH query
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &hQuery_);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("PdhOpenQuery failed with error " + std::to_string(status));
    }
    
    // Enumerate physical disk instances
    // We'll use wildcard (*) to get all physical disks
    
    // Get list of instances
    DWORD bufferSize = 0;
    DWORD instanceCount = 0;
    status = PdhEnumObjectItemsW(
        nullptr,                    // Local machine
        nullptr,                    // Default data source
        L"PhysicalDisk",           // Object name
        nullptr,                    // Counter list buffer (we don't need it)
        &bufferSize,               // Buffer size for counters
        nullptr,                    // Instance list buffer (we'll allocate it)
        &instanceCount,            // Buffer size for instances
        PERF_DETAIL_WIZARD,        // Detail level
        0                          // Reserved
    );
    
    // Allocate buffer for instance names
    if (instanceCount > 0) {
        std::vector<wchar_t> instanceBuffer(instanceCount);
        bufferSize = 0;  // Reset for counters
        
        status = PdhEnumObjectItemsW(
            nullptr,
            nullptr,
            L"PhysicalDisk",
            nullptr,
            &bufferSize,
            instanceBuffer.data(),
            &instanceCount,
            PERF_DETAIL_WIZARD,
            0
        );
        
        if (status == ERROR_SUCCESS || status == PDH_MORE_DATA) {
            // Parse instance names (null-terminated strings)
            const wchar_t* instance = instanceBuffer.data();
            while (*instance != L'\0') {
                std::string instanceName = wideToUtf8(instance);
                
                // Add counters for this instance
                try {
                    addDiskCounters(instanceName);
                } catch (const std::exception&) {
                    // Log error but continue with other disks
                    // In production, use proper logging
                }
                
                // Move to next instance
                instance += wcslen(instance) + 1;
            }
        }
    }
    
    // If no instances found via enumeration, try adding common instances
    if (counters_.empty()) {
        // Add _Total counter as fallback
        try {
            addDiskCounters("_Total");
        } catch (const std::exception&) {
            // If even _Total fails, cleanup and throw
            cleanup();
            throw std::runtime_error("Failed to add any disk counters");
        }
    }
    
    // Collect first sample (required for rate calculations)
    status = PdhCollectQueryData(hQuery_);
    if (status != ERROR_SUCCESS) {
        cleanup();
        throw std::runtime_error("Initial PdhCollectQueryData failed with error " + 
                                std::to_string(status));
    }
    
    initialized_ = true;
}

std::vector<DiskStats> DiskMonitor::getCurrentStats() {
    if (!initialized_) {
        throw std::runtime_error("DiskMonitor not initialized");
    }
    
    std::vector<DiskStats> disks;
    
    // Collect current sample
    PDH_STATUS status = PdhCollectQueryData(hQuery_);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("PdhCollectQueryData failed with error " + 
                                std::to_string(status));
    }
    
    // Wait a moment for PDH to process (important for first real sample)
    // PDH needs time between samples for rate calculations
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Retrieve formatted values for each disk
    for (const auto& [diskName, counters] : counters_) {
        DiskStats stats;
        stats.deviceName = extractFriendlyDiskName(diskName);
        
        // Get read rate
        PDH_FMT_COUNTERVALUE readValue;
        status = PdhGetFormattedCounterValue(
            counters.bytesRead,
            PDH_FMT_LARGE,
            nullptr,
            &readValue
        );
        if (status == ERROR_SUCCESS && readValue.CStatus == PDH_CSTATUS_VALID_DATA) {
            stats.bytesReadPerSec = static_cast<uint64_t>(readValue.largeValue);
        } else {
            stats.bytesReadPerSec = 0;
        }
        
        // Get write rate
        PDH_FMT_COUNTERVALUE writeValue;
        status = PdhGetFormattedCounterValue(
            counters.bytesWritten,
            PDH_FMT_LARGE,
            nullptr,
            &writeValue
        );
        if (status == ERROR_SUCCESS && writeValue.CStatus == PDH_CSTATUS_VALID_DATA) {
            stats.bytesWrittenPerSec = static_cast<uint64_t>(writeValue.largeValue);
        } else {
            stats.bytesWrittenPerSec = 0;
        }
        
        // Get busy percentage
        PDH_FMT_COUNTERVALUE busyValue;
        status = PdhGetFormattedCounterValue(
            counters.percentBusy,
            PDH_FMT_DOUBLE,
            nullptr,
            &busyValue
        );
        if (status == ERROR_SUCCESS && busyValue.CStatus == PDH_CSTATUS_VALID_DATA) {
            stats.percentBusy = busyValue.doubleValue;
        } else {
            stats.percentBusy = 0.0;
        }
        
        // Get disk space information (extract drive letter from instance name)
        std::string driveLetter = extractDriveLetter(diskName);
        if (!driveLetter.empty()) {
            DiskSpaceInfo spaceInfo = getDiskSpace(driveLetter);
            stats.totalSizeBytes = spaceInfo.totalBytes;
            stats.freeBytes = spaceInfo.freeBytes;
            stats.usedBytes = spaceInfo.usedBytes;
        } else {
            stats.totalSizeBytes = 0;
            stats.freeBytes = 0;
            stats.usedBytes = 0;
        }
        
        // Cumulative counters (these would be tracked by StateManager in real usage)
        // For now, we set them to 0 as they require persistent state
        stats.totalBytesRead = 0;
        stats.totalBytesWritten = 0;
        
        // Optional IOPS (not currently collected, but structure supports it)
        // Could be added via additional PDH counters:
        // - \\PhysicalDisk(*)\\Disk Reads/sec
        // - \\PhysicalDisk(*)\\Disk Writes/sec
        
        disks.push_back(stats);
    }
    
    return disks;
}

void DiskMonitor::cleanup() {
    if (hQuery_ != nullptr) {
        // Counter handles are automatically closed when query is closed
        PdhCloseQuery(hQuery_);
        hQuery_ = nullptr;
    }
    counters_.clear();
    initialized_ = false;
}

void DiskMonitor::addDiskCounters(const std::string& diskInstance) {
    DiskCounters counters;
    
    // Build counter paths
    std::wstring wInstanceName(diskInstance.begin(), diskInstance.end());
    
    // Read bytes/sec counter
    std::wstring readPath = L"\\PhysicalDisk(" + wInstanceName + L")\\Disk Read Bytes/sec";
    PDH_STATUS status = PdhAddCounterW(hQuery_, readPath.c_str(), 0, &counters.bytesRead);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to add read counter for " + diskInstance + 
                                ": error " + std::to_string(status));
    }
    
    // Write bytes/sec counter
    std::wstring writePath = L"\\PhysicalDisk(" + wInstanceName + L")\\Disk Write Bytes/sec";
    status = PdhAddCounterW(hQuery_, writePath.c_str(), 0, &counters.bytesWritten);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to add write counter for " + diskInstance + 
                                ": error " + std::to_string(status));
    }
    
    // Disk time percentage counter
    std::wstring busyPath = L"\\PhysicalDisk(" + wInstanceName + L")\\% Disk Time";
    status = PdhAddCounterW(hQuery_, busyPath.c_str(), 0, &counters.percentBusy);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to add busy counter for " + diskInstance + 
                                ": error " + std::to_string(status));
    }
    
    // Store counters
    counters_[diskInstance] = counters;
}

DiskSpaceInfo DiskMonitor::getDiskSpace(const std::string& driveLetter) {
    // Convert to wide string with backslash (e.g., "C:" -> "C:\\")
    std::wstring wDrive(driveLetter.begin(), driveLetter.end());
    if (wDrive.back() != L'\\') {
        wDrive += L'\\';
    }
    
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalFreeBytes;
    
    if (GetDiskFreeSpaceExW(
            wDrive.c_str(),
            &freeBytesAvailable,
            &totalBytes,
            &totalFreeBytes
        )) {
        uint64_t total = totalBytes.QuadPart;
        uint64_t free = totalFreeBytes.QuadPart;
        uint64_t used = (total > free) ? (total - free) : 0;
        return DiskSpaceInfo{total, free, used};
    }
    
    return DiskSpaceInfo{0, 0, 0};
}

std::string DiskMonitor::extractDriveLetter(const std::string& diskInstance) {
    // Disk instance names are like "0 C:", "1 D:", or "_Total"
    // Extract the drive letter part (e.g., "C:")
    
    size_t colonPos = diskInstance.find(':');
    if (colonPos != std::string::npos && colonPos > 0) {
        // Get character before colon
        char driveLetter = diskInstance[colonPos - 1];
        if ((driveLetter >= 'A' && driveLetter <= 'Z') || 
            (driveLetter >= 'a' && driveLetter <= 'z')) {
            return std::string(1, driveLetter) + ":";
        }
    }
    
    return "";
}

std::string DiskMonitor::wideToUtf8(const wchar_t* wstr) {
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

