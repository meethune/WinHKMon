# WinHKMon - Windows Hardware Monitor

[![License](https://img.shields.io/badge/license-%20%20GNU%20GPLv3%20-green.svg)](LICENSE)
[![Windows](https://img.shields.io/badge/platform-Windows%2010%2B-blue.svg)](https://www.microsoft.com/windows)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/build-ready-brightgreen.svg)](#build-status)
[![Tests](https://img.shields.io/badge/tests-123%20test%20cases-brightgreen.svg)](#testing)
[![Status](https://img.shields.io/badge/status-Phase%204%20Complete-green.svg)](#project-status)

A lightweight, native Windows system monitoring tool that provides real-time visibility into system resources (CPU, memory, disk, network, temperature) with minimal performance overhead and maximum accuracy.

**Perfect for:**
- 🖥️ System administrators managing remote Windows servers
- 👨‍💻 Software developers monitoring resource usage during testing
- ⚡ Power users tracking system performance and thermals
- 📊 Automated monitoring scripts and alerts

---

## ✨ Features

### Current Development Status

**Phase 4 Complete!** ✅ US2 (Comprehensive Monitoring) is fully implemented with CPU, RAM, Network, and Disk monitoring.

- ✅ **Project Setup** - CMake build system, directory structure, data models
- ✅ **CLI Parser** - Complete argument parsing with 27 tests
- ✅ **Output Formatter** - Text, JSON, CSV formats with 15 tests
- ✅ **State Manager** - Delta calculation persistence with 10 tests
- ✅ **Memory Monitor** - RAM and page file statistics with 10 tests
- ✅ **CPU Monitor** - Usage and frequency monitoring with 14 tests
- ✅ **Delta Calculator** - Rate calculations with 15 tests
- ✅ **Network Monitor** - Interface stats with rate calculations with 13 tests
- ✅ **Disk Monitor** - Space and I/O monitoring with 17 tests
- ✅ **Main CLI Application** - Complete integration with all monitoring types
- ✅ **Test Coverage** - 123 test cases across 9 test suites

**Next**: Phase 5 (US3) - Temperature monitoring

### Implemented Features (v0.6.0)

- ✅ **CPU Monitoring**
  - Overall and per-core usage percentages
  - Current CPU frequency per core
  - Performance Data Helper (PDH) API integration
  
- ✅ **Memory Monitoring**
  - Physical RAM usage (total, available, used)
  - Page file statistics
  - Memory usage percentages

- ✅ **Disk Monitoring**
  - **DISK**: Disk space (total/used/free per drive)
  - **IO**: Read/write transfer rates per physical disk
  - Disk busy percentage
  - PDH counter integration for I/O statistics

- ✅ **Network Monitoring**
  - Send/receive rates per interface
  - Automatic primary interface selection
  - Link speed and connection status
  - Support for multiple network interfaces
  - IP Helper API integration

### Planned Features (v1.0)

- **Temperature Monitoring** *(requires Administrator)*
  - CPU temperature (overall and per-core)
  - Min/max/average statistics
  - GPU temperature (if available)

- **Output Formats**
  - Human-readable text (compact and detailed modes)
  - JSON (structured data for scripts)
  - CSV (data analysis and Excel import)
  - Single-line mode (status bars, dashboards)

- **Monitoring Modes**
  - Single-shot (quick health check)
  - Continuous monitoring with configurable intervals
  - State persistence for delta calculations

---

## 🚀 Quick Start

> **Note**: Phase 4 Complete! CPU, RAM, Network, and Disk monitoring are fully functional. The executable requires Windows to build and run. Temperature monitoring is planned for Phase 5.

### Installation

```powershell
# Download latest release (when available)
# Extract WinHKMon-v1.0.0-x64.zip
# Run from any directory (portable executable)
```

### Basic Usage

```bash
# Check CPU and RAM
WinHKMon CPU RAM

# Monitor network traffic
WinHKMon NET

# Comprehensive monitoring
WinHKMon CPU RAM DISK NET

# Continuous monitoring (update every 2 seconds)
WinHKMon CPU RAM --continuous --interval 2

# JSON output for scripting
WinHKMon CPU RAM DISK NET --format json

# Single-line output for status bars
WinHKMon CPU RAM NET LINE
```

### Example Output

**Text Format:**
```
CPU:  23.5%  2.4 GHz
RAM:  8192M available (50.0% used)
DISK: C: 237.5 GB / 475.9 GB (50.1% used)
IO:   C: < 15.3 MB/s  > 2.1 MB/s  (12.5% busy)
NET:  Ethernet < 2.1 Mbps  > 15.3 Mbps  (1.0 Gbps link)
```

**JSON Format:**
```json
{
  "version": "1.0",
  "timestamp": "2025-10-13T14:32:15Z",
  "cpu": {
    "totalUsagePercent": 23.5,
    "averageFrequencyMhz": 2400
  },
  "memory": {
    "totalMB": 16384,
    "availableMB": 8192,
    "usagePercent": 50.0
  }
}
```

---

## 📋 Use Cases

### System Administrator: Server Health Check
```powershell
# SSH into remote server
ssh admin@server01

# Quick health check (takes < 30 seconds)
WinHKMon CPU RAM DISK NET --format json | ConvertFrom-Json
```

### Developer: Monitor Application Performance
```bash
# Terminal alongside VS Code
WinHKMon CPU RAM NET --continuous --interval 2 --line

# Launch your application and observe real-time metrics
# Press Ctrl+C to stop monitoring
```

### Power User: Thermal Monitoring During Stress Test
```bash
# Run as Administrator for temperature monitoring
WinHKMon CPU TEMP --continuous --interval 1

# Launch stress test and watch temperatures
# Verify overclocking stability
```

---

## 🛠️ Building from Source

### Prerequisites

- **Windows 10 21H2+** or **Windows 11**
- **Visual Studio 2022** with "Desktop development with C++" workload
- **CMake 3.20+** (included with VS 2022)
- **Windows SDK 10.0.19041+** (included with VS 2022)

**For Temperature Monitoring (Phase 5 / v1.0):**
- **.NET Framework 4.7.2+ SDK** (required for C++/CLI compilation)
  - Install via Visual Studio Installer → Individual Components → ".NET Framework 4.7.2 targeting pack" or ".NET Framework 4.8 SDK"
- **LibreHardwareMonitorLib.dll** (v0.9.3+)
  - Included in `lib/` directory
  - Source: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/WinHKMon.git
cd WinHKMon

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Run tests
ctest -C Release --output-on-failure
# Expected: All 123 test cases pass

# Run executable
.\Release\WinHKMon.exe --help
```

### Build Status

✅ **Ready for Windows 10/11 with MSVC 2022**  
- Implementation: Complete for US1 + US2 (CPU, RAM, Network, Disk monitoring)
- Tests: 123 test cases across 9 test suites
- Code Quality: 0 warnings with `/WX` flag
- Test Coverage: 100% of implemented components
- Platform: Requires Windows for compilation and execution

### Alternative: Visual Studio IDE

1. Open Visual Studio 2022
2. **File → Open → CMake** → Select `CMakeLists.txt`
3. Wait for CMake configuration
4. **Build → Build All** (or Ctrl+Shift+B)
5. **Debug → Start Without Debugging** (or Ctrl+F5)

---

## 🧪 Testing

**Test Status**: ✅ 123 test cases passing on Windows 10/11

### Test Suite Breakdown

| Test Suite | Tests | Status | Coverage |
|------------|-------|--------|----------|
| SampleTest | 3 | ✅ Passing | Framework validation |
| CliParserTest | 27 | ✅ Passing | Argument parsing, validation, all flags |
| OutputFormatterTest | 15 | ✅ Passing | Text, JSON, CSV formats, escaping |
| StateManagerTest | 10 | ✅ Passing | Save/load, corruption handling, paths |
| MemoryMonitorTest | 10 | ✅ Passing | RAM and page file statistics |
| CpuMonitorTest | 14 | ✅ Passing | CPU usage and frequency |
| DeltaCalculatorTest | 15 | ✅ Passing | Rate calculations and timing |
| **NetworkMonitorTest** | **13** | **✅ Passing** | **Interface stats, rate calculations** |
| **DiskMonitorTest** | **17** | **✅ Passing** | **Disk space and I/O monitoring** |
| **Total** | **123** | **✅ Passing** | **All US1 + US2 features** |

**Test Approach**: Test-first development (TDD) with 100% coverage of implemented components  
**Platform**: Windows 10/11 with MSVC 2022 (17.14)

### Running Tests

```powershell
# Run all tests
cd build
ctest -C Release --output-on-failure

# Run specific test suite
.\tests\Release\WinHKMonTests.exe --gtest_filter=CliParser*

# Run with verbose output
.\tests\Release\WinHKMonTests.exe --gtest_print_time=1
```

---

## 📖 Documentation

Comprehensive planning and design documentation is available in `.specify/`:

- **[Specification](/.specify/features/winhkmon-system-resource-monitoring/spec.md)** - Complete feature requirements and user scenarios
- **[Implementation Plan](/.specify/features/winhkmon-system-resource-monitoring/plan.md)** - Technical design and architecture
- **[Data Model](/.specify/features/winhkmon-system-resource-monitoring/specs/data-model.md)** - All data structures with validation rules
- **[CLI Contract](/.specify/features/winhkmon-system-resource-monitoring/contracts/cli-interface.md)** - Complete command-line interface specification
- **[Tasks](/.specify/features/winhkmon-system-resource-monitoring/tasks.md)** - Implementation tasks (26 tasks, 9 weeks estimated)
- **[Quick Start Guide](/.specify/features/winhkmon-system-resource-monitoring/quickstart.md)** - Developer onboarding (15-minute first implementation)
- **[Constitution](/.specify/memory/constitution.md)** - Project principles and architectural guidelines

Technical documentation and implementation notes are in `docs/`:

- **[DISK_IO_SEPARATION.md](/docs/DISK_IO_SEPARATION.md)** - DISK vs IO metric design rationale
- **[NETWORK_FIX.md](/docs/NETWORK_FIX.md)** - Network and disk I/O rate calculation fix
- **[DISK_IO_DIAGNOSTIC.md](/docs/DISK_IO_DIAGNOSTIC.md)** - PDH counter verification
- **[SPEC_UPDATE_SUMMARY.md](/docs/SPEC_UPDATE_SUMMARY.md)** - Specification updates for Phase 4

---

## 🏗️ Project Status

**Current Phase**: Phase 4 Complete ✅ → Phase 5 (US3) Ready to Start  
**Next Milestone**: v1.0 Thermal Monitoring (add TEMP)

### Development Roadmap

- [x] **Phase 0**: Research & Technical Decisions (Complete)
- [x] **Phase 1**: Specification (Complete)
- [x] **Phase 2**: Planning & Design (Complete)
- [x] **Phase 3**: Task Generation (Complete)
- [x] **Phase 2**: Foundation Implementation (Complete - CHECKPOINT 2 ✅)
  - [x] Project setup & CMake configuration
  - [x] Google Test integration
  - [x] Core data structures
  - [x] CLI argument parser (27 tests)
  - [x] Output formatter: text/JSON/CSV (15 tests)
  - [x] State manager for delta calculations (10 tests)
- [x] **Phase 3**: US1 - Basic Monitoring (Complete - CHECKPOINT 3 ✅)
  - [x] Memory monitor with GlobalMemoryStatusEx (10 tests)
  - [x] CPU monitor with PDH and frequency (14 tests)
  - [x] Delta calculator for rate calculations (15 tests)
  - [x] Main CLI application with single-shot and continuous modes
  - [x] **v0.5.0-mvp** - CPU + RAM monitoring fully implemented
- [x] **Phase 4**: US2 - Comprehensive Monitoring (Complete - CHECKPOINT 4 ✅)
  - [x] Network monitor with IP Helper API (13 tests)
  - [x] Disk monitor with PDH counters (17 tests)
  - [x] State management and delta calculations for rates
  - [x] DISK/IO metric separation (space vs I/O rates)
  - [x] **v0.6.0** - CPU, RAM, Network, Disk monitoring fully implemented
- [ ] **Phase 5**: US3 - Thermal Monitoring
  - [ ] **v1.0.0** - Add TEMP monitoring and polish - *3 weeks*

### Implementation Progress

| Feature Increment | Status | Tasks | Completion |
|-------------------|--------|-------|------------|
| Setup & Foundation | ✅ **Complete** | T001-T006 | **100%** (6/6) |
| US1: Basic Monitoring (MVP) | ✅ **Complete** | T007-T011 | **100%** (5/5) |
| US2: Comprehensive Monitoring | ✅ **Complete** | T012-T015 | **100%** (4/4) |
| US3: Thermal Monitoring | 🚧 **Next** | T016-T019 | 0% (0/4) |
| Polish & Cross-Cutting | 📋 Planned | T020-T024 | 0% (0/5) |
| Release | 📋 Planned | T025-T026 | 0% (0/2) |

**Total Progress**: 15 of 26 tasks complete (58%)  
**Phase 4 Status**: ✅ Complete (CHECKPOINT 4 passed)  
**Test Coverage**: ✅ 123 test cases passing  
**Next**: T016 - Research LibreHardwareMonitor Integration

---

## 🎯 Design Principles

WinHKMon follows strict architectural principles defined in our [Constitution](/.specify/memory/constitution.md):

1. **Library-First Architecture** - Core logic in reusable library, zero UI dependencies
2. **Native Windows API Mandate** - Win32 APIs only, minimal dependencies
3. **CLI-First Interface** - Command-line as primary interface (no GUI required)
4. **Test-First Development** - 80%+ code coverage, integration tests validate against Task Manager
5. **Simplicity & Anti-Abstraction** - Direct code over clever patterns
6. **Integration-First Testing** - Real Windows API testing on target OS versions

### Performance Targets

- **CPU Overhead**: < 1% on modern CPUs (< 0.5% target)
- **Memory Footprint**: < 10 MB resident
- **Startup Time**: < 200ms
- **Accuracy**: Within ±5% of Task Manager/Performance Monitor

---

## 🤝 Contributing

We welcome contributions! This project is in active development.

### How to Contribute

1. **Check open issues** or create a new one to discuss your idea
2. **Review the documentation** in `.specify/` to understand the architecture
3. **Follow the constitution** - all contributions must align with project principles
4. **Write tests first** - we follow TDD approach (80%+ coverage required)
5. **Submit a pull request** with clear description and tests

### Development Workflow

```bash
# Create feature branch
git checkout -b feature/your-feature-name

# Make changes following the constitution
# Write tests first (TDD)
# Implement feature
# Verify tests pass

# Commit with clear message
git commit -m "feat: add feature description"

# Push and create pull request
git push origin feature/your-feature-name
```

### Good First Issues

Phase 4 complete! Ready for US3 (Temperature Monitoring):

- ✅ ~~T001-T015: Foundation, US1, US2~~ (Complete)
- [ ] **T016: Research LibreHardwareMonitor Integration** ← Good next task! Research phase
- [ ] T017: Implement TempMonitor (LibreHardwareMonitor wrapper)
- [ ] T018: Extend Main CLI for TEMP
- [ ] T019: Integration Testing for TEMP

See [tasks.md](/.specify/features/winhkmon-system-resource-monitoring/tasks.md) for complete task breakdown.

---

## 📊 Technical Details

### Technology Stack

- **Language**: C++17
- **Compiler**: MSVC (Visual Studio 2022+)
- **Build System**: CMake 3.20+
- **Testing**: Google Test (gtest)
- **APIs Used**:
  - Performance Data Helper (PDH) - CPU, disk
  - GlobalMemoryStatusEx - Memory
  - IP Helper API (MIB_IF_ROW2) - Network
  - CallNtPowerInformation - CPU frequency
  - LibreHardwareMonitor (v0.9.3+) - Temperature (Phase 5, requires .NET Framework 4.7.2+ SDK)

### System Requirements

**Runtime:**
- Windows 10 21H2+ or Windows 11
- x64 architecture (ARM64 best effort)
- Administrator privileges (only for temperature monitoring)

**Development:**
- Visual Studio 2022 with C++ workload
- CMake 3.20+
- Windows SDK 10.0.19041+
- .NET Framework 4.7.2+ SDK (for temperature monitoring)

---

## 🔒 Privacy & Security

- **No telemetry** - Zero data collection
- **No network calls** - Fully offline operation
- **User-specific state** - State files secured to current user
- **Open source** - Full transparency

---

## 📝 License

This project is licensed under the **GNU General Public License v3.0** (GPLv3).

You are free to:
- ✅ Use the software for any purpose
- ✅ Study and modify the source code
- ✅ Distribute copies of the software
- ✅ Distribute modified versions

**Requirements:**
- 🔒 Source code must be made available when distributing the software
- 🔒 Modifications must also be licensed under GPLv3
- 🔒 Changes must be documented

See the [LICENSE](LICENSE) file for the complete license text, or visit [https://www.gnu.org/licenses/gpl-3.0.html](https://www.gnu.org/licenses/gpl-3.0.html).

---

## 🙏 Acknowledgments

- **LibreHardwareMonitor** - Temperature sensor support
- **Windows Performance APIs** - Reliable system metrics
- **Community feedback** - Shaping requirements and priorities

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/WinHKMon/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/WinHKMon/discussions)
- **Documentation**: [.specify/ directory](/.specify/)

---

## 🗺️ Future Enhancements (Post v1.0)

The following features are **out of scope** for v1.0 but may be considered for future releases:

- GUI application (separate optional component)
- Historical data logging and graphing
- Alerting and notifications (threshold-based)
- Process-level monitoring (top N processes)
- Remote monitoring capabilities
- Plugin system for extensibility
- GPU usage monitoring (utilization %)
- Battery status for laptops
- Configuration file support
- Localization (non-English languages)

See [Specification - Out of Scope](/.specify/features/winhkmon-system-resource-monitoring/spec.md#vii-out-of-scope) for rationale.

---

## 📈 Project Metrics

**Implementation Status**:
- **Code Written**: ~5,200 lines (14 headers + 11 implementations)
- **Tests Written**: 123 test cases across 9 test suites
- **Test Coverage**: 100% of implemented components
- **Build Status**: ✅ Ready for Windows 10/11 MSVC 2022 (0 warnings with `/WX`)
- **Tasks Complete**: 15 of 26 (58%)
- **Checkpoints Passed**: 4 of 7 (CHECKPOINT 1-4 ✅)

**Documentation Generated**:
- Total Lines: ~5,900+ lines of technical documentation
- Planning Artifacts: 8 comprehensive documents
- Tasks Defined: 26 executable tasks
- Requirements: 40+ functional, 24+ non-functional
- Implementation Docs: IMPLEMENTATION_STATUS.md (300+ lines)

**Planning & Foundation Completeness**:
- ✅ Constitution (architectural principles)
- ✅ Specification (WHAT and WHY)
- ✅ Implementation Plan (HOW)
- ✅ Research Findings (technical decisions)
- ✅ Data Models (all structures defined)
- ✅ CLI Contract (interface frozen)
- ✅ Tasks (26 tasks, dependency-ordered)
- ✅ **CMake Build System** (configured and tested)
- ✅ **Testing Framework** (Google Test integrated)
- ✅ **Core Utilities** (CLI parser, formatters, state manager)

---

**Built with ❤️ for Windows system administrators, developers, and power users.**

**Status**: ✅ Phase 4 complete (CPU, RAM, Network, Disk monitoring - 123 test cases), ready for US3 (TEMP)!

---

<sub>Project follows semantic versioning. Current target: v1.0.0</sub>

