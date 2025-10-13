# WinHKMon Research Findings

**Phase:** 0 (Outline & Research)
**Date:** 2025-10-13
**Status:** Complete

This document consolidates research findings for technical decisions and resolves all "NEEDS CLARIFICATION" items from the planning phase.

---

## Research Task 1: CPU Monitoring API Selection

### Decision
**Selected**: Performance Data Helper (PDH) API

### Rationale
1. **High-level abstraction**: PDH provides pre-calculated CPU percentages, eliminating complex manual calculations
2. **Consistency**: Same API used by Windows Performance Monitor and Task Manager (high compatibility)
3. **Per-core support**: Automatic enumeration of logical processors via `\\Processor(*)\\` wildcards
4. **Proven stability**: API exists since Windows 2000, well-documented and stable

### Alternatives Considered

**Alternative A: GetSystemTimes() + GetProcessorCount()**
- **Pros**: Lower-level, no PDH dependency, suitable for OneCore apps
- **Cons**: Manual calculation required (delta of kernel/user/idle times), more complex error handling
- **Verdict**: Keep as fallback if PDH unavailable

**Alternative B: WMI Win32_Processor queries**
- **Pros**: Rich metadata available
- **Cons**: Significantly slower (100-500ms per query), requires COM initialization, overkill for metrics
- **Verdict**: Rejected

### Implementation Notes
```cpp
// Initialize PDH query once
PDH_HQUERY hQuery;
PdhOpenQuery(NULL, 0, &hQuery);

// Add counters for total and per-core
PdhAddEnglishCounter(hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &hCpuTotal);

// Per-core: iterate 0 to N-1
for (int i = 0; i < coreCount; i++) {
    wchar_t counterPath[256];
    swprintf(counterPath, 256, L"\\Processor(%d)\\%% Processor Time", i);
    PdhAddEnglishCounter(hQuery, counterPath, 0, &hCpuCores[i]);
}

// Collect samples (need two samples for percentage calculation)
PdhCollectQueryData(hQuery);
Sleep(1000); // Wait between samples
PdhCollectQueryData(hQuery);
```

