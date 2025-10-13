# WinHKMon Implementation Plan

**Version:** 1.0
**Last Updated:** 2025-10-13
**Status:** Draft
**Feature Branch:** `feature/winhkmon-system-resource-monitoring`

This document translates requirements from `spec.md` into technical decisions and implementation strategy, following principles in `.specify/memory/constitution.md`.

---

## Constitution Check

**Pre-Implementation Gates** - Verify before proceeding:

- [x] **Library-First Architecture** (Principle 1): Core logic in `WinHKMonLib`, CLI wrapper separate
- [x] **Native Windows API Mandate** (Principle 2): Win32 APIs only, LibreHardwareMonitor for temperature
- [x] **CLI-First Interface** (Principle 3): Command-line as primary interface
- [x] **Test-First Development** (Principle 4): Tests defined before implementation (see Section IX)
- [x] **Simplicity and Anti-Abstraction** (Principle 5): Direct, minimal design - no over-engineering
- [x] **Integration-First Testing** (Principle 6): Real Windows API testing, validation against Task Manager

**Constitutional Compliance Summary:**
✅ All principles satisfied. Plan maintains library-first architecture with zero UI dependencies in core library. Uses only native Windows APIs (PDH, sysinfoapi, netioapi, powerbase) with LibreHardwareMonitor for temperature as permitted exception. CLI provides complete functionality. Tests are defined upfront with 80%+ coverage target. Design avoids unnecessary abstractions with direct API usage. Integration tests validate against real Windows systems and Task Manager.

---

## I. System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     WinHKMon.exe (CLI)                      │
│  - Command-line parsing                                     │
│  - Output formatting (text, JSON, CSV)                      │
│  - Main loop for continuous monitoring                      │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ uses
                     ↓
┌─────────────────────────────────────────────────────────────┐
│              WinHKMonLib.lib (Core Library)                 │
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ CpuMonitor   │  │ MemoryMonitor│  │ NetworkMonitor│     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ DiskMonitor  │  │ TempMonitor  │  │ StateManager │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│                                                              │
│  ┌─────────────────────────────────────────────────┐       │
│  │         SystemMetrics (Data Container)          │       │
│  └─────────────────────────────────────────────────┘       │
└────────────────────┬────────────────────────────────────────┘
                     │
                     │ calls
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                   Windows APIs                              │
│  - sysinfoapi.h (GlobalMemoryStatusEx)                     │
│  - pdh.h (Performance Data Helper)                         │
│  - netioapi.h (GetIfEntry2, MIB_IF_ROW2)                   │
│  - powerbase.h (CPU frequency)                             │
│  - LibreHardwareMonitor (Temperature - .NET interop)       │
└─────────────────────────────────────────────────────────────┘
```

**Rationale:**
- Library-first architecture (Constitution Principle 1)
- Clear separation of concerns
- Testable components
- No GUI dependencies in library
- CLI wrapper provides user interface

---

## II. Technology Stack

### Programming Language
- **Language**: C++17
- **Rationale**: Native performance, direct Windows API access, modern C++ features (std::optional, std::filesystem)

### Compiler
- **Compiler**: MSVC (Microsoft Visual C++) 2022 or later
- **Flags**: `/std:c++17 /W4 /WX` (treat warnings as errors)
- **Rationale**: Best Windows API support, aligns with constitution Principle 2

### Build System
- **Build Tool**: CMake 3.20+
- **Alternative**: MSBuild (Visual Studio solution files)
- **Rationale**: Cross-IDE support, industry standard, enables automated builds

### Testing Framework
- **Unit Tests**: Google Test (gtest) 1.12+
- **Rationale**: Industry standard, well-documented, actively maintained

### Dependencies

**Core Library (`WinHKMonLib`):**
- Windows SDK 10.0.19041+ (Windows 10 20H1 SDK)
- C++ Standard Library (STL)
- **No other dependencies** (constitution compliant)

**Temperature Monitoring (Optional):**
- LibreHardwareMonitor 0.9+ (via .NET interop or COM)
- **Alternative**: WMI `MSAcpi_ThermalZoneTemperature` (limited hardware support)

**CLI Application (`WinHKMon.exe`):**
- WinHKMonLib.lib
- Manual JSON generation (no external JSON library)

---

## III. Component Design

### Component 1: CpuMonitor

**Responsibility**: Collect CPU usage statistics

**Technical Approach**: Performance Data Helper (PDH) API

**Implementation**:
```cpp
class CpuMonitor {
public:
    void initialize();
    CpuStats getCurrentStats();
    void cleanup();

private:
    PDH_HQUERY hQuery_;
    PDH_HCOUNTER hCpuTotal_;
    std::vector<PDH_HCOUNTER> hCpuCores_;
};
```

**APIs Used:**
- `PdhOpenQuery()` - Initialize performance query
- `PdhAddEnglishCounter()` - Add counters:
  - `"\\Processor(_Total)\\% Processor Time"`
  - `"\\Processor(N)\\% Processor Time"` (per core)
- `PdhCollectQueryData()` - Collect sample
- `PdhGetFormattedCounterValue()` - Retrieve value
- `CallNtPowerInformation()` with `ProcessorInformation` for frequency

**Data Structure:**
```cpp
struct CpuStats {
    double totalUsagePercent;
    std::vector<CoreStats> cores;
    uint64_t averageFrequencyMhz;

