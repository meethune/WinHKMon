# WinHKMon Data Model

**Phase:** 1 (Design & Contracts)
**Date:** 2025-10-13
**Status:** Complete

This document defines all data structures used in WinHKMon, extracted from functional requirements in the specification.

---

## Core Data Structures

### SystemMetrics

**Purpose**: Central container for all collected metrics at a specific point in time

**Structure**:
```cpp
struct SystemMetrics {
    std::optional<CpuStats> cpu;
    std::optional<MemoryStats> memory;
    std::optional<std::vector<DiskStats>> disks;
    std::optional<std::vector<InterfaceStats>> network;
    std::optional<TempStats> temperature;
    
    uint64_t timestamp;  // Monotonic timestamp (QueryPerformanceCounter)
};
```

**Fields**:
| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| cpu | `std::optional<CpuStats>` | CPU metrics (optional if not requested) | Present if CPU monitoring enabled |
| memory | `std::optional<MemoryStats>` | Memory metrics | Present if RAM monitoring enabled |
| disks | `std::optional<std::vector<DiskStats>>` | Disk I/O metrics per physical disk | Present if DISK monitoring enabled |
| network | `std::optional<std::vector<InterfaceStats>>` | Network metrics per interface | Present if NET monitoring enabled |
| temperature | `std::optional<TempStats>` | Temperature sensor readings | Present if TEMP monitoring enabled |
| timestamp | `uint64_t` | Monotonic time when metrics captured | Always > 0, monotonically increasing |

**Rationale**: 
- `std::optional` enables selective monitoring (only collect requested metrics)
- Single timestamp ensures all metrics are synchronized
- Vector types allow multiple disks/interfaces

---

### CpuStats

**Purpose**: CPU usage and frequency information

**Structure**:
```cpp
struct CpuStats {
    double totalUsagePercent;
    std::vector<CoreStats> cores;
    uint64_t averageFrequencyMhz;
    
    // Optional breakdown (if available from API)
    std::optional<double> userPercent;
    std::optional<double> systemPercent;
    std::optional<double> idlePercent;
    
    struct CoreStats {
        int coreId;
        double usagePercent;
        uint64_t frequencyMhz;
    };
};
```

**Fields**:
| Field | Type | Description | Validation | Source Requirement |
|-------|------|-------------|------------|-------------------|
| totalUsagePercent | `double` | Overall CPU usage (0-100%) | 0.0 ≤ value ≤ 100.0 | FR-1.1 |
| cores | `vector<CoreStats>` | Per-core statistics | size ≥ 1, size ≤ 256 | FR-1.2 |
| averageFrequencyMhz | `uint64_t` | Average frequency across cores | > 0, typically 800-6000 MHz | FR-1.3 |
| userPercent | `optional<double>` | User-mode CPU time | 0.0 ≤ value ≤ 100.0 | FR-1.4 |
| systemPercent | `optional<double>` | Kernel-mode CPU time | 0.0 ≤ value ≤ 100.0 | FR-1.4 |
| idlePercent | `optional<double>` | Idle time | 0.0 ≤ value ≤ 100.0 | FR-1.4 |

**CoreStats Fields**:
| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| coreId | `int` | Logical processor ID | 0 ≤ value < 256 |
| usagePercent | `double` | Per-core usage percentage | 0.0 ≤ value ≤ 100.0 |
| frequencyMhz | `uint64_t` | Current frequency in MHz | > 0 |

**Invariants**:
- `cores.size()` matches logical processor count
- If userPercent, systemPercent, idlePercent all present: `user + system + idle ≈ 100.0` (within 1%)
- `coreId` values are unique within cores vector

---

### MemoryStats

**Purpose**: Physical and virtual memory statistics

**Structure**:
```cpp
struct MemoryStats {
    uint64_t totalPhysicalBytes;
    uint64_t availablePhysicalBytes;
    uint64_t usedPhysicalBytes;
    double usagePercent;
    
    uint64_t totalPageFileBytes;
    uint64_t availablePageFileBytes;
    uint64_t usedPageFileBytes;
    double pageFilePercent;
    
    // Optional: cached memory breakdown
    std::optional<uint64_t> cachedBytes;
    std::optional<uint64_t> committedBytes;
};
```

