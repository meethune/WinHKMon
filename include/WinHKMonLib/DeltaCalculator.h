#pragma once

#include <cstdint>

/**
 * @file DeltaCalculator.h
 * @brief Utility for calculating rates from delta values
 * 
 * Provides methods to compute rates (bytes/sec) from cumulative counters
 * and handle monotonic timestamps for elapsed time calculation.
 */

namespace WinHKMon {

/**
 * @brief Calculates rates from delta values between samples
 * 
 * This class provides utility functions for:
 * - Computing rates from counter deltas
 * - Handling elapsed time with monotonic timestamps (QueryPerformanceCounter)
 * - Converting between units (bytes/sec to Mbps, MB/s)
 * - Handling edge cases (rollover, zero elapsed time, negative deltas)
 * 
 * @note All methods are thread-safe (no internal state)
 * @note Uses QueryPerformanceCounter for monotonic, high-resolution timestamps
 */
class DeltaCalculator {
public:
    /**
     * @brief Calculate rate from counter delta
     * 
     * Computes rate as: (current - previous) / elapsedSeconds
     * 
     * @param current Current counter value
     * @param previous Previous counter value
     * @param elapsedSeconds Time elapsed between samples
     * @return Rate in units/second (typically bytes/sec)
     * 
     * @note Returns 0 if:
     *   - elapsedSeconds is 0 (avoid division by zero)
     *   - current < previous (counter rollover or invalid data)
     * 
     * @par Example:
     * @code
     * DeltaCalculator calc;
     * uint64_t current = 10000000;   // 10 MB
     * uint64_t previous = 0;
     * double rate = calc.calculateRate(current, previous, 1.0);  // 10 MB/s
     * @endcode
     */
    double calculateRate(uint64_t current, uint64_t previous, double elapsedSeconds);

    /**
     * @brief Calculate elapsed time from monotonic timestamps
     * 
     * Computes elapsed seconds using QueryPerformanceCounter timestamps.
     * 
     * @param currentTimestamp Current timestamp (QPC ticks)
     * @param previousTimestamp Previous timestamp (QPC ticks)
     * @param frequency Performance counter frequency (ticks per second)
     * @return Elapsed time in seconds
     * 
     * @note Returns 0 if currentTimestamp < previousTimestamp (invalid/rollover)
     * 
     * @par Example:
     * @code
     * DeltaCalculator calc;
     * uint64_t freq = calc.getPerformanceFrequency();
     * uint64_t ts1 = calc.getCurrentTimestamp();
     * // ... do work ...
     * uint64_t ts2 = calc.getCurrentTimestamp();
     * double elapsed = calc.calculateElapsedSeconds(ts2, ts1, freq);
     * @endcode
     */
    double calculateElapsedSeconds(uint64_t currentTimestamp, 
                                   uint64_t previousTimestamp, 
                                   uint64_t frequency);

    /**
     * @brief Get current monotonic timestamp
     * 
     * Retrieves current value of QueryPerformanceCounter.
     * 
     * @return Current timestamp in QPC ticks
     * @throws std::runtime_error if QueryPerformanceCounter fails
     * 
     * @note Timestamps are monotonic (unaffected by system time changes)
     * @note Resolution is typically microseconds on modern systems
     */
    uint64_t getCurrentTimestamp();

    /**
     * @brief Get performance counter frequency
     * 
     * Retrieves QueryPerformanceFrequency (ticks per second).
     * 
     * @return Frequency in ticks per second
     * @throws std::runtime_error if QueryPerformanceFrequency fails
     * 
     * @note Frequency is constant for the lifetime of the process
     * @note Can be cached if needed (doesn't change)
     */
    uint64_t getPerformanceFrequency();

    /**
     * @brief Convert bytes/sec to Megabits/sec (Mbps)
     * 
     * Conversion: Mbps = (bytes/sec * 8) / 1,000,000
     * 
     * @param bytesPerSec Rate in bytes per second
     * @return Rate in Megabits per second
     */
    double bytesPerSecToMegabitsPerSec(double bytesPerSec);

    /**
     * @brief Convert bytes/sec to Megabytes/sec (MB/s)
     * 
     * Conversion: MB/s = bytes/sec / 1,000,000
     * 
     * @param bytesPerSec Rate in bytes per second
     * @return Rate in Megabytes per second
     */
    double bytesPerSecToMegabytesPerSec(double bytesPerSec);
};

}  // namespace WinHKMon