    struct CoreStats {
        int coreId;
        double usagePercent;
        uint64_t frequencyMhz;
    };
};
```

**Rationale**: PDH provides high-level, stable API consistent with Performance Monitor. Handles per-core enumeration automatically.

**Alternative**: `GetSystemTimes()` for fallback if PDH unavailable (OneCore apps).

---

### Component 2: MemoryMonitor

**Responsibility**: Collect memory usage statistics

**Technical Approach**: Single API call to `GlobalMemoryStatusEx()`

**Implementation**:
```cpp
class MemoryMonitor {
public:
    MemoryStats getCurrentStats();
};
```

**APIs Used:**
- `GlobalMemoryStatusEx()` - Returns all needed memory data

**Data Structure:**
```cpp
struct MemoryStats {
    uint64_t totalPhysicalBytes;
    uint64_t availablePhysicalBytes;
    uint64_t usedPhysicalBytes;      // Calculated: total - available
    double usagePercent;              // Calculated: used / total

    uint64_t totalPageFileBytes;
    uint64_t availablePageFileBytes;
    uint64_t usedPageFileBytes;
    double pageFilePercent;
};
```

**Rationale**: Simple, fast, single API call. No PDH dependency needed.

---

### Component 3: DiskMonitor

**Responsibility**: Collect disk I/O statistics

**Technical Approach**: PDH Counters

**Implementation**:
```cpp
class DiskMonitor {
public:
    void initialize();
    std::vector<DiskStats> getCurrentStats();
    void cleanup();

private:
    PDH_HQUERY hQuery_;
    std::map<std::string, DiskCounters> counters_;

    struct DiskCounters {
        PDH_HCOUNTER bytesRead;
        PDH_HCOUNTER bytesWritten;
        PDH_HCOUNTER percentBusy;
    };
};
```

**APIs Used:**
- PDH counters:
  - `"\\PhysicalDisk(*)\\Disk Read Bytes/sec"`
  - `"\\PhysicalDisk(*)\\Disk Write Bytes/sec"`
  - `"\\PhysicalDisk(*)\\% Disk Time"`
- `GetDiskFreeSpaceEx()` - For disk size

**Data Structure:**
```cpp
struct DiskStats {
    std::string deviceName;          // e.g., "0 C:", "1 D:"
    uint64_t totalSizeBytes;
    uint64_t bytesReadPerSec;
    uint64_t bytesWrittenPerSec;
    double percentBusy;

