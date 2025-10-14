# WinHKMon - Windows Hardware Monitor

[![License](https://img.shields.io/badge/license-TBD-blue.svg)](LICENSE)
[![Windows](https://img.shields.io/badge/platform-Windows%2010%2B-blue.svg)](https://www.microsoft.com/windows)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](#build-status)
[![Tests](https://img.shields.io/badge/tests-57%2F57%20passing-brightgreen.svg)](#testing)
[![Status](https://img.shields.io/badge/status-foundation%20complete-green.svg)](#project-status)

A lightweight, native Windows system monitoring tool that provides real-time visibility into system resources (CPU, memory, disk, network, temperature) with minimal performance overhead and maximum accuracy.

**Perfect for:**
- 🖥️ System administrators managing remote Windows servers
- 👨‍💻 Software developers monitoring resource usage during testing
- ⚡ Power users tracking system performance and thermals
- 📊 Automated monitoring scripts and alerts

---

## ✨ Features

### Current Development Status

**Foundation Complete!** ✅ The project structure, testing framework, and core utilities are fully implemented and tested.

- ✅ **Project Setup** - CMake build system, directory structure, data models
- ✅ **CLI Parser** - Complete argument parsing with 31 tests passing
- ✅ **Output Formatter** - Text, JSON, CSV formats with 13 tests passing  
- ✅ **State Manager** - Delta calculation persistence with 9 tests passing
- ✅ **Build Verified** - MSVC 19.44, all 57 tests passing (100%), 0.86s execution time

**Next**: Implementing actual monitoring features (CPU, RAM, Disk, Network, Temperature)

### Planned Features (v1.0)

- **CPU Monitoring**
  - Overall and per-core usage percentages
  - Current CPU frequency per core
  - User/system/idle time breakdown
  
- **Memory Monitoring**
  - Physical RAM usage (total, available, used)
  - Page file statistics
  - Memory usage percentages

- **Disk I/O Monitoring**
  - Read/write transfer rates per physical disk
  - Cumulative bytes read/written
  - Disk busy percentage
  - Disk capacity information

- **Network Monitoring**
  - Send/receive rates per interface
  - Automatic primary interface selection
  - Link speed and connection status
  - Support for multiple network interfaces

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

> **Note**: Foundation is complete and tested. Monitoring features are in development. Currently the executable runs but returns placeholder output until monitors are implemented (Phase 3).

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
DISK: 0 C: ↑ 15.3 MB/s  ↓ 2.1 MB/s  (12.5% busy)
NET:  Ethernet ↑ 2.1 Mbps  ↓ 15.3 Mbps  (1 Gbps link)
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

# Build (verified working - all 57 tests pass!)
cmake --build . --config Release

# Run tests
ctest -C Release --output-on-failure
# Expected: 100% tests passed, 0 tests failed out of 57

# Run executable
.\Release\WinHKMon.exe --help
```

### Build Status

✅ **Verified on Windows 10/11 with MSVC 19.44**  
- Build: Successful (0 warnings with `/WX`)
- Tests: 57/57 passing (100%)
- Execution Time: 0.86 seconds
- Test Coverage: 100% of implemented components

### Alternative: Visual Studio IDE

1. Open Visual Studio 2022
2. **File → Open → CMake** → Select `CMakeLists.txt`
3. Wait for CMake configuration
4. **Build → Build All** (or Ctrl+Shift+B)
5. **Debug → Start Without Debugging** (or Ctrl+F5)

---

## 🧪 Testing

**Test Status**: ✅ All tests passing (100%)

### Test Suite Breakdown

| Test Suite | Tests | Status | Coverage |
|------------|-------|--------|----------|
| SampleTest | 3 | ✅ Passing | Framework validation |
| CliParserTest | 31 | ✅ Passing | Argument parsing, validation, all flags |
| OutputFormatterTest | 13 | ✅ Passing | Text, JSON, CSV formats, escaping |
| StateManagerTest | 9 | ✅ Passing | Save/load, corruption handling, paths |
| **Total** | **56** | **✅ 100%** | **All implemented features** |

**Aggregate Test**: 1 additional test runs all suites together  
**Execution Time**: 0.86 seconds (extremely fast)  
**Platform**: Windows 10/11, MSVC 19.44

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

---

## 🏗️ Project Status

**Current Phase**: Foundation Complete ✅ → Phase 3 Implementation Starting  
**Next Milestone**: v0.5 MVP (CPU + RAM monitoring)

### Development Roadmap

- [x] **Phase 0**: Research & Technical Decisions (Complete)
- [x] **Phase 1**: Specification (Complete)
- [x] **Phase 2**: Planning & Design (Complete)
- [x] **Phase 3**: Task Generation (Complete)
- [x] **Phase 4**: Foundation Implementation (Complete - CHECKPOINT 2 ✅)
  - [x] Project setup & CMake configuration
  - [x] Google Test integration
  - [x] Core data structures
  - [x] CLI argument parser (31 tests)
  - [x] Output formatter: text/JSON/CSV (13 tests)
  - [x] State manager for delta calculations (9 tests)
- [ ] **Phase 5**: Feature Implementation
  - [ ] **v0.5 (MVP)** - Basic monitoring (CPU + RAM) - *3 weeks*
  - [ ] **v0.8** - Comprehensive monitoring (+DISK, +NET) - *5 weeks*
  - [ ] **v1.0** - Full feature set (+TEMP, polish) - *9 weeks*

### Implementation Progress

| Feature Increment | Status | Tasks | Completion |
|-------------------|--------|-------|------------|
| Setup & Foundation | ✅ **Complete** | T001-T006 | **100%** (6/6) |
| US1: Basic Monitoring (MVP) | 🚧 Next | T007-T011 | 0% (0/5) |
| US2: Comprehensive Monitoring | 📋 Planned | T012-T015 | 0% (0/4) |
| US3: Thermal Monitoring | 📋 Planned | T016-T019 | 0% (0/4) |
| Polish & Cross-Cutting | 📋 Planned | T020-T024 | 0% (0/5) |
| Release | 📋 Planned | T025-T026 | 0% (0/2) |

**Total Progress**: 6 of 26 tasks complete (23%)  
**Foundation**: ✅ Complete (CHECKPOINT 2 passed)  
**Build Status**: ✅ 57/57 tests passing (100%)  
**Next**: T007 - Implement MemoryMonitor

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

Foundation complete! Ready for feature implementation:

- ✅ ~~T001: Initialize Project Structure~~ (Complete)
- ✅ ~~T002: Set Up Testing Framework~~ (Complete)
- ✅ ~~T003: Define Core Data Structures~~ (Complete)
- ✅ ~~T004: CLI Argument Parser~~ (Complete)
- ✅ ~~T005: Output Formatter~~ (Complete)
- ✅ ~~T006: State Manager~~ (Complete)
- [ ] **T007: Implement MemoryMonitor** ← Great starting point! Single API call, easiest monitor
- [ ] T008: Implement CpuMonitor (PDH API, moderate complexity)
- [ ] T012: Implement NetworkMonitor (IP Helper API)
- [ ] T013: Implement DiskMonitor (PDH counters)

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
  - LibreHardwareMonitor - Temperature (optional)

### System Requirements

**Runtime:**
- Windows 10 21H2+ or Windows 11
- x64 architecture (ARM64 best effort)
- Administrator privileges (only for temperature monitoring)

**Development:**
- Visual Studio 2022 with C++ workload
- CMake 3.20+
- Windows SDK 10.0.19041+

---

## 🔒 Privacy & Security

- **No telemetry** - Zero data collection
- **No network calls** - Fully offline operation
- **User-specific state** - State files secured to current user
- **Open source** - Full transparency

---

## 📝 License

[License TBD - To be added]

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
- **Code Written**: ~2,000 lines (7 headers + 4 implementations)
- **Tests Written**: 56 test cases across 4 test suites
- **Test Results**: 57/57 passing (100%) - 0.86s execution time
- **Test Coverage**: 100% of implemented components
- **Build Status**: ✅ Passing (MSVC 19.44, 0 warnings with `/WX`)
- **Tasks Complete**: 6 of 26 (23%)
- **Checkpoints Passed**: 2 of 7 (CHECKPOINT 1 ✅, CHECKPOINT 2 ✅)

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

**Status**: ✅ Foundation complete (57/57 tests passing), ready for feature implementation!

---

<sub>Project follows semantic versioning. Current target: v1.0.0</sub>

