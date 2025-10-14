#include "WinHKMonLib/DeltaCalculator.h"
#include <windows.h>
#include <stdexcept>

namespace WinHKMon {

double DeltaCalculator::calculateRate(uint64_t current, uint64_t previous, double elapsedSeconds) {
    // Handle zero elapsed time to avoid division by zero
    if (elapsedSeconds <= 0.0) {
        return 0.0;
    }

    // Handle counter rollover or invalid data (current < previous)
    if (current < previous) {
        // In v1.0, we treat this as invalid and return 0
        // Future versions could handle 64-bit rollover calculation
        return 0.0;
    }

    // Calculate delta
    uint64_t delta = current - previous;

    // Calculate rate: delta / time
    return static_cast<double>(delta) / elapsedSeconds;
}

double DeltaCalculator::calculateElapsedSeconds(uint64_t currentTimestamp, 
                                                uint64_t previousTimestamp, 
                                                uint64_t frequency) {
    // Handle invalid case (current < previous)
    if (currentTimestamp < previousTimestamp) {
        return 0.0;
    }

    // Handle zero frequency
    if (frequency == 0) {
        return 0.0;
    }

    // Calculate elapsed ticks
    uint64_t elapsedTicks = currentTimestamp - previousTimestamp;

    // Convert to seconds
    return static_cast<double>(elapsedTicks) / static_cast<double>(frequency);
}

uint64_t DeltaCalculator::getCurrentTimestamp() {
    LARGE_INTEGER counter;
    
    if (!QueryPerformanceCounter(&counter)) {
        throw std::runtime_error("QueryPerformanceCounter failed");
    }

    return static_cast<uint64_t>(counter.QuadPart);
}

uint64_t DeltaCalculator::getPerformanceFrequency() {
    LARGE_INTEGER frequency;
    
    if (!QueryPerformanceFrequency(&frequency)) {
        throw std::runtime_error("QueryPerformanceFrequency failed");
    }

    return static_cast<uint64_t>(frequency.QuadPart);
}

double DeltaCalculator::bytesPerSecToMegabitsPerSec(double bytesPerSec) {
    // 1 byte/sec = 8 bits/sec
    // 1 Mbps = 1,000,000 bits/sec
    return (bytesPerSec * 8.0) / 1000000.0;
}

double DeltaCalculator::bytesPerSecToMegabytesPerSec(double bytesPerSec) {
    // 1 MB/s = 1,000,000 bytes/sec
    return bytesPerSec / 1000000.0;
}

}  // namespace WinHKMon

