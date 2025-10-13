# WinHKMon - Windows Hardware Monitor

[![License](https://img.shields.io/badge/license-TBD-blue.svg)](LICENSE)
[![Windows](https://img.shields.io/badge/platform-Windows%2010%2B-blue.svg)](https://www.microsoft.com/windows)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/status-in%20development-yellow.svg)](#project-status)

A lightweight, native Windows system monitoring tool that provides real-time visibility into system resources (CPU, memory, disk, network, temperature) with minimal performance overhead and maximum accuracy.

**Perfect for:**
- 🖥️ System administrators managing remote Windows servers
- 👨‍💻 Software developers monitoring resource usage during testing
- ⚡ Power users tracking system performance and thermals
- 📊 Automated monitoring scripts and alerts

---

## ✨ Features

### Current Development Status

This project is currently in the **planning and design phase**. We have comprehensive specifications and implementation plans ready. See [Project Status](#project-status) below.

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

> **Note**: WinHKMon is currently in development. The following examples show planned functionality.

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

# Build
cmake --build . --config Release

# Run
.\Release\WinHKMon.exe --help
```

### Alternative: Visual Studio IDE

1. Open Visual Studio 2022
2. **File → Open → CMake** → Select `CMakeLists.txt`
3. Wait for CMake configuration
4. **Build → Build All** (or Ctrl+Shift+B)
5. **Debug → Start Without Debugging** (or Ctrl+F5)

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

**Current Phase**: Planning & Design Complete ✅  
**Next Phase**: Implementation (Starting with MVP)

### Development Roadmap

- [x] **Phase 0**: Research & Technical Decisions (Complete)
- [x] **Phase 1**: Specification (Complete)
- [x] **Phase 2**: Planning & Design (Complete)
- [x] **Phase 3**: Task Generation (Complete)
- [ ] **Phase 4**: Implementation
  - [ ] **v0.5 (MVP)** - Basic monitoring (CPU + RAM) - *3 weeks*
  - [ ] **v0.8** - Comprehensive monitoring (+DISK, +NET) - *5 weeks*
  - [ ] **v1.0** - Full feature set (+TEMP, polish) - *9 weeks*

### Implementation Progress

| Feature Increment | Status | Tasks | Completion |
|-------------------|--------|-------|------------|
| Setup & Foundation | 📋 Planned | T001-T006 | 0% |
| US1: Basic Monitoring (MVP) | 📋 Planned | T007-T011 | 0% |
| US2: Comprehensive Monitoring | 📋 Planned | T012-T015 | 0% |
| US3: Thermal Monitoring | 📋 Planned | T016-T019 | 0% |
| Polish & Cross-Cutting | 📋 Planned | T020-T024 | 0% |
| Release | 📋 Planned | T025-T026 | 0% |

**Total Tasks**: 26 tasks across 7 phases  
**Estimated Duration**: 9 weeks (with parallelization)  
**MVP Duration**: 3 weeks (US1: CPU + RAM monitoring)

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

- [ ] T001: Initialize Project Structure
- [ ] T002: Set Up Testing Framework
- [ ] T003: Define Core Data Structures
- [ ] T007: Implement MemoryMonitor (great starting point!)

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

**Documentation Generated**:
- Total Lines: ~5,900+ lines of technical documentation
- Planning Artifacts: 8 comprehensive documents
- Tasks Defined: 26 executable tasks
- Requirements: 40+ functional, 24+ non-functional
- Test Coverage Target: 80%+

**Planning Completeness**:
- ✅ Constitution (architectural principles)
- ✅ Specification (WHAT and WHY)
- ✅ Implementation Plan (HOW)
- ✅ Research Findings (technical decisions)
- ✅ Data Models (all structures defined)
- ✅ CLI Contract (interface frozen)
- ✅ Tasks (26 tasks, dependency-ordered)

---

**Built with ❤️ for Windows system administrators, developers, and power users.**

**Status**: 🚧 Planning complete, implementation starting soon!

---

<sub>Project follows semantic versioning. Current target: v1.0.0</sub>