**Fields**:
| Field | Type | Description | Validation | Source Requirement |
|-------|------|-------------|------------|-------------------|
| totalPhysicalBytes | `uint64_t` | Total RAM installed | ≥ 1 GB (practical minimum) | FR-2.1 |
| availablePhysicalBytes | `uint64_t` | RAM available for allocation | ≤ totalPhysicalBytes | FR-2.2 |
| usedPhysicalBytes | `uint64_t` | RAM currently in use | = total - available | FR-2.3 |
| usagePercent | `double` | RAM usage percentage | 0.0 ≤ value ≤ 100.0 | Calculated |
| totalPageFileBytes | `uint64_t` | Total page file size | ≥ 0 | FR-2.4 |
| availablePageFileBytes | `uint64_t` | Page file available | ≤ totalPageFileBytes | FR-2.4 |
| usedPageFileBytes | `uint64_t` | Page file in use | = total - available | FR-2.4 |
| pageFilePercent | `double` | Page file usage % | 0.0 ≤ value ≤ 100.0 | FR-2.4 |
| cachedBytes | `optional<uint64_t>` | File cache size | ≤ totalPhysicalBytes | FR-2.3 |
| committedBytes | `optional<uint64_t>` | Committed memory | Can exceed physical | FR-2.3 |

**Invariants**:
- `usedPhysicalBytes = totalPhysicalBytes - availablePhysicalBytes`
- `usagePercent = (usedPhysicalBytes / totalPhysicalBytes) * 100.0`
- `usedPageFileBytes = totalPageFileBytes - availablePageFileBytes`
- `pageFilePercent = (usedPageFileBytes / totalPageFileBytes) * 100.0` (or 0 if no page file)

---

### DiskStats

**Purpose**: Disk I/O statistics per physical drive

**Structure**:
```cpp
struct DiskStats {
    std::string deviceName;
    uint64_t totalSizeBytes;
    
    // Real-time rates
    uint64_t bytesReadPerSec;
    uint64_t bytesWrittenPerSec;
    double percentBusy;
    
    // Cumulative counters
    uint64_t totalBytesRead;
    uint64_t totalBytesWritten;
    
    // Optional: IOPS
    std::optional<uint64_t> readsPerSec;
    std::optional<uint64_t> writesPerSec;
};
```

**Fields**:
| Field | Type | Description | Validation | Source Requirement |
|-------|------|-------------|------------|-------------------|
| deviceName | `string` | Physical disk identifier | Non-empty, e.g., "0 C:", "1 D:" | FR-3.1 |
| totalSizeBytes | `uint64_t` | Disk capacity | > 0 | FR-3.5 |
| bytesReadPerSec | `uint64_t` | Current read rate | ≥ 0 | FR-3.4 |
| bytesWrittenPerSec | `uint64_t` | Current write rate | ≥ 0 | FR-3.4 |
| percentBusy | `double` | Disk active time % | 0.0 ≤ value ≤ 100.0 | FR-3.6 |
| totalBytesRead | `uint64_t` | Cumulative reads since boot | ≥ 0, monotonic | FR-3.2 |
| totalBytesWritten | `uint64_t` | Cumulative writes since boot | ≥ 0, monotonic | FR-3.3 |
| readsPerSec | `optional<uint64_t>` | Read operations/sec | ≥ 0 | Optional |
| writesPerSec | `optional<uint64_t>` | Write operations/sec | ≥ 0 | Optional |

**Invariants**:
- `deviceName` uniquely identifies physical disk
- `totalBytesRead` and `totalBytesWritten` are monotonically increasing
- `bytesReadPerSec` and `bytesWrittenPerSec` calculated from deltas

---

### InterfaceStats

**Purpose**: Network interface statistics

**Structure**:
```cpp
struct InterfaceStats {
    std::string name;
    std::string description;
    
    // Connection state
    bool isConnected;
    uint64_t linkSpeedBitsPerSec;
    
    // Real-time rates
    uint64_t inBytesPerSec;
    uint64_t outBytesPerSec;
    
    // Cumulative counters
    uint64_t totalInOctets;
    uint64_t totalOutOctets;
    
    // Optional: packet-level stats
    std::optional<uint64_t> inPacketsPerSec;
    std::optional<uint64_t> outPacketsPerSec;
    std::optional<uint64_t> inErrors;
    std::optional<uint64_t> outErrors;
};
```

**Fields**:
| Field | Type | Description | Validation | Source Requirement |
|-------|------|-------------|------------|-------------------|
| name | `string` | Interface alias | Non-empty, e.g., "Ethernet", "Wi-Fi" | FR-4.1 |
| description | `string` | Hardware description | Non-empty | FR-4.1 |
| isConnected | `bool` | Media connection state | true or false | FR-4.1 |
| linkSpeedBitsPerSec | `uint64_t` | Max negotiated speed | > 0 if connected | FR-4.7 |
| inBytesPerSec | `uint64_t` | Current download rate | ≥ 0 | FR-4.4 |
| outBytesPerSec | `uint64_t` | Current upload rate | ≥ 0 | FR-4.4 |
| totalInOctets | `uint64_t` | Total bytes received | ≥ 0, monotonic | FR-4.2 |
| totalOutOctets | `uint64_t` | Total bytes sent | ≥ 0, monotonic | FR-4.3 |
| inPacketsPerSec | `optional<uint64_t>` | Packets received/sec | ≥ 0 | Optional |
| outPacketsPerSec | `optional<uint64_t>` | Packets sent/sec | ≥ 0 | Optional |
| inErrors | `optional<uint64_t>` | Cumulative receive errors | ≥ 0 | Optional |
| outErrors | `optional<uint64_t>` | Cumulative transmit errors | ≥ 0 | Optional |