**Source**: 
- [Microsoft Docs: Using PDH Functions](https://docs.microsoft.com/en-us/windows/win32/perfctrs/using-the-pdh-functions)
- [Performance Counters Reference](https://docs.microsoft.com/en-us/windows/win32/perfctrs/performance-counters-functions)

---

## Research Task 2: CPU Frequency Monitoring

### Decision
**Selected**: CallNtPowerInformation() with ProcessorInformation class

### Rationale
1. **Per-core frequency**: Returns array of `PROCESSOR_POWER_INFORMATION` with `CurrentMhz` field
2. **Direct API**: No PDH overhead, fast execution
3. **Widespread support**: Works on Intel, AMD, ARM64

### Implementation Notes
```cpp
#include <powerbase.h>

SYSTEM_POWER_CAPABILITIES powerCaps;
std::vector<PROCESSOR_POWER_INFORMATION> procInfo(coreCount);

NTSTATUS status = CallNtPowerInformation(
    ProcessorInformation,
    NULL, 0,
    &procInfo[0], sizeof(PROCESSOR_POWER_INFORMATION) * coreCount
);

for (const auto& info : procInfo) {
    uint64_t freqMhz = info.CurrentMhz;
    // Store in CoreStats
}
```

**Alternative**: WMI `Win32_Processor.CurrentClockSpeed`
- **Verdict**: Rejected - too slow, often returns nominal frequency instead of current

**Source**: [CallNtPowerInformation Documentation](https://docs.microsoft.com/en-us/windows/win32/api/powerbase/nf-powerbase-callntpowerinformation)

---

## Research Task 3: Network Interface Monitoring API Selection

### Decision
**Selected**: IP Helper API with MIB_IF_ROW2 structures

### Rationale
1. **Complete data**: `MIB_IF_ROW2` provides all required fields (InOctets, OutOctets, speed, connection state)
2. **Modern API**: Windows Vista+, within our support range (Windows 10+)
3. **No PDH dependency**: Direct API access, faster than PDH network counters

### Implementation Notes
```cpp
#include <netioapi.h>

// Enumerate all interfaces
PMIB_IF_TABLE2 pIfTable = NULL;
GetIfTable2(&pIfTable);

for (ULONG i = 0; i < pIfTable->NumEntries; i++) {
    MIB_IF_ROW2* pIfRow = &pIfTable->Table[i];
    
    // Skip loopback
    if (pIfRow->Type == IF_TYPE_SOFTWARE_LOOPBACK) continue;
    
    InterfaceStats stats;
    stats.name = wstringToString(pIfRow->Alias);  // e.g., "Ethernet"
    stats.inOctets = pIfRow->InOctets;             // Bytes received
    stats.outOctets = pIfRow->OutOctets;           // Bytes sent
    stats.inSpeed = pIfRow->ReceiveLinkSpeed;      // Bits/sec
    stats.isConnected = (pIfRow->MediaConnectState == MediaConnectStateConnected);
}

FreeMibTable(pIfTable);
```

**Alternative**: PDH `\\Network Interface(*)\\Bytes Received/sec`
- **Verdict**: Rejected - slower, requires PDH initialization, less direct

**Source**: [GetIfTable2 Documentation](https://docs.microsoft.com/en-us/windows/win32/api/netioapi/nf-netioapi-getiftable2)

---

## Research Task 4: Temperature Monitoring Integration

### Decision
**Selected**: LibreHardwareMonitor via C++/CLI wrapper

### Rationale
1. **Wide hardware support**: Intel, AMD, NVIDIA, AMD GPUs, motherboard sensors
2. **Active maintenance**: Last release Nov 2024, community-supported
3. **MPL 2.0 license**: Permissive, compatible with our project
4. **Constitution-compliant**: Principle 2 allows third-party libraries for temperature monitoring

### Integration Approach: C++/CLI Wrapper

**Pros**:
- Native C++ library can call managed code directly
- Better performance than subprocess approach
- Type safety with managed references

**Cons**:
- Requires /clr compilation flag for wrapper module
- Core library remains native (only wrapper is C++/CLI)

**Architecture**:
```
WinHKMonLib (Native C++) 
  → TempMonitorWrapper (C++/CLI)
    → LibreHardwareMonitorLib.dll (.NET)
```

### Implementation Notes
```cpp
// TempMonitorWrapper.cpp (compiled with /clr)
#using <LibreHardwareMonitorLib.dll>

using namespace LibreHardwareMonitor::Hardware;

class TempMonitor {
private:
    gcroot<Computer^> computer_;

public:
    void initialize() {
        computer_ = gcnew Computer();
        computer_->IsCpuEnabled = true;
        computer_->IsGpuEnabled = true;
        computer_->Open();
    }

    TempStats getCurrentStats() {
        TempStats stats;
        
        computer_->Accept(gcnew UpdateVisitor());
        
        for each (IHardware^ hardware in computer_->Hardware) {
            if (hardware->HardwareType == HardwareType::CPU) {
                for each (ISensor^ sensor in hardware->Sensors) {
                    if (sensor->SensorType == SensorType::Temperature) {
                        SensorReading reading;
                        reading.name = msclr::interop::marshal_as<std::string>(sensor->Name);
                        reading.tempCelsius = static_cast<int>(sensor->Value.GetValueOrDefault());
                        stats.cpuTemps.push_back(reading);
                    }
                }
            }
        }
        
        return stats;
    }
};
```

### Fallback: WMI MSAcpi_ThermalZoneTemperature
- **Use case**: If LibreHardwareMonitor unavailable
- **Limitation**: Very limited hardware support (mostly laptops)
- **Query**: `SELECT * FROM MSAcpi_ThermalZoneTemperature`

**Source**: 
- [LibreHardwareMonitor GitHub](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor)
- [C++/CLI Documentation](https://docs.microsoft.com/en-us/cpp/dotnet/mixed-native-and-managed-assemblies)

---

## Research Task 5: State File Format and Location

### Decision
**Format**: Simple text-based format with version header
**Location**: `%TEMP%\WinHKMon.dat`

### Rationale
1. **Debuggability**: Text format easily inspected by users and developers
2. **Simplicity**: No serialization library needed
3. **Temp directory**: Automatically cleaned by Windows, appropriate for transient data
4. **User-specific**: Each user has their own temp directory (security boundary)

### File Format Specification
```
VERSION 1.0
TIMESTAMP 133449876543210000
NETWORK_eth0_IN 1234567890123
NETWORK_eth0_OUT 9876543210987
DISK_0_READ 5678901234567
DISK_0_WRITE 1234567890123
```

**Format Rules**:
- Line 1: `VERSION <major>.<minor>`
- Line 2: `TIMESTAMP <win32-filetime>` (100-nanosecond intervals since 1601)
- Remaining: `<METRIC>_<ID>_<FIELD> <value>`

**Alternative Formats Considered**:

**Binary Format**:
- **Pros**: Smaller, faster to parse
- **Cons**: Not human-readable, harder to debug
- **Verdict**: Defer to v1.1 if performance becomes issue

**JSON Format**:
- **Pros**: Structured, extensible
- **Cons**: Requires JSON library or manual generation/parsing
- **Verdict**: Overkill for simple state storage

### Security Considerations
```cpp
// Set file permissions to current user only
SECURITY_ATTRIBUTES sa;
sa.nLength = sizeof(SECURITY_ATTRIBUTES);
sa.lpSecurityDescriptor = NULL;  // Use default (current user)
sa.bInheritHandle = FALSE;

HANDLE hFile = CreateFileW(
    statePath.c_str(),
    GENERIC_WRITE,
    0,  // No sharing
    &sa,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL
);
```

**Source**: [GetTempPath Documentation](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppathw)

---

## Research Task 6: JSON Generation Strategy

### Decision
**Selected**: Manual JSON string generation (no external library)

### Rationale
1. **Zero dependencies**: Keeps core library dependency-free (constitution compliance)
2. **Simple output**: Our JSON structure is straightforward, no complex nesting
3. **Performance**: Direct string building is fast for simple structures
4. **Small code**: < 200 lines for complete JSON formatter

### Implementation Pattern
```cpp
std::string OutputFormatter::formatJson(const SystemMetrics& metrics) {
    std::ostringstream json;
    json << "{\n";
    json << "  \"version\": \"1.0\",\n";
    json << "  \"timestamp\": \"" << formatTimestamp(metrics.timestamp) << "\",\n";
    
    if (metrics.cpu) {
        json << "  \"cpu\": {\n";
        json << "    \"totalUsagePercent\": " << metrics.cpu->totalUsagePercent << ",\n";
        json << "    \"averageFrequencyMhz\": " << metrics.cpu->averageFrequencyMhz << ",\n";
        json << "    \"cores\": [\n";
        
        for (size_t i = 0; i < metrics.cpu->cores.size(); i++) {
            const auto& core = metrics.cpu->cores[i];
            json << "      {\"id\": " << core.coreId 
                 << ", \"usagePercent\": " << core.usagePercent
                 << ", \"frequencyMhz\": " << core.frequencyMhz << "}";
            if (i < metrics.cpu->cores.size() - 1) json << ",";
            json << "\n";
        }
        
        json << "    ]\n";
        json << "  }";
        if (metrics.memory || metrics.disks || metrics.network || metrics.temperature) {
            json << ",";
        }
        json << "\n";
    }
    
    // ... similar for other metrics
    
    json << "}\n";
    return json.str();
}
```

**String Escaping**:
```cpp
std::string escapeJson(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:   escaped += c;
        }
    }
    return escaped;
}
```

### Alternatives Considered

**nlohmann/json** (header-only library):
- **Pros**: Type-safe, modern C++, widely used
- **Cons**: Adds dependency, ~20K LOC header
- **Verdict**: Overkill for our simple needs

**RapidJSON**:
- **Pros**: Fast, complete JSON library
- **Cons**: Adds complexity, multiple source files
- **Verdict**: Overkill for our simple needs

**Source**: [JSON.org Specification](https://www.json.org/)

---

## Research Task 7: CMake Best Practices for Windows C++

### Key Findings

**1. Minimum CMake Version**: 3.20+
- Provides `cmake_path()` for path manipulation
- Better Windows SDK integration

**2. Compiler Flags**:
```cmake
if(MSVC)
    add_compile_options(/W4 /WX)  # Warnings as errors
    add_compile_options(/std:c++17)
    
    # Release optimizations
    add_compile_options("$<$<CONFIG:Release>:/O2>")
    add_compile_options("$<$<CONFIG:Release>:/GL>")  # Whole program optimization
    
    # Debug symbols
    add_compile_options("$<$<CONFIG:Debug>:/Zi>")
endif()
```

**3. Windows SDK Linking**:
```cmake
target_link_libraries(WinHKMonLib
    PRIVATE
        pdh        # Performance Data Helper
        iphlpapi   # IP Helper API
        powrprof   # Power management
)
```

**4. Testing Integration**:
```cmake
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(WinHKMonTests
    tests/CpuMonitorTest.cpp
    tests/MemoryMonitorTest.cpp
    # ... other tests
)

target_link_libraries(WinHKMonTests 
    PRIVATE 
        WinHKMonLib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(WinHKMonTests)
```

**Source**: 
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [Professional CMake Book](https://crascit.com/professional-cmake/)

---

## Research Task 8: Windows API Stability and Versioning

### Key Findings

All selected APIs are **stable and long-standing**:

| API | Introduced | Status | Notes |
|-----|-----------|--------|-------|
| GlobalMemoryStatusEx | Windows 2000 | Stable | Replacement for deprecated GlobalMemoryStatus |
| PDH (Performance Data Helper) | Windows 2000 | Stable | Core performance monitoring API |
| GetIfTable2 / MIB_IF_ROW2 | Windows Vista | Stable | Modern network API |
| CallNtPowerInformation | Windows 2000 | Stable | Power management API |
| QueryPerformanceCounter | Windows XP | Stable | High-resolution timing |

**Deprecation Risk**: **Very Low**
- All APIs are fundamental to Windows monitoring ecosystem
- Microsoft maintains backward compatibility for these APIs
- No deprecation notices in Windows SDK documentation (as of 2025)

**Windows 10+ Support**: All APIs fully supported on:
- Windows 10 21H2+
- Windows 11 (all versions)
- Windows Server 2022+

**Source**: [Windows API Index](https://docs.microsoft.com/en-us/windows/win32/api/)

---

## Summary

All research tasks complete. Key decisions:

1. ✅ **CPU Monitoring**: PDH API (with GetSystemTimes fallback)
2. ✅ **CPU Frequency**: CallNtPowerInformation
3. ✅ **Network**: IP Helper API (MIB_IF_ROW2)
4. ✅ **Temperature**: LibreHardwareMonitor via C++/CLI wrapper
5. ✅ **State File**: Text format in %TEMP%
6. ✅ **JSON Generation**: Manual (no external library)
7. ✅ **Build System**: CMake 3.20+ with best practices
8. ✅ **API Stability**: All selected APIs are stable, low deprecation risk

**Next Phase**: Proceed to Phase 1 (Design & Contracts)

---

**Document Control:**
- **Completed By**: WinHKMon Research Team
- **Review Date**: 2025-10-13
- **Status**: All decisions finalized, ready for implementation

