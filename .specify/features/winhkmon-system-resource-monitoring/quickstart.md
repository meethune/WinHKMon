# WinHKMon Quick Start Guide

**Target Audience**: Developers implementing WinHKMon
**Last Updated**: 2025-10-13
**Prerequisites**: Windows 10/11, Visual Studio 2022, CMake 3.20+

This guide gets you from zero to running code in under 30 minutes.

---

## Setup (5 minutes)

### 1. Clone and Branch
```bash
git clone https://github.com/yourorg/WinHKMon.git
cd WinHKMon
git checkout feature/winhkmon-system-resource-monitoring
```

### 2. Install Prerequisites
- **Visual Studio 2022** with "Desktop development with C++" workload
- **CMake 3.20+**: Included with VS 2022 or install standalone
- **Windows SDK 10.0.19041+**: Included with VS 2022

### 3. Verify Environment
```cmd
cmake --version    REM Should be 3.20+
cl.exe            REM Should invoke MSVC compiler
```

---

## Project Structure

```
WinHKMon/
├── .specify/                       # Specification and planning
│   └── features/
│       └── winhkmon-system-resource-monitoring/
│           ├── spec.md            # Feature specification
│           ├── plan.md            # Implementation plan
│           ├── research.md        # Research findings
│           ├── specs/
│           │   └── data-model.md  # Data structures
│           └── contracts/
│               └── cli-interface.md # CLI contract
├── src/                           # Source files
│   ├── WinHKMonLib/              # Core library
│   │   ├── CpuMonitor.cpp
│   │   ├── MemoryMonitor.cpp
│   │   ├── DiskMonitor.cpp
│   │   ├── NetworkMonitor.cpp
│   │   ├── TempMonitor.cpp       # Temperature (C++/CLI)
│   │   ├── StateManager.cpp
│   │   └── OutputFormatter.cpp
│   └── WinHKMon/                 # CLI executable
│       └── main.cpp
├── include/                       # Public headers
│   └── WinHKMonLib/
│       ├── CpuMonitor.h
│       ├── MemoryMonitor.h
│       └── ...
├── tests/                         # Unit tests
│   ├── CpuMonitorTest.cpp
│   ├── MemoryMonitorTest.cpp
│   └── ...
├── CMakeLists.txt                 # Build configuration
└── README.md                      # User documentation
```

---

## Build Process (10 minutes)

### Option A: CMake Command Line

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Run
.\Release\WinHKMon.exe --help
```

### Option B: Visual Studio IDE

1. Open Visual Studio 2022
2. **File → Open → CMake** → Select `CMakeLists.txt`
3. Wait for CMake configuration
4. **Build → Build All** (or Ctrl+Shift+B)
5. **Debug → Start Without Debugging** (or Ctrl+F5)

### Option C: Command Line (MSVC)

```cmd
REM Set up MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Build library
cd src\WinHKMonLib
cl /c /std:c++17 /W4 /EHsc *.cpp /I..\..\include
lib /OUT:WinHKMonLib.lib *.obj

REM Build executable
cd ..\WinHKMon
cl /std:c++17 /W4 /EHsc main.cpp /I..\..\include /link ..\WinHKMonLib\WinHKMonLib.lib pdh.lib iphlpapi.lib powrprof.lib
```

---

## First Implementation: MemoryMonitor (15 minutes)

Start with the simplest component to understand the pattern.

### 1. Create Header (`include/WinHKMonLib/MemoryMonitor.h`)

```cpp
#pragma once
#include <cstdint>

namespace WinHKMon {

struct MemoryStats {
    uint64_t totalPhysicalBytes;
    uint64_t availablePhysicalBytes;
    uint64_t usedPhysicalBytes;
    double usagePercent;

    uint64_t totalPageFileBytes;
    uint64_t availablePageFileBytes;
    uint64_t usedPageFileBytes;
    double pageFilePercent;
};

class MemoryMonitor {
public:
    MemoryStats getCurrentStats();
};

}  // namespace WinHKMon
```

### 2. Implement (`src/WinHKMonLib/MemoryMonitor.cpp`)

```cpp
#include "WinHKMonLib/MemoryMonitor.h"
#include <windows.h>
#include <stdexcept>