**Invariants**:
- `name` uniquely identifies interface
- `totalInOctets` and `totalOutOctets` are monotonically increasing
- If `isConnected == false`, rates may be 0
- `linkSpeedBitsPerSec` should be 0 or power of 10 (10 Mbps, 100 Mbps, 1 Gbps, etc.)

---

### TempStats

**Purpose**: Temperature sensor readings

**Structure**:
```cpp
struct TempStats {
    std::vector<SensorReading> cpuTemps;
    std::vector<SensorReading> gpuTemps;
    std::vector<SensorReading> otherTemps;
    
    int maxCpuTempCelsius;
    std::optional<int> minCpuTempCelsius;
    std::optional<int> avgCpuTempCelsius;
    
    struct SensorReading {
        std::string name;
        int tempCelsius;
        std::string hardwareType;  // "CPU", "GPU", "Motherboard", etc.
    };
};
```

**Fields**:
| Field | Type | Description | Validation | Source Requirement |
|-------|------|-------------|------------|-------------------|
| cpuTemps | `vector<SensorReading>` | CPU sensor readings | size ≥ 0 (may be empty) | FR-5.1, FR-5.2 |
| gpuTemps | `vector<SensorReading>` | GPU sensor readings | size ≥ 0 | FR-5.4 |
| otherTemps | `vector<SensorReading>` | Other sensor readings | size ≥ 0 | FR-5.5 |
| maxCpuTempCelsius | `int` | Maximum CPU temp | -40 ≤ value ≤ 150 (practical) | FR-5.3 |
| minCpuTempCelsius | `optional<int>` | Minimum CPU temp | -40 ≤ value ≤ 150 | FR-5.3 |
| avgCpuTempCelsius | `optional<int>` | Average CPU temp | -40 ≤ value ≤ 150 | FR-5.3 |

**SensorReading Fields**:
| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| name | `string` | Sensor identifier | Non-empty, e.g., "CPU Core #0" |
| tempCelsius | `int` | Temperature in Celsius | -40 ≤ value ≤ 150 (practical range) |
| hardwareType | `string` | Hardware category | "CPU", "GPU", "Motherboard", etc. |

**Invariants**:
- If `cpuTemps` not empty: `maxCpuTempCelsius` equals max of `cpuTemps[*].tempCelsius`
- If `minCpuTempCelsius` present: `minCpuTempCelsius ≤ maxCpuTempCelsius`
- If `avgCpuTempCelsius` present: `minCpuTempCelsius ≤ avgCpuTempCelsius ≤ maxCpuTempCelsius`

---

## State Persistence Data

### MonitorState

**Purpose**: Persistent state for delta calculations between runs

**Structure**:
```cpp
struct MonitorState {
    std::string version;
    uint64_t timestamp;
    
    // Previous counters for rate calculations
    std::map<std::string, uint64_t> networkInOctets;
    std::map<std::string, uint64_t> networkOutOctets;
    std::map<std::string, uint64_t> diskBytesRead;
    std::map<std::string, uint64_t> diskBytesWritten;
};
```

**Fields**:
| Field | Type | Description | Validation |
|-------|------|-------------|------------|
| version | `string` | State file format version | Format: "MAJOR.MINOR" |
| timestamp | `uint64_t` | Previous sample timestamp | QueryPerformanceCounter value |
| networkInOctets | `map<string, uint64_t>` | Previous inbound bytes by interface | Key = interface name |
| networkOutOctets | `map<string, uint64_t>` | Previous outbound bytes by interface | Key = interface name |
| diskBytesRead | `map<string, uint64_t>` | Previous read bytes by disk | Key = device name |
| diskBytesWritten | `map<string, uint64_t>` | Previous write bytes by disk | Key = device name |

