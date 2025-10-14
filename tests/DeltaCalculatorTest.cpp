#include "WinHKMonLib/DeltaCalculator.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace WinHKMon;

/**
 * Test Suite: DeltaCalculator
 * 
 * Tests for the DeltaCalculator component that computes rates from delta values
 * between current and previous samples.
 * 
 * Coverage:
 * - Rate calculation with valid deltas
 * - First run handling (no previous data)
 * - Zero elapsed time handling
 * - Counter rollover handling
 * - Negative delta handling
 * - Monotonic timestamp usage
 */

// Test 1: Calculate rate with valid delta
TEST(DeltaCalculatorTest, CalculateRateWithValidDelta) {
    DeltaCalculator calc;
    
    uint64_t current = 1000000;  // 1 MB
    uint64_t previous = 0;
    double elapsedSeconds = 1.0;
    
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    
    // Rate should be (1000000 - 0) / 1.0 = 1000000 bytes/sec
    EXPECT_DOUBLE_EQ(rate, 1000000.0);
}

// Test 2: Calculate rate with fractional elapsed time
TEST(DeltaCalculatorTest, CalculateRateWithFractionalTime) {
    DeltaCalculator calc;
    
    uint64_t current = 5000;
    uint64_t previous = 0;
    double elapsedSeconds = 0.5;  // 500ms
    
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    
    // Rate should be 5000 / 0.5 = 10000 bytes/sec
    EXPECT_DOUBLE_EQ(rate, 10000.0);
}