    // Cumulative (requires state persistence)
    uint64_t totalBytesRead;
    uint64_t totalBytesWritten;
};
```

**Rationale**: PDH provides per-disk statistics automatically. Consistent with CPU monitoring approach.

---

### Component 4: NetworkMonitor

**Responsibility**: Collect network interface statistics

**Technical Approach**: IP Helper API (MIB_IF_ROW2)

**Implementation**:
```cpp
class NetworkMonitor {
public:
    void initialize();
    std::vector<InterfaceStats> getCurrentStats();
    std::string selectPrimaryInterface(const std::vector<InterfaceStats>& interfaces);
};
```

**APIs Used:**
- `GetIfTable2()` - Enumerate all interfaces
- `GetIfEntry2()` - Get detailed stats for specific interface

**Data Structure:**
```cpp
struct InterfaceStats {
    std::string name;                // InterfaceAlias (e.g., "Ethernet", "Wi-Fi")
    std::string description;         // InterfaceDescription
    uint64_t inOctets;               // Bytes received (cumulative)
    uint64_t outOctets;              // Bytes sent (cumulative)
    uint64_t inSpeed;                // Link speed (bits/sec)
    uint64_t outSpeed;
    bool isConnected;                // MediaConnectState

    // Calculated (requires previous sample)
    uint64_t inBytesPerSec;
    uint64_t outBytesPerSec;
};
```

**Primary Interface Selection Algorithm:**
```cpp
std::string NetworkMonitor::selectPrimaryInterface(
    const std::vector<InterfaceStats>& interfaces) {
    // 1. Exclude loopback
    // 2. Prefer interface with highest (inOctets + outOctets)
    // 3. If tie, prefer first connected Ethernet, then Wi-Fi
    // 4. Fallback: first non-loopback interface
}
```

**Rationale**: `MIB_IF_ROW2` provides exactly needed data. Modern API (Windows Vista+). No PDH dependency.

---

### Component 5: TempMonitor

**Responsibility**: Collect temperature sensor readings

**Technical Approach**: LibreHardwareMonitor Integration via C++/CLI Wrapper

**Implementation**:
```cpp
// TempMonitorWrapper.cpp (C++/CLI)
class TempMonitor {
public:
    void initialize();
    TempStats getCurrentStats();
    void cleanup();

private:
    // Managed wrapper to .NET library
    gcroot<Computer^> computer_;
};
```

**APIs Used:**
- LibreHardwareMonitor `Computer` class
- Access `Hardware` → `Sensors` → Filter by `SensorType.Temperature`

**Alternative Fallback**: WMI `MSAcpi_ThermalZoneTemperature` (rarely works, hardware-dependent)

**Data Structure:**
```cpp
struct TempStats {
    std::vector<SensorReading> cpuTemps;
    std::vector<SensorReading> gpuTemps;
    int maxCpuTempCelsius;          // Convenience field

    struct SensorReading {
        std::string name;            // e.g., "CPU Core #0"
        int tempCelsius;
        std::string hardwareType;    // "CPU", "GPU", etc.
    };
};
```

**Rationale**: LibreHardwareMonitor supports wide variety of sensors, actively maintained, well-tested. Constitution allows third-party libraries for temperature.

**Privilege Requirements**: Requires **Administrator** privileges for sensor access. Must document clearly and gracefully degrade if running without admin.

---

### Component 6: StateManager

**Responsibility**: Persist and retrieve previous monitoring state for delta calculations

**Technical Approach**: Text file in user temp directory

**Implementation**:
```cpp
class StateManager {
public:
    StateManager(const std::string& appName);

    bool load(SystemMetrics& previousState, uint64_t& timestamp);
    bool save(const SystemMetrics& currentState, uint64_t timestamp);

private:
    std::filesystem::path getStatePath();
    bool validateVersion(const std::string& version);
};
```

**File Location:**
```cpp
std::filesystem::path StateManager::getStatePath() {
    // %TEMP%\WinHKMon.dat
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    return std::filesystem::path(tempPath) / L"WinHKMon.dat";
}
```

**File Format:**
```
VERSION 1.0
TIMESTAMP 132891234567890
CPU 23.5 2400
DISK sda|1234567890|9876543210
NETWORK eth0|5678901234|1234567890
```

**Error Handling:**
- Missing file: OK (first run)
- Corrupted file: Log warning, delete, start fresh
- Version mismatch: Ignore, overwrite with current version

**Rationale**: Simple text format for debugging. Version header enables format evolution.

---

### Component 7: SystemMetrics (Data Container)

**Responsibility**: Central data structure holding all metrics

**Implementation**:
```cpp
struct SystemMetrics {
    std::optional<CpuStats> cpu;
    std::optional<MemoryStats> memory;
    std::optional<std::vector<DiskStats>> disks;
    std::optional<std::vector<InterfaceStats>> network;
    std::optional<TempStats> temperature;

