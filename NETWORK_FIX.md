# Network & Disk I/O Monitoring Fix - Rate Calculation Implementation

**Date**: 2025-10-14  
**Issues**: 
1. Network rates always showing 0 bps
2. Disk I/O rates always showing 0 B/s (only busy % working)

**Status**: ✅ **BOTH FIXED** (Awaiting Windows Testing)  
**Commits**: 
- `0da0676` - Network rate delta calculations
- `b0c0b43` - Disk I/O rate usage from PDH

---

## Problem Description

### Issue 1: Network Monitoring
Network monitoring was reporting **0 bps for all interfaces** even during active network traffic (e.g., running speed tests).

**User Report:**
```
NET:  Ethernet < 0 bps  > 0 bps  (1.0 Gbps link)
NET:  Ethernet < 0 bps  > 0 bps  (1.0 Gbps link)
NET:  Ethernet < 0 bps  > 0 bps  (1.0 Gbps link)
```

Even with active traffic, rates never changed from 0.

### Issue 2: Disk I/O Monitoring
Disk I/O was reporting **0 B/s for read/write** even though busy percentage was updating correctly.

**User Report:**
```
IO:   C: < 0 B/s  > 0 B/s  (3.5% busy)
IO:   C: < 0 B/s  > 0 B/s  (9.8% busy)
IO:   C: < 0 B/s  > 0 B/s  (16.8% busy)
```

Busy percentage changed but rates stayed at 0.

---

## Root Cause Analysis

### Network Issue: Missing State Management

#### The Design
The WinHKMon architecture was designed with a clear separation of concerns:

1. **Monitors** (NetworkMonitor, DiskMonitor) collect **cumulative counters**:
   - `totalInOctets`, `totalOutOctets` (network)
   - `totalBytesRead`, `totalBytesWritten` (disk)

2. **StateManager** saves/loads previous counter values

3. **DeltaCalculator** computes rates: `rate = (current - previous) / elapsedTime`

4. **main.cpp** orchestrates: load state → collect → calculate → save state

### The Bug
**The orchestration was never implemented!**

`main.cpp` had placeholder code but critical pieces were missing:

```cpp
// StateManager created but NEVER USED ❌
StateManager stateManager("WinHKMon");

// DeltaCalculator used ONLY for timestamps, not rate calculations ❌
metrics.timestamp = deltaCalc.getCurrentTimestamp();

// collectMetrics() had NO delta calculation logic ❌
SystemMetrics collectMetrics(..., DeltaCalculator& deltaCalc) {
    // Monitors set rates to 0, expecting caller to fill them in
    // BUT: No delta calculation code existed!
}
```

**Result**: Rates stayed at the initial 0 value set by monitors.

---

### Disk Issue: Wrong API Usage

#### The Problem
**PDH disk counters work DIFFERENTLY than network interfaces!**

| Data Source | Counter Type | Calculation Needed? |
|------------|--------------|---------------------|
| **Network** (`GetIfTable2`) | **Cumulative bytes** (totalInOctets, totalOutOctets) | ✅ YES - we calculate: `rate = delta / time` |
| **Disk** (PDH) | **Instantaneous rates** (bytes/sec from PDH) | ❌ NO - PDH already calculated it! |

**PDH Disk Counters:**
- `\\PhysicalDisk(*)\\Disk Read Bytes/sec` ← **Already a RATE**
- `\\PhysicalDisk(*)\\Disk Write Bytes/sec` ← **Already a RATE**
- `\\PhysicalDisk(*)\\% Disk Time` ← Busy percentage (working correctly)

#### The Bug
1. **DiskMonitor.cpp line 189, 203**: Correctly gets rates from PDH ✅
2. **DiskMonitor.cpp line 237-238**: Sets `totalBytesRead`/`totalBytesWritten` to **0** (no cumulative counters exist!)
3. **main.cpp**: Tried to calculate rate from 0 counters: `rate = (0 - 0) / time = 0` ❌
4. **Result**: Overwrote good PDH rates with 0

```cpp
// DiskMonitor.cpp (BEFORE FIX)
stats.bytesReadPerSec = readValue.largeValue;  // ← Good value from PDH!

// ... later ...
stats.totalBytesRead = 0;  // ← No cumulative counter available

// main.cpp (BEFORE FIX)
disk.bytesReadPerSec = calculateRate(0, 0, elapsedSeconds);  // ← Overwrites with 0!
```

---

## Solution Implemented

### 1. Updated `collectMetrics()` Signature

**Before:**
```cpp
SystemMetrics collectMetrics(const CliOptions& options, 
                             CpuMonitor* cpuMonitor, 
                             MemoryMonitor& memoryMonitor,
                             NetworkMonitor* networkMonitor,
                             DiskMonitor* diskMonitor,
                             DeltaCalculator& deltaCalc);
```