namespace WinHKMon {

MemoryStats MemoryMonitor::getCurrentStats() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        throw std::runtime_error("GlobalMemoryStatusEx failed");
    }

    MemoryStats stats;
    stats.totalPhysicalBytes = memStatus.ullTotalPhys;
    stats.availablePhysicalBytes = memStatus.ullAvailPhys;
    stats.usedPhysicalBytes = stats.totalPhysicalBytes - stats.availablePhysicalBytes;
    stats.usagePercent = (static_cast<double>(stats.usedPhysicalBytes) / 
                         stats.totalPhysicalBytes) * 100.0;

    stats.totalPageFileBytes = memStatus.ullTotalPageFile;
    stats.availablePageFileBytes = memStatus.ullAvailPageFile;
    stats.usedPageFileBytes = stats.totalPageFileBytes - stats.availablePageFileBytes;
    stats.pageFilePercent = stats.totalPageFileBytes > 0 ?
                           (static_cast<double>(stats.usedPageFileBytes) / 
                            stats.totalPageFileBytes) * 100.0 : 0.0;

    return stats;
}

}  // namespace WinHKMon
```

### 3. Test (`tests/MemoryMonitorTest.cpp`)

```cpp
#include "WinHKMonLib/MemoryMonitor.h"
#include <gtest/gtest.h>

using namespace WinHKMon;

TEST(MemoryMonitorTest, ReturnsValidStats) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    // Basic sanity checks
    EXPECT_GT(stats.totalPhysicalBytes, 0);
    EXPECT_LE(stats.availablePhysicalBytes, stats.totalPhysicalBytes);
    EXPECT_EQ(stats.usedPhysicalBytes, 
              stats.totalPhysicalBytes - stats.availablePhysicalBytes);
}

TEST(MemoryMonitorTest, PercentageInValidRange) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    EXPECT_GE(stats.usagePercent, 0.0);
    EXPECT_LE(stats.usagePercent, 100.0);
    EXPECT_GE(stats.pageFilePercent, 0.0);
    EXPECT_LE(stats.pageFilePercent, 100.0);
}

TEST(MemoryMonitorTest, CalculatesUsedMemoryCorrectly) {
    MemoryMonitor monitor;
    MemoryStats stats = monitor.getCurrentStats();

    double expectedPercent = (static_cast<double>(stats.usedPhysicalBytes) / 
                             stats.totalPhysicalBytes) * 100.0;
    EXPECT_NEAR(stats.usagePercent, expectedPercent, 0.01);
}
```

### 4. Run Tests

```bash
cd build
ctest --output-on-failure
```

---

## Minimal CLI (10 minutes)

### 1. Create `src/WinHKMon/main.cpp`

```cpp
#include "WinHKMonLib/MemoryMonitor.h"
#include <iostream>
#include <iomanip>

using namespace WinHKMon;

int main(int argc, char* argv[]) {
    try {
        MemoryMonitor monitor;
        MemoryStats stats = monitor.getCurrentStats();

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "RAM:  " << (stats.availablePhysicalBytes / 1024 / 1024) 
                  << " MB available (" << stats.usagePercent << "% used)\n";
        std::cout << "PAGE: " << (stats.usedPageFileBytes / 1024 / 1024) 
                  << " MB used (" << stats.pageFilePercent << "%)\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 2;
    }
}
```

### 2. Build and Run

```bash
cmake --build . --config Release
.\Release\WinHKMon.exe
```

**Expected Output**:
```
RAM:  8192 MB available (50.0% used)
PAGE: 2048 MB used (25.0%)
```

---

## Development Workflow

### Typical Development Cycle

1. **Read Specification**: Review `spec.md` for requirements
2. **Check Plan**: Consult `plan.md` for technical approach
3. **Review Data Model**: Check `specs/data-model.md` for structure
4. **Write Test First**: Create test in `tests/`
5. **Implement**: Write code in `src/WinHKMonLib/`
6. **Run Tests**: `ctest --output-on-failure`
7. **Integration Test**: Run CLI manually
8. **Compare with Task Manager**: Verify accuracy
9. **Commit**: Small, focused commits

### Build Targets

```bash
# Build everything
cmake --build . --config Release

# Build only library
cmake --build . --config Release --target WinHKMonLib

# Build only tests
cmake --build . --config Release --target WinHKMonTests

# Clean build
cmake --build . --config Release --target clean
```

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test
ctest -R MemoryMonitorTest --verbose

# Run tests with Google Test filters
.\tests\Release\WinHKMonTests.exe --gtest_filter=MemoryMonitor*
```

---

## Debugging Tips

### Visual Studio Debugger

1. Set breakpoint in code (F9)
2. **Debug → Start Debugging** (F5)
3. Inspect variables in **Autos** / **Locals** window
4. Use **Watch** window for specific expressions

### Command-Line Debugging

```bash
# Run with debugger attached
devenv .\WinHKMon.exe /debugexe

# Or use WinDbg
windbg .\WinHKMon.exe
```

### Common Issues

**Issue**: "PDH.DLL not found"
- **Fix**: Link against `pdh.lib` in CMake: `target_link_libraries(WinHKMonLib pdh)`

**Issue**: "GlobalMemoryStatusEx undefined"
- **Fix**: Include `<windows.h>` and ensure Windows SDK is installed

**Issue**: Tests fail with "Access Denied"
- **Fix**: Run Visual Studio as Administrator (required for temperature sensors)

