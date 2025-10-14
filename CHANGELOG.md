# Changelog

All notable changes to WinHKMon will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned
- US3: Temperature monitoring with LibreHardwareMonitor integration
- Performance optimizations
- Documentation improvements
- Security review

## [0.6.0] - 2025-10-14

### Added
- **Network Monitoring**: Complete network interface statistics with rate calculations
- **Disk Space Monitoring**: Per-drive capacity, used, and free space (DISK metric)
- **Disk I/O Monitoring**: Read/write rates and busy percentage (IO metric)
- Separate DISK (space) and IO (I/O rates) metrics for clarity
- State management for network and disk rate calculations
- Delta calculator for accurate rate computations
- ASCII symbol output (`<`, `>`) for universal Windows console compatibility

### Changed
- Separated disk monitoring into two distinct metrics:
  - `DISK`: Shows disk space like Linux `df` command
  - `IO`: Shows I/O rates like Linux `iostat` command
- Output formatter now accepts `CliOptions` parameter
- Replaced UTF-8 arrows with ASCII symbols for console compatibility
- Drive letter extraction from PDH counter names
- `_Total` entry handling: skip for DISK space, show for IO aggregation

### Fixed
- Network rate calculations now use proper state management
- Disk I/O rates now use PDH-provided rates directly (not cumulative counters)
- Console garbled characters fixed with ASCII symbols
- StateManager integration for persistent delta calculations

### Technical
- **Tests**: 123 tests passing (100% success rate)
- **Platform**: Windows 10/11 with MSVC 17.14
- **Coverage**: All core system monitoring components complete
- **Architecture**: Library-first design with CLI wrapper

### Documentation
- Updated spec.md with DISK/IO separation
- Updated plan.md with implementation details
- Updated tasks.md marking Phase 4 complete
- Added comprehensive diagnostic documentation in docs/
- Updated IMPLEMENTATION_STATUS.md

## [0.5.0-mvp] - 2025-10-13

### Added
- **CPU Monitoring**: Total and per-core usage percentages and frequencies
- **Memory Monitoring**: Physical RAM and page file statistics
- Command-line interface with argument parsing
- Output formats: text, JSON, CSV
- Single-shot and continuous monitoring modes
- State manager for delta calculations
- Delta calculator for rate metrics
- Ctrl+C graceful shutdown

### Technical
- Library-first architecture (WinHKMonLib)
- Performance Data Helper (PDH) API integration
- `GlobalMemoryStatusEx()` for memory stats
- `CallNtPowerInformation()` for CPU frequency
- Google Test framework with 80+ tests
- CMake build system

### Documentation
- Complete specification (spec.md)
- Implementation plan (plan.md)
- Task breakdown (tasks.md)
- CLI interface contract
- Quick start guide

## [0.1-foundation] - 2025-10-12

### Added
- Project structure with CMake
- Core data structures (Types.h)
- Testing framework setup with Google Test
- CLI argument parser
- Output formatter (text, JSON, CSV)
- State manager for persistence
- Initial build configuration

### Technical
- C++17 with MSVC compiler
- Windows SDK 10.0.19041+ support
- Constitutional principles established
- Test-first development approach

---

## Release Notes Format

Each release includes:
- **Architecture**: Windows x64 (ARM64 optional)
- **Compiler**: MSVC 17.0 (Visual Studio 2022)
- **OS Support**: Windows 10 21H2+ and Windows 11
- **Quality Gates**: All tests passing, performance validated

For detailed implementation status, see [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md).

[unreleased]: https://github.com/YOUR_USERNAME/WinHKMon/compare/v0.6.0...HEAD
[0.6.0]: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v0.6.0
[0.5.0-mvp]: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v0.5.0-mvp
[0.1-foundation]: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v0.1-foundation