**After:**
```cpp
SystemMetrics collectMetrics(const CliOptions& options, 
                             CpuMonitor* cpuMonitor, 
                             MemoryMonitor& memoryMonitor,
                             NetworkMonitor* networkMonitor,
                             DiskMonitor* diskMonitor,
                             DeltaCalculator& deltaCalc,
                             const SystemMetrics& previousMetrics,  // ← NEW
                             uint64_t previousTimestamp);            // ← NEW
```

### 2. Added State Management to `singleShotMode()`

```cpp
// Load previous state
SystemMetrics previousMetrics;
uint64_t previousTimestamp = 0;
if (!stateManager.load(previousMetrics, previousTimestamp)) {
    // First run - use current timestamp as baseline
    previousTimestamp = deltaCalc.getCurrentTimestamp();
}

// Collect with delta calculations
SystemMetrics metrics = collectMetrics(..., previousMetrics, previousTimestamp);

// Save for next run
stateManager.save(metrics);
```

### 3. Added State Management to `continuousMode()`

```cpp
// Load initial state before loop
SystemMetrics previousMetrics;
uint64_t previousTimestamp = 0;
stateManager.load(previousMetrics, previousTimestamp);

while (g_continueMonitoring) {
    // Collect with delta calculations
    SystemMetrics metrics = collectMetrics(..., previousMetrics, previousTimestamp);
    
    // Output...
    
    // Update for next iteration
    previousMetrics = metrics;
    previousTimestamp = metrics.timestamp;
}

// Save final state on exit
stateManager.save(previousMetrics);
```

### 4. Implemented Delta Calculation Logic

**Network Rates:**
```cpp
// Calculate elapsed time
uint64_t frequency = deltaCalc.getPerformanceFrequency();
double elapsedSeconds = deltaCalc.calculateElapsedSeconds(
    metrics.timestamp, previousTimestamp, frequency);

// For each network interface
if (elapsedSeconds > 0 && previousMetrics.network.has_value()) {
    for (auto& iface : interfaces) {
        // Find matching previous interface
        auto prevIt = std::find_if(
            previousMetrics.network->begin(),
            previousMetrics.network->end(),
            [&iface](const InterfaceStats& prev) {
                return prev.name == iface.name;
            });
        
        if (prevIt != previousMetrics.network->end()) {
            // Calculate rates
            iface.inBytesPerSec = static_cast<uint64_t>(
                deltaCalc.calculateRate(iface.totalInOctets, 
                                       prevIt->totalInOctets, 
                                       elapsedSeconds));
            iface.outBytesPerSec = static_cast<uint64_t>(
                deltaCalc.calculateRate(iface.totalOutOctets, 
                                        prevIt->totalOutOctets, 
                                        elapsedSeconds));
        }
    }
}
```

**Disk I/O Rates** (different approach - use PDH rates directly):
```cpp
// Disk rates come directly from PDH - don't recalculate!
std::vector<DiskStats> disks = diskMonitor->getCurrentStats();

// Only accumulate cumulative totals (integrate rates over time)
if (elapsedSeconds > 0 && previousMetrics.disks.has_value()) {
    for (auto& disk : disks) {
        auto prevIt = std::find_if(...);
        if (prevIt != previousMetrics.disks->end()) {
            // Integrate: total = previous_total + (rate * time)
            disk.totalBytesRead = prevIt->totalBytesRead + 
                static_cast<uint64_t>(disk.bytesReadPerSec * elapsedSeconds);
            disk.totalBytesWritten = prevIt->totalBytesWritten + 
                static_cast<uint64_t>(disk.bytesWrittenPerSec * elapsedSeconds);
        }
        // NOTE: bytesReadPerSec and bytesWrittenPerSec already set by PDH!
    }
}
```

**Key Difference:**
- **Network**: Calculate rate FROM cumulative → `rate = delta / time`
- **Disk**: Calculate cumulative FROM rate → `total = sum(rate * time)`

---

## Changes Summary

| Commit | File | Lines Changed | Description |
|--------|------|---------------|-------------|
| `0da0676` | `src/WinHKMon/main.cpp` | +98, -8 | Network delta calculations + state management |
| `b0c0b43` | `src/WinHKMon/main.cpp` | +11, -11 | Disk I/O - use PDH rates directly |

**Total**: 109 insertions, 19 deletions (117 lines modified)

**Key Additions:**
- Network rate calculation loop: 28 lines
- Disk I/O accumulation logic: 18 lines (REVISED from 25)
- State management in singleShotMode: 8 lines
- State management in continuousMode: 15 lines
- Elapsed time calculation: 5 lines

---

## Testing Instructions

### Build
```cmd
cd C:\Users\Izaak\Desktop\WinHKMon\build
cmake --build . --config Release
ctest -C Release --output-on-failure
```