**File Format** (text-based):
```
VERSION 1.0
TIMESTAMP 133449876543210000
NETWORK_Ethernet_IN 1234567890123
NETWORK_Ethernet_OUT 9876543210987
NETWORK_Wi-Fi_IN 5555555555555
NETWORK_Wi-Fi_OUT 6666666666666
DISK_0_READ 5678901234567
DISK_0_WRITE 1234567890123
DISK_1_READ 9999999999999
DISK_1_WRITE 8888888888888
```

**Validation Rules**:
- Version string must match `^\d+\.\d+$` regex
- Timestamp must be > 0
- All counter values must be ≥ 0
- Keys must not contain pipe `|` or newline characters

---

## Configuration Data

### CliOptions

**Purpose**: Parsed command-line options

**Structure**:
```cpp
struct CliOptions {
    bool showCpu;
    bool showMemory;
    bool showDisk;
    bool showNetwork;
    bool showTemp;
    
    std::string networkInterface;  // Empty = auto-select
    
    enum OutputFormat { TEXT, JSON, CSV } format;
    bool singleLine;
    bool continuous;
    int intervalSeconds;
    
    enum NetworkUnit { BITS, BYTES } networkUnit;
    
    // Help/version flags
    bool showHelp;
    bool showVersion;
};
```

**Fields**:
| Field | Type | Description | Default | Validation | Source Requirement |
|-------|------|-------------|---------|------------|-------------------|
| showCpu | `bool` | Monitor CPU | false | - | FR-8.1 |
| showMemory | `bool` | Monitor memory | false | - | FR-8.1 |
| showDisk | `bool` | Monitor disk I/O | false | - | FR-8.1 |
| showNetwork | `bool` | Monitor network | false | - | FR-8.1 |
| showTemp | `bool` | Monitor temperature | false | - | FR-8.1 |
| networkInterface | `string` | Specific interface | "" (auto) | Must match existing interface | FR-8.2 |
| format | `OutputFormat` | Output format | TEXT | TEXT, JSON, or CSV | FR-8.3 |
| singleLine | `bool` | Single-line mode | false | - | FR-8.6 |
| continuous | `bool` | Continuous monitoring | false | - | FR-8.4 |
| intervalSeconds | `int` | Update interval | 1 | 0.1 ≤ value ≤ 3600 | FR-8.5 |
| networkUnit | `NetworkUnit` | Network speed unit | BITS | BITS or BYTES | FR-8.9 |
| showHelp | `bool` | Display help | false | - | FR-8.8 |
| showVersion | `bool` | Display version | false | - | FR-8.7 |

**Validation Rules**:
- At least one of showCpu, showMemory, showDisk, showNetwork, showTemp must be true (unless showHelp or showVersion)
- If `networkInterface` non-empty, must match existing interface name
- `intervalSeconds` must be in range [0.1, 3600]

---

## Data Flow

### Collection Flow
```
1. User invokes: WinHKMon CPU RAM NET
2. CLI parses → CliOptions {showCpu=true, showMemory=true, showNetwork=true}
3. Monitors initialized: CpuMonitor, MemoryMonitor, NetworkMonitor
4. StateManager loads previous state (if exists)
5. Monitors collect current data → SystemMetrics
6. Delta calculations: (current - previous) / timeElapsed
7. OutputFormatter formats based on CliOptions.format
8. StateManager saves current state
9. Output to stdout
```

### Delta Calculation Example
```cpp
// Network rate calculation
uint64_t previousInOctets = state.networkInOctets["Ethernet"];
uint64_t currentInOctets = interface.totalInOctets;
uint64_t deltaOctets = currentInOctets - previousInOctets;

double elapsedSeconds = (currentTimestamp - state.timestamp) / frequency;
interface.inBytesPerSec = deltaOctets / elapsedSeconds;
```

---

## Type Mapping to Windows APIs

| Internal Type | Windows API Type | API | Notes |
|---------------|------------------|-----|-------|
| `double usagePercent` | `PDH_FMT_COUNTERVALUE.doubleValue` | PDH | Range: 0.0-100.0 |
| `uint64_t frequencyMhz` | `PROCESSOR_POWER_INFORMATION.CurrentMhz` | powerbase.h | 32-bit in API, cast to 64-bit |
| `uint64_t totalPhysicalBytes` | `MEMORYSTATUSEX.ullTotalPhys` | sysinfoapi.h | Direct mapping |
| `uint64_t inOctets` | `MIB_IF_ROW2.InOctets` | netioapi.h | Direct mapping |
| `int tempCelsius` | `ISensor.Value` (.NET) | LibreHardwareMonitor | Convert float to int |

---

**Document Control:**
- **Completed By**: WinHKMon Design Team
- **Review Date**: 2025-10-13
- **Status**: Complete - all entities defined
- **Related Documents**: [Specification](../spec.md), [Implementation Plan](../plan.md)