**Issue**: CMake can't find Visual Studio
- **Fix**: Use `-G "Visual Studio 17 2022"` explicitly

---

## Implementation Order (Recommended)

Follow this order to minimize dependencies:

1. ✅ **MemoryMonitor** (simplest, single API call)
2. **OutputFormatter** (needed for CLI output)
3. **CpuMonitor** (moderate complexity, PDH)
4. **NetworkMonitor** (moderate, IP Helper API)
5. **StateManager** (file I/O, needed for deltas)
6. **DiskMonitor** (uses StateManager)
7. **TempMonitor** (most complex, C++/CLI)
8. **CLI Argument Parsing**
9. **Integration Tests**
10. **Documentation**

---

## Key Windows APIs Reference

### Memory Monitoring
```cpp
#include <windows.h>
MEMORYSTATUSEX memStatus;
memStatus.dwLength = sizeof(MEMORYSTATUSEX);
GlobalMemoryStatusEx(&memStatus);
// memStatus.ullTotalPhys, memStatus.ullAvailPhys, etc.
```

### CPU Monitoring (PDH)
```cpp
#include <pdh.h>
PDH_HQUERY hQuery;
PDH_HCOUNTER hCounter;
PdhOpenQuery(NULL, 0, &hQuery);
PdhAddEnglishCounter(hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &hCounter);
PdhCollectQueryData(hQuery);
Sleep(1000);
PdhCollectQueryData(hQuery);
PDH_FMT_COUNTERVALUE counterValue;
PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &counterValue);
// counterValue.doubleValue contains CPU percentage
```

### Network Monitoring
```cpp
#include <netioapi.h>
PMIB_IF_TABLE2 pIfTable = NULL;
GetIfTable2(&pIfTable);
for (ULONG i = 0; i < pIfTable->NumEntries; i++) {
    MIB_IF_ROW2* pIfRow = &pIfTable->Table[i];
    // pIfRow->InOctets, pIfRow->OutOctets
}
FreeMibTable(pIfTable);
```

### Monotonic Timestamps
```cpp
LARGE_INTEGER frequency, counter;
QueryPerformanceFrequency(&frequency);
QueryPerformanceCounter(&counter);
uint64_t timestamp = counter.QuadPart;
```

---

## Performance Profiling

### Measure Component Overhead

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
MemoryStats stats = monitor.getCurrentStats();
auto end = std::chrono::high_resolution_clock::now();

auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Execution time: " << duration.count() << " μs\n";
```

**Target**: < 50ms per component, < 200ms total startup

---

## Documentation Standards

### Code Comments

```cpp
/**
 * @brief Collects current memory usage statistics
 *
 * Calls GlobalMemoryStatusEx to retrieve physical and page file memory data.
 * Calculates usage percentages and byte values.
 *
 * @return MemoryStats structure with all memory metrics
 * @throws std::runtime_error if GlobalMemoryStatusEx fails
 *
 * @note This function is thread-safe
 * @see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
 */
MemoryStats getCurrentStats();
```

### Commit Messages

```
feat(memory): implement MemoryMonitor with GlobalMemoryStatusEx

- Add MemoryMonitor class with getCurrentStats()
- Implement MemoryStats structure matching data model
- Add unit tests with 100% coverage
- Validate against Task Manager (within 1% accuracy)

Closes #12
```

---

## Next Steps

After completing the quickstart:

1. **Read Full Documentation**:
   - [Specification](spec.md) - Complete requirements
   - [Implementation Plan](plan.md) - Technical design
   - [Data Model](specs/data-model.md) - All data structures
   - [CLI Contract](contracts/cli-interface.md) - Interface specification

2. **Implement Remaining Components**: Follow recommended order above

3. **Run Integration Tests**: Compare with Task Manager / Performance Monitor

4. **Optimize Performance**: Profile and optimize to meet < 1% CPU overhead target

5. **Write User Documentation**: README.md with usage examples

---

## Getting Help

**Documentation**:
- Project specification: `.specify/features/winhkmon-system-resource-monitoring/spec.md`
- Implementation plan: `.specify/features/winhkmon-system-resource-monitoring/plan.md`
- Constitution (principles): `.specify/memory/constitution.md`

**Windows API Documentation**:
- [Windows SDK API Index](https://docs.microsoft.com/en-us/windows/win32/api/)
- [PDH Reference](https://docs.microsoft.com/en-us/windows/win32/perfctrs/using-the-pdh-functions)

**Build Issues**:
- Check CMake version: `cmake --version` (need 3.20+)
- Verify MSVC is in PATH: `cl.exe`
- Ensure Windows SDK installed via Visual Studio Installer

---

**Document Control:**
- **Author**: WinHKMon Development Team
- **Last Updated**: 2025-10-13
- **Status**: Complete
- **Feedback**: Report issues or improvements to the team