    uint64_t timestamp;  // Monotonic clock (QueryPerformanceCounter)
};
```

**Rationale**: `std::optional` allows selective monitoring. Single structure simplifies data passing. Timestamp stored with metrics for delta calculations.

---

## IV. Command-Line Interface Design

### Argument Parsing Strategy

**Library**: Built-in parsing (no external dependencies)

**Data Structure**:
```cpp
struct CliOptions {
    bool showCpu = false;
    bool showMemory = false;
    bool showDisk = false;
    bool showNetwork = false;
    bool showTemp = false;

    std::string networkInterface;  // Empty = auto-select

    enum OutputFormat { TEXT, JSON, CSV } format = TEXT;
    bool singleLine = false;
    bool continuous = false;
    int intervalSeconds = 1;

    enum NetworkUnit { BITS, BYTES } networkUnit = BITS;
};

CliOptions parseArguments(int argc, char* argv[]);
```

### Usage Examples

```bash
# Show CPU and RAM
WinHKMon CPU RAM

# Show network stats for specific interface
WinHKMon "Ethernet 2"

# Continuous monitoring with 5-second interval
WinHKMon CPU RAM TEMP --continuous --interval 5

# JSON output for scripting
WinHKMon CPU RAM --format json

# Single-line output (status bar mode)
WinHKMon CPU RAM NET LINE

# All metrics
WinHKMon CPU RAM DISK NET TEMP

# Help
WinHKMon --help
```

### Exit Codes

- **0**: Success
- **1**: Invalid arguments / usage error
- **2**: Runtime error (API failure, permissions)
- **3**: State file error (non-fatal, but significant)

---

## V. Output Formatting

### Text Output Format

**Compact Mode** (default):
```
CPU:  23.5%  2.4 GHz
RAM:  8192M available
DISK: sda ↑ 15.3 MB/s  ↓ 2.1 MB/s
NET:  eth0 ↑ 2.1 Mbps  ↓ 15.3 Mbps
TEMP: 45°C
```

**Single-Line Mode** (`LINE` flag):
```
23.5% 2.4GHz  8192M  sda:15.3MB/s  eth0:2.1Mbps↑15.3Mbps↓  45°C
```

### JSON Output Format

```json
{
  "version": "1.0",
  "timestamp": "2025-10-13T14:32:15Z",
  "cpu": {
    "totalUsagePercent": 23.5,
    "averageFrequencyMhz": 2400,
    "cores": [
      {"id": 0, "usagePercent": 45.0, "frequencyMhz": 2800},
      {"id": 1, "usagePercent": 12.0, "frequencyMhz": 2100}
    ]
  },
  "memory": {
    "totalMB": 16384,
    "availableMB": 8192,
    "usedMB": 8192,
    "usagePercent": 50.0,
    "pageFile": {
      "totalMB": 8192,
      "usedMB": 2048,
      "usagePercent": 25.0
    }
  }
}
```

### CSV Output Format

```csv
timestamp,cpu_percent,cpu_mhz,ram_available_mb,ram_percent,disk_read_mbps,disk_write_mbps,net_recv_mbps,net_sent_mbps,temp_celsius
2025-10-13T14:32:15Z,23.5,2400,8192,50.0,2.1,15.3,15.3,2.1,52
```

**Implementation**:
```cpp
class OutputFormatter {
public:
    static std::string formatText(const SystemMetrics& metrics, bool singleLine);
    static std::string formatJson(const SystemMetrics& metrics);
    static std::string formatCsv(const SystemMetrics& metrics, bool includeHeader);
};
```

---

## VI. Delta Calculation Strategy

**Approach**:
1. Load previous state from `StateManager`
2. Collect current state
3. Calculate delta: `rate = (current - previous) / timeElapsed`
4. Save current state for next run

**Timestamp Handling**:
```cpp
// Use QueryPerformanceCounter for monotonic time
LARGE_INTEGER frequency, counter;
QueryPerformanceFrequency(&frequency);
QueryPerformanceCounter(&counter);
uint64_t timestamp = counter.QuadPart;