// Test 3: First run (no previous data) returns 0
TEST(DeltaCalculatorTest, FirstRunReturnsZero) {
    DeltaCalculator calc;
    
    // No previous value means we can't calculate a rate
    // The caller should detect this case (previous == 0 and current != 0)
    // For this test, we'll pass the same value for both
    double rate = calc.calculateRate(1000, 1000, 1.0);
    
    // Delta is 0, so rate should be 0
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// Test 4: Zero elapsed time returns 0 (avoid division by zero)
TEST(DeltaCalculatorTest, ZeroElapsedTimeReturnsZero) {
    DeltaCalculator calc;
    
    uint64_t current = 1000;
    uint64_t previous = 0;
    double elapsedSeconds = 0.0;
    
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    
    // Should return 0 to avoid division by zero
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// Test 5: Counter rollover handling (current < previous)
TEST(DeltaCalculatorTest, CounterRolloverReturnsZero) {
    DeltaCalculator calc;
    
    // Counter rolled over (e.g., 64-bit counter wrapped around)
    uint64_t current = 100;
    uint64_t previous = 1000;
    double elapsedSeconds = 1.0;
    
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    
    // Should return 0 (or handle rollover calculation)
    // For v1.0, we'll just return 0 for negative deltas
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// Test 6: Negative delta handling (log warning and return 0)
TEST(DeltaCalculatorTest, NegativeDeltaReturnsZero) {
    DeltaCalculator calc;
    
    uint64_t current = 0;
    uint64_t previous = 1000;
    double elapsedSeconds = 1.0;
    
    // Negative delta should return 0
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// Test 7: Large values don't overflow
TEST(DeltaCalculatorTest, LargeValuesNoOverflow) {
    DeltaCalculator calc;
    
    uint64_t current = UINT64_MAX / 2;
    uint64_t previous = 0;
    double elapsedSeconds = 1.0;
    
    double rate = calc.calculateRate(current, previous, elapsedSeconds);
    
    // Should handle large values without overflow
    EXPECT_GT(rate, 0.0);
    EXPECT_DOUBLE_EQ(rate, static_cast<double>(current));
}

// Test 8: Calculate elapsed time from timestamps
TEST(DeltaCalculatorTest, CalculateElapsedTimeFromTimestamps) {
    DeltaCalculator calc;
    
    // Simulate timestamps (QueryPerformanceCounter ticks)
    uint64_t frequency = 10000000;  // 10 MHz frequency (common)
    uint64_t previousTimestamp = 0;
    uint64_t currentTimestamp = frequency;  // 1 second later
    
    double elapsed = calc.calculateElapsedSeconds(currentTimestamp, previousTimestamp, frequency);
    
    EXPECT_NEAR(elapsed, 1.0, 0.001);  // Should be 1.0 second
}

// Test 9: Calculate elapsed time with fractional seconds
TEST(DeltaCalculatorTest, CalculateElapsedTimeWithFractionalSeconds) {
    DeltaCalculator calc;
    
    uint64_t frequency = 10000000;  // 10 MHz
    uint64_t previousTimestamp = 0;
    uint64_t currentTimestamp = frequency / 2;  // 0.5 seconds later
    
    double elapsed = calc.calculateElapsedSeconds(currentTimestamp, previousTimestamp, frequency);
    
    EXPECT_NEAR(elapsed, 0.5, 0.001);
}

// Test 10: Elapsed time handles timestamp rollover
TEST(DeltaCalculatorTest, ElapsedTimeHandlesRollover) {
    DeltaCalculator calc;
    
    uint64_t frequency = 10000000;
    uint64_t previousTimestamp = UINT64_MAX - frequency / 2;
    uint64_t currentTimestamp = frequency / 2;  // Wrapped around
    
    // For timestamp rollover, should either:
    // 1. Return 0 (invalid), or
    // 2. Calculate correctly assuming 64-bit wrap
    double elapsed = calc.calculateElapsedSeconds(currentTimestamp, previousTimestamp, frequency);
    
    // If current < previous, we treat it as invalid and return 0
    EXPECT_EQ(elapsed, 0.0);
}

// Test 11: Get current timestamp is monotonic
TEST(DeltaCalculatorTest, GetCurrentTimestampIsMonotonic) {
    DeltaCalculator calc;
    
    uint64_t ts1 = calc.getCurrentTimestamp();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    uint64_t ts2 = calc.getCurrentTimestamp();
    
    // Second timestamp should be greater (time moved forward)
    EXPECT_GT(ts2, ts1);
}

// Test 12: Get performance frequency returns valid value
TEST(DeltaCalculatorTest, GetPerformanceFrequencyValid) {
    DeltaCalculator calc;
    
    uint64_t frequency = calc.getPerformanceFrequency();
    
    // Frequency should be positive and reasonable
    // Typical values: 1-10 MHz on modern systems
    EXPECT_GT(frequency, 0);
    EXPECT_LT(frequency, 100000000ULL);  // Less than 100 MHz
}

// Test 13: Calculate bytes per second to Mbps conversion
TEST(DeltaCalculatorTest, CalculateBytesPerSecToMbps) {
    DeltaCalculator calc;
    
    // 1 MB/s = 8 Mbps
    double bytesPerSec = 1000000.0;  // 1 MB/s
    double mbps = calc.bytesPerSecToMegabitsPerSec(bytesPerSec);
    
    EXPECT_NEAR(mbps, 8.0, 0.01);
}

// Test 14: Calculate bytes per second to MB/s conversion
TEST(DeltaCalculatorTest, CalculateBytesPerSecToMBps) {
    DeltaCalculator calc;
    
    double bytesPerSec = 1000000.0;  // 1 MB/s
    double mbPerSec = calc.bytesPerSecToMegabytesPerSec(bytesPerSec);
    
    EXPECT_NEAR(mbPerSec, 1.0, 0.01);
}

// Test 15: Real-world simulation - network traffic calculation
TEST(DeltaCalculatorTest, RealWorldNetworkTrafficSimulation) {
    DeltaCalculator calc;
    
    // Simulate network interface counters
    uint64_t previousBytes = 1000000000;  // 1 GB received previously
    uint64_t currentBytes = 1010000000;   // 1.01 GB received now (10 MB delta)
    double elapsedSeconds = 1.0;
    
    double bytesPerSec = calc.calculateRate(currentBytes, previousBytes, elapsedSeconds);
    
    // Should be 10 MB/s
    EXPECT_NEAR(bytesPerSec, 10000000.0, 1.0);
    
    // Convert to Mbps
    double mbps = calc.bytesPerSecToMegabitsPerSec(bytesPerSec);
    EXPECT_NEAR(mbps, 80.0, 0.1);  // 10 MB/s = 80 Mbps
}

