#include "WinHKMonLib/CpuMonitor.h"
#include <windows.h>
#include <powerbase.h>
#include <powrprof.h>
#include <stdexcept>
#include <algorithm>
#include <numeric>

namespace WinHKMon {

CpuMonitor::CpuMonitor() 
    : hQuery_(nullptr)
    , hCpuTotal_(nullptr)
    , initialized_(false)
    , coreCount_(0) {
}

CpuMonitor::~CpuMonitor() {
    cleanup();
}

void CpuMonitor::initialize() {
    if (initialized_) {
        return;  // Already initialized
    }

    // Get number of logical processors
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    coreCount_ = static_cast<int>(sysInfo.dwNumberOfProcessors);

    // Open PDH query
    PDH_STATUS status = PdhOpenQueryW(nullptr, 0, &hQuery_);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("PdhOpenQuery failed: " + std::to_string(status));
    }

    // Add total CPU counter
    status = PdhAddEnglishCounterW(
        hQuery_,
        L"\\Processor(_Total)\\% Processor Time",
        0,
        &hCpuTotal_
    );
    if (status != ERROR_SUCCESS) {
        PdhCloseQuery(hQuery_);
        hQuery_ = nullptr;
        throw std::runtime_error("PdhAddEnglishCounter (total) failed: " + std::to_string(status));
    }

    // Add per-core counters
    hCpuCores_.resize(coreCount_);
    for (int i = 0; i < coreCount_; ++i) {
        std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        
        status = PdhAddEnglishCounterW(
            hQuery_,
            counterPath.c_str(),
            0,
            &hCpuCores_[i]
        );
        
        if (status != ERROR_SUCCESS) {
            // Clean up already-added counters
            cleanup();
            throw std::runtime_error("PdhAddEnglishCounter (core " + std::to_string(i) + 
                                   ") failed: " + std::to_string(status));
        }
    }

    // Collect first sample (baseline for percentage calculation)
    status = PdhCollectQueryData(hQuery_);
    if (status != ERROR_SUCCESS) {
        cleanup();
        throw std::runtime_error("PdhCollectQueryData (initial) failed: " + std::to_string(status));
    }

    initialized_ = true;
}

CpuStats CpuMonitor::getCurrentStats() {
    if (!initialized_) {
        throw std::runtime_error("CpuMonitor not initialized. Call initialize() first.");
    }

    CpuStats stats;

    // Collect current sample
    PDH_STATUS status = PdhCollectQueryData(hQuery_);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("PdhCollectQueryData failed: " + std::to_string(status));
    }

    // Get total CPU usage
    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(hCpuTotal_, PDH_FMT_DOUBLE, nullptr, &counterValue);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("PdhGetFormattedCounterValue (total) failed: " + std::to_string(status));
    }
    stats.totalUsagePercent = counterValue.doubleValue;

    // Clamp to valid range (PDH sometimes returns slightly > 100%)
    if (stats.totalUsagePercent < 0.0) stats.totalUsagePercent = 0.0;
    if (stats.totalUsagePercent > 100.0) stats.totalUsagePercent = 100.0;

    // Get per-core usage
    stats.cores.resize(coreCount_);
    for (int i = 0; i < coreCount_; ++i) {
        status = PdhGetFormattedCounterValue(hCpuCores_[i], PDH_FMT_DOUBLE, nullptr, &counterValue);
        if (status != ERROR_SUCCESS) {
            throw std::runtime_error("PdhGetFormattedCounterValue (core " + std::to_string(i) + 
                                   ") failed: " + std::to_string(status));
        }

        CoreStats& core = stats.cores[i];
        core.coreId = i;
        core.usagePercent = counterValue.doubleValue;

        // Clamp to valid range
        if (core.usagePercent < 0.0) core.usagePercent = 0.0;
        if (core.usagePercent > 100.0) core.usagePercent = 100.0;
    }

    // Get CPU frequencies
    try {
        std::vector<uint64_t> frequencies = getFrequencies();
        
        // Assign frequencies to cores
        for (int i = 0; i < coreCount_ && i < static_cast<int>(frequencies.size()); ++i) {
            stats.cores[i].frequencyMhz = frequencies[i];
        }

        // Calculate average frequency
        stats.averageFrequencyMhz = calculateAverageFrequency(frequencies);
    } catch (const std::exception&) {
        // If frequency retrieval fails, set to 0 (non-fatal)
        stats.averageFrequencyMhz = 0;
        for (auto& core : stats.cores) {
            core.frequencyMhz = 0;
        }
    }

    // Optional fields: Not populated in v1.0
    // Would require additional PDH counters or Windows APIs

    return stats;
}

void CpuMonitor::cleanup() {
    if (hQuery_ != nullptr) {
        PdhCloseQuery(hQuery_);
        hQuery_ = nullptr;
    }
    
    hCpuTotal_ = nullptr;
    hCpuCores_.clear();
    initialized_ = false;
    coreCount_ = 0;
}

std::vector<uint64_t> CpuMonitor::getFrequencies() {
    std::vector<uint64_t> frequencies;

    // Allocate buffer for processor information
    std::vector<PROCESSOR_POWER_INFORMATION> procInfo(coreCount_);

    // Call CallNtPowerInformation
    NTSTATUS status = CallNtPowerInformation(
        ProcessorInformation,
        nullptr,
        0,
        procInfo.data(),
        static_cast<ULONG>(procInfo.size() * sizeof(PROCESSOR_POWER_INFORMATION))
    );

    if (status != 0) {  // STATUS_SUCCESS = 0
        throw std::runtime_error("CallNtPowerInformation failed: " + std::to_string(status));
    }

    // Extract frequencies
    frequencies.reserve(coreCount_);
    for (const auto& info : procInfo) {
        frequencies.push_back(static_cast<uint64_t>(info.CurrentMhz));
    }

    return frequencies;
}

uint64_t CpuMonitor::calculateAverageFrequency(const std::vector<uint64_t>& frequencies) {
    if (frequencies.empty()) {
        return 0;
    }

    uint64_t sum = std::accumulate(frequencies.begin(), frequencies.end(), 0ULL);
    return sum / frequencies.size();
}

}  // namespace WinHKMon