// Calculate elapsed time
double elapsedSeconds = (currentTimestamp - previousTimestamp) / 
                        (double)frequency.QuadPart;
```

**Rationale**: `QueryPerformanceCounter` is monotonic (unaffected by system time changes), high resolution, available on all Windows versions.

---

## VII. Error Handling Strategy

### Error Categories

**1. Fatal Errors** (exit immediately)
- Invalid command-line arguments
- Critical API initialization failures

**2. Degraded Mode** (continue with reduced functionality)
- Temperature monitoring fails → skip temperature
- Specific disk not accessible → skip that disk

**3. Transient Errors** (retry next cycle)
- Network interface temporarily unavailable
- API call timeout

### Implementation

```cpp
class MonitorException : public std::runtime_error {
public:
    enum class Severity { FATAL, DEGRADED, TRANSIENT };

    MonitorException(const std::string& message, Severity severity);
    Severity severity() const;
};
```

**Error Logging**: Log to stderr (not stdout) to preserve clean output.

---

## VIII. Testing Strategy

### Unit Tests

**Target Coverage**: 80% minimum

**Test Framework**: Google Test (gtest)

**Test Structure**:
```
tests/
├── CpuMonitorTest.cpp
├── MemoryMonitorTest.cpp
├── DiskMonitorTest.cpp
├── NetworkMonitorTest.cpp
├── TempMonitorTest.cpp
├── StateManagerTest.cpp
└── OutputFormatterTest.cpp
```

**Example Test**:
```cpp
TEST(MemoryMonitorTest, ReturnsValidMemoryStats) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    EXPECT_GT(stats.totalPhysicalBytes, 0);
    EXPECT_LE(stats.availablePhysicalBytes, stats.totalPhysicalBytes);
    EXPECT_GE(stats.usagePercent, 0.0);
    EXPECT_LE(stats.usagePercent, 100.0);
}
```

**Mock Strategy**: Avoid mocking Windows APIs for most tests (integration-first principle). Mock only for edge case simulation.

### Integration Tests

**Test Scenarios**:
1. **Accuracy Test**: Compare with Task Manager (< 5% variance)
2. **Performance Test**: CPU overhead < 1%
3. **Stability Test**: 24-hour continuous monitoring
4. **Compatibility Test**: Windows 10 21H2, Windows 11 22H2, 23H2
5. **Hardware Test**: Intel, AMD, ARM64, 2-16 cores, physical vs. VM

---

## IX. Build and Deployment

### Build Configuration

**CMakeLists.txt** structure:
```cmake
cmake_minimum_required(VERSION 3.20)
project(WinHKMon VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library
add_library(WinHKMonLib STATIC
    src/CpuMonitor.cpp
    src/MemoryMonitor.cpp
    src/DiskMonitor.cpp
    src/NetworkMonitor.cpp
    src/TempMonitor.cpp
    src/StateManager.cpp
    src/OutputFormatter.cpp
)

target_include_directories(WinHKMonLib PUBLIC include)
target_link_libraries(WinHKMonLib pdh iphlpapi powrprof)

# Executable
add_executable(WinHKMon src/main.cpp)
target_link_libraries(WinHKMon WinHKMonLib)

# Tests
enable_testing()
add_subdirectory(tests)
```

### Compiler Flags

**Release**: `/O2 /GL /W4 /WX` (optimize, whole program, warnings as errors)
**Debug**: `/Od /Zi /W4 /WX` (no optimization, debug info, warnings)

### Deployment Package

```
WinHKMon-v1.0-x64.zip
├── WinHKMon.exe
├── LibreHardwareMonitor/  (Temperature support)
├── README.md
├── LICENSE.txt
└── CHANGELOG.md
```

**Distribution**: GitHub Releases (portable ZIP, no installer required)

---

## X. Security Considerations

### Input Validation

- CLI arguments: Validate interval range (0-3600 seconds)
- State file: Check size (< 1 MB), validate version, sanitize parsed values

### File Permissions

```cpp
void StateManager::setSecurePermissions(const std::filesystem::path& filePath) {
    // Windows: Set ACL to current user only
    // (Full implementation uses SECURITY_DESCRIPTOR)
}
```

### Privilege Management

**Administrator Check**:
```cpp
bool isRunningAsAdmin() {
    // Check if current process has admin privileges
    // Required only for temperature monitoring
}
```

**Graceful Degradation**:
```cpp
if (!isRunningAsAdmin() && options.showTemp) {
    std::cerr << "[WARNING] Temperature monitoring requires "
              << "Administrator privileges. Skipping." << std::endl;
    options.showTemp = false;
}
```

---

## XI. Performance Optimization

### Strategies

1. **Minimize API Calls**: Single `GlobalMemoryStatusEx()` for all memory metrics
2. **Efficient Data Structures**: Use `std::vector::reserve()`, move semantics
3. **Lazy Initialization**: Only initialize monitors for requested metrics
4. **Caching**: Cache PDH counter handles, network interface lists

**Benchmark Targets**:
- Startup: < 200ms
- Sample collection: < 50ms
- CPU overhead: < 0.5% average

---

## XII. Implementation Timeline

**Phase 1: Foundation** (2 weeks)
- Project setup, CMake, directory structure
- Library skeleton, CLI argument parsing
- Unit test framework setup

**Phase 2: Core Monitors** (3 weeks)
- CpuMonitor, MemoryMonitor, NetworkMonitor, DiskMonitor
- StateManager
- Unit tests for each component

**Phase 3: Advanced Features** (2 weeks)
- TempMonitor (LibreHardwareMonitor integration)
- OutputFormatter (text, JSON, CSV)
- Continuous monitoring mode
- Integration tests

**Phase 4: Polish** (1 week)
- Error handling refinement
- Performance optimization
- Documentation

**Phase 5: Testing & Release** (1 week)
- Compatibility testing
- Hardware testing
- Security review
- Package and release

**Total**: ~9 weeks

---

## XIII. Risks and Mitigations

| Risk | Impact | Mitigation | Likelihood |
|------|--------|------------|------------|
| LibreHardwareMonitor unavailable | Temperature monitoring fails | Make optional, document requirement | Low |
| PDH API limitations | Metrics unavailable | Use native APIs as fallback | Low |
| Performance overhead | Tool impacts system | Early benchmarking, lazy init | Medium |
| Accuracy discrepancies | Values don't match Task Manager | Validation tests, use same APIs | Medium |
| Windows API changes | APIs deprecated | Use stable, long-standing APIs | Very Low |

---

## XIV. Success Criteria

Implementation is successful if:

1. **Functional**: All specified metrics collected and displayed
2. **Performant**: < 1% CPU overhead, < 10 MB memory
3. **Accurate**: Within 5% of Task Manager/Performance Monitor
4. **Reliable**: Zero crashes in 24-hour continuous test
5. **Tested**: 80%+ code coverage, passes all integration tests
6. **Compatible**: Works on Windows 10 21H2+ and Windows 11
7. **Documented**: Complete README, API docs, examples

---

## XV. Open Technical Decisions

**TD-1: Static vs. Dynamic Linking**
- **Recommendation**: Static library for v1.0 (simpler deployment)

**TD-2: JSON Library Selection**
- **Recommendation**: Manual JSON generation (simple output, no dependencies)

**TD-3: Temperature Monitoring Implementation**
- **Recommendation**: C++/CLI wrapper for LibreHardwareMonitor (best UX)

**TD-4: Configuration File Support**
- **Recommendation**: Defer to v1.1, CLI args sufficient for v1.0

---

**Document Control:**
- **Author**: WinHKMon Implementation Team
- **Reviewers**: TBD
- **Approval Date**: Pending
- **Next Phase**: Generate tasks.md and begin implementation
- **Related Documents**:
  - [Specification](./spec.md)
  - [Research Findings](./research.md)
  - [Data Model](./specs/data-model.md)
  - [CLI Contract](./contracts/cli-interface.md)