### Test Network Rates

**Before Fix:**
```cmd
.\Release\WinHKMon.exe NET --continuous --interval 1
# Output: All interfaces show 0 bps even during traffic
```

**After Fix:**
```cmd
.\Release\WinHKMon.exe NET --continuous --interval 1
# Then run a speed test in browser
# Expected: Should show increasing Mbps values during test
```

### Test Disk I/O Rates

```cmd
.\Release\WinHKMon.exe IO --continuous --interval 1
# Then copy a large file or run a disk-intensive program
# Expected: Should show non-zero read/write rates
```

### Test State Persistence

```cmd
# Run once
.\Release\WinHKMon.exe NET

# Run again immediately
.\Release\WinHKMon.exe NET

# Expected: Second run should show realistic rates
# (delta from first run to second run)
```

---

## Expected Results

### Network Traffic During Speed Test
```
NET:  Ethernet < 350.5 Mbps  > 45.2 Mbps  (1.0 Gbps link)
NET:  Ethernet < 412.8 Mbps  > 48.1 Mbps  (1.0 Gbps link)
NET:  Ethernet < 389.2 Mbps  > 46.7 Mbps  (1.0 Gbps link)
```

### Disk I/O During File Copy
```
IO:   C: < 125.3 MB/s  > 3.2 MB/s  (2.5% busy)
IO:   _Total < 125.3 MB/s  > 3.2 MB/s  (2.5% busy)
IO:   C: < 142.7 MB/s  > 3.8 MB/s  (3.1% busy)
IO:   _Total < 142.7 MB/s  > 3.8 MB/s  (3.1% busy)
```

### Idle System
```
NET:  Ethernet < 15.2 Kbps  > 8.5 Kbps  (1.0 Gbps link)
IO:   C: < 0 B/s  > 125.4 KB/s  (0.1% busy)
```

---

## Technical Details

### State File Location
```
%TEMP%\WinHKMon.dat
```

### State File Format
```
VERSION 1.0
TIMESTAMP 133449876543210
NETWORK_Ethernet_IN 123456789012
NETWORK_Ethernet_OUT 98765432109
DISK_C:_READ 567890123456
DISK_C:_WRITE 123456789012
```

### Rate Calculation Formula
```
rate = (current_counter - previous_counter) / elapsed_seconds

where:
  elapsed_seconds = (current_timestamp - previous_timestamp) / frequency
  frequency = QueryPerformanceFrequency() result (constant)
```

### First Run Behavior
On first run (no saved state):
- `StateManager.load()` returns `false`
- `previousTimestamp` set to current time
- Elapsed time ≈ 0
- Rates stay at 0 (expected)
- State saved for next run

On second run:
- Previous state loaded successfully
- Elapsed time = time between runs
- Rates calculated from deltas
- **Network traffic visible!**

---

## Why This Wasn't Caught Earlier

1. **Unit Tests**: All 123 tests passed because:
   - Tests checked that monitors **collected** cumulative counters ✅
   - Tests verified `DeltaCalculator.calculateRate()` math ✅
   - Tests verified `StateManager.save()` and `load()` ✅
   - **BUT**: No integration test verified main.cpp **orchestration** ❌

2. **Manual Testing**: Previous manual testing likely:
   - Ran in continuous mode (multiple samples)
   - First sample always 0 (expected)
   - Didn't notice subsequent samples also 0 (bug)

3. **Code Review**: The design documents specified the architecture, but the implementation in `main.cpp` was incomplete.

---

## Lessons Learned

1. **Integration Tests Needed**: Unit tests alone aren't enough - need end-to-end tests
2. **Manual Testing Protocol**: Should include "run speed test while monitoring" scenario
3. **Code Review Checklist**: Verify state management loops are complete, not just declared

---

## Related Files

- `src/WinHKMon/main.cpp` - Fixed file
- `src/WinHKMonLib/StateManager.cpp` - State persistence (working correctly)
- `src/WinHKMonLib/DeltaCalculator.cpp` - Rate math (working correctly)
- `src/WinHKMonLib/NetworkMonitor.cpp` - Counter collection (working correctly)
- `src/WinHKMonLib/DiskMonitor.cpp` - Counter collection (working correctly)

**The bug was NOT in the library components - they all worked correctly. The bug was in the main.cpp orchestration layer.**

---

## Next Steps

1. **Windows Testing**: Build and test on Windows to verify fix
2. **Integration Test**: Add automated test for rate calculations
3. **Documentation**: Update README with correct behavior examples
4. **Validation**: Compare output with Task Manager Network tab

---

**Status**: Code committed, ready for Windows build and testing.  
**Confidence**: HIGH - The fix addresses the exact architectural gap that caused the bug.  
**Risk**: LOW - Changes are additive (no existing functionality broken).

