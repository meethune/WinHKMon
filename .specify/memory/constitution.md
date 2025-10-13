<!--
SYNC IMPACT REPORT
==================
Migration Date: 2025-10-13
Action: Initial constitution migration to .specify structure

Version Change: 1.0 → 1.0 (no change)
  Rationale: This is a structural migration from docs/constitution.md to 
  .specify/memory/constitution.md without semantic changes to principles.

Modified Principles: None (initial migration)

Added Sections: None

Removed Sections: None

Template Sync Status:
  ⚠ .specify/templates/plan-template.md - Not yet created
  ⚠ .specify/templates/spec-template.md - Not yet created
  ⚠ .specify/templates/tasks-template.md - Not yet created
  ⚠ .specify/templates/commands/*.md - Not yet created

Follow-up TODOs:
  - Create template files that reference constitutional principles
  - Establish CI/CD checks for constitutional compliance (Section XI)
  - Define code formatting standards (Section IX references but not detailed)
  - Obtain code signing certificate for releases (Section VIII)

Notes:
  - All placeholder values have been filled from existing docs/constitution.md
  - RATIFICATION_DATE inferred as 2025-10-12 based on "Last Updated" metadata
  - No bracketed placeholder tokens remain
  - Constitution is marked as "Draft" status, pending formal ratification
-->

# WinHKMon Constitution

**Version:** 1.0  
**Ratification Date:** 2025-10-12  
**Last Amended Date:** 2025-10-12  
**Status:** Draft

This document establishes the immutable architectural principles and design philosophy for the Windows Hardware Monitor (WinHKMon) project.

---

## I. Project Vision

WinHKMon is a lightweight, native Windows system monitoring tool that provides real-time visibility into system resources (CPU, memory, disk, network, temperature) with minimal performance overhead and maximum accuracy.

**Core Values:**
1. **Performance First**: The monitor must not significantly impact system performance
2. **Accuracy Over Aesthetics**: Precise measurements trump visual polish
3. **Native Integration**: Use Windows native APIs; avoid heavy frameworks
4. **User Privacy**: No telemetry, no network calls, no data collection

---

## II. Architectural Principles

### Principle 1: Library-First Architecture

**Rationale**: Separation of concerns enables reusability and testability.

**Requirements:**
- Core monitoring logic MUST be implemented as a library (`WinHKMonLib`)
- Library MUST have zero UI dependencies
- Library MUST be usable from any C++ application
- All data collection MUST be synchronous and deterministic

**Forbidden:**
- ❌ Embedding monitoring logic in UI code
- ❌ Global state that prevents multiple instances
- ❌ Assumptions about consumer application type

---

### Principle 2: Native Windows API Mandate

**Rationale**: Minimize dependencies, maximize compatibility, ensure longevity.

**Requirements:**
- MUST use Windows SDK APIs (Win32, WinAPI, Windows Runtime)
- MAY use standard C++ library (STL)
- MAY use well-maintained, security-audited third-party libraries for 
  temperature monitoring only
- MUST compile with MSVC (Visual Studio 2022+)
- MUST support Windows 10 21H2+ and Windows 11

**Allowed Third-Party Dependencies:**
- LibreHardwareMonitor (for temperature sensors only)
- Standard Windows SDK components

**Forbidden:**
- ❌ Cross-platform frameworks (Qt, wxWidgets, etc.)
- ❌ Heavy runtime dependencies (.NET Framework for core library)
- ❌ External web services or cloud APIs
- ❌ Kernel drivers (userspace only)

---

### Principle 3: CLI-First Interface

**Rationale**: Command-line interface enables automation, scripting, and flexibility.

**Requirements:**
- Primary interface MUST be command-line executable
- MUST output structured data (JSON, CSV, or custom format)
- MUST support single-shot and continuous monitoring modes
- MUST support selective metric monitoring (user chooses CPU, RAM, etc.)
- GUI components are OPTIONAL extensions, not core requirements

**Forbidden:**
- ❌ GUI-only functionality
- ❌ Requiring interactive input during monitoring
- ❌ Breaking changes to CLI interface without major version bump

---

### Principle 4: Test-First Development

**Rationale**: Ensure reliability and prevent regressions.

**Requirements:**
- MUST write unit tests for all library functions
- MUST achieve minimum 80% code coverage
- MUST include integration tests for API interactions
- MUST validate against known-good values (e.g., Task Manager comparison)
- Performance tests MUST verify monitoring overhead < 1% CPU

**Test Framework:**
- C++ unit tests: Google Test (gtest) or Catch2
- Integration tests: PowerShell scripts or C++ test harness

**Forbidden:**
- ❌ Shipping untested code
- ❌ Mocking Windows APIs without real API validation tests
- ❌ Tests that require specific hardware configurations

---

### Principle 5: Simplicity and Anti-Abstraction

**Rationale**: Over-engineering creates maintenance burden and bugs.

**Requirements:**
- MUST favor simple, direct code over clever abstractions
- MUST avoid unnecessary inheritance hierarchies
- MUST prefer composition over inheritance
- MUST use clear, descriptive names over abbreviations
- SHOULD limit nesting depth to 3 levels

**Examples:**
```cpp
// ✅ GOOD: Direct, clear
class CpuMonitor {
    double getCurrentUsage();
    CpuStats getDetailedStats();
};

// ❌ BAD: Over-abstracted
template<typename MetricProvider, typename StatsAggregator>
class AbstractSystemMonitor : public IMonitorable<T> {
    // ...complex template metaprogramming
};
```

**Forbidden:**
- ❌ Design patterns for the sake of patterns
- ❌ Premature optimization
- ❌ Macro magic and preprocessor abuse
- ❌ Template metaprogramming unless clearly justified

---

### Principle 6: Integration-First Testing

**Rationale**: Real-world behavior matters more than isolated unit behavior.

**Requirements:**
- MUST test against real Windows APIs on target OS versions
- MUST validate output matches Windows Task Manager / Performance Monitor
- MUST test on physical and virtual machines
- MUST test on different CPU architectures (Intel, AMD, ARM64)
- MUST test with different hardware configurations

**Test Scenarios:**
1. Idle system (< 5% CPU usage)
2. High CPU load (stress test)
3. Memory pressure (low available RAM)
4. Network saturation
5. Disk I/O intensive workload
6. Multi-monitor, multi-adapter scenarios

**Forbidden:**
- ❌ Relying solely on mocked API responses
- ❌ Testing only on development machines
- ❌ Ignoring platform-specific behavior differences

---

## III. Security and Privacy Constraints

### Data Handling
- MUST NOT transmit any data over network
- MUST NOT write sensitive data to disk (except transient state files)
- MUST use secure file permissions for state files
- MUST validate all API inputs (buffer sizes, array bounds)

### Resource Access
- MUST run with standard user privileges (no admin required)
- MUST gracefully handle denied permissions
- MUST NOT attempt privilege escalation
- Temperature monitoring MAY require elevation (document clearly)

### State Persistence
- State files MUST be stored in user-specific directories
- State files MUST be readable only by current user
- MUST handle corrupted state files gracefully
- MUST NOT trust data from state files without validation

---

## IV. Performance Constraints

### Resource Limits
- CPU overhead: < 1% on modern CPUs (< 5% on older systems)
- Memory footprint: < 10 MB resident
- Disk I/O: < 1 KB/sec average
- Update frequency: Configurable, default 1 second

### Scalability
- MUST support systems with 1-256 CPU cores
- MUST support systems with 1 GB - 1 TB RAM
- MUST support 1-100 network interfaces
- MUST support 1-100 disk drives

---

## V. Error Handling Philosophy

**Principle**: Fail gracefully, inform clearly, continue when possible.

**Requirements:**
- MUST NOT crash on API failures
- MUST log errors to stderr or log file
- MUST continue monitoring other metrics if one fails
- MUST provide clear error messages (avoid "Unknown error")

**Error Categories:**
1. **Fatal**: Cannot continue (exit with error code)
2. **Degraded**: Continue with reduced functionality (warn user)
3. **Transient**: Retry on next cycle (log at debug level)

---

## VI. Compatibility and Portability

### Platform Support
- **Primary**: Windows 11 (latest release)
- **Secondary**: Windows 10 21H2+
- **Future**: Windows Server 2022+ (nice-to-have)

### Architecture Support
- **Primary**: x64 (AMD64)
- **Secondary**: ARM64 (Windows on ARM)
- **Legacy**: x86 (best effort, may drop in future)

### API Version Constraints
- MUST use APIs available in Windows 10 SDK
- MAY use Windows 11-specific APIs with runtime detection
- MUST provide fallback for older API versions

---

## VII. Documentation Standards

**Requirements:**
- MUST document all public API functions (Doxygen format)
- MUST provide usage examples for CLI
- MUST document all configuration options
- MUST maintain changelog (Keep a Changelog format)
- MUST document known limitations

**Forbidden:**
- ❌ Code without comments for complex logic
- ❌ Undocumented breaking changes
- ❌ Examples that don't compile

---

## VIII. Versioning and Releases

**Semantic Versioning**: MAJOR.MINOR.PATCH

- **MAJOR**: Breaking changes to CLI or library API
- **MINOR**: New features, backward-compatible
- **PATCH**: Bug fixes, no API changes

**Release Checklist:**
1. All tests passing
2. Documentation updated
3. Changelog updated
4. Version numbers updated
5. Tested on Windows 10 and 11
6. Signed binaries (code signing certificate)

---

## IX. Code Quality Gates

### Pre-Commit
- Code compiles without warnings (`/W4` on MSVC)
- Unit tests pass
- Code formatted (consistent style)

### Pre-Implementation (for each feature)
1. Does this violate any constitutional principles?
2. Is there a simpler approach?
3. Are tests defined before implementation?
4. Is the library-first architecture maintained?

### Pre-Release
1. Integration tests pass on Windows 10 and 11
2. Performance benchmarks meet constraints
3. Security review completed (buffer overflows, injection risks)
4. Documentation is complete and accurate

---

## X. Governance

### Amendment Process

This constitution may be amended through:

1. **Proposal**: Document proposed change with rationale
2. **Review**: Project maintainers review for 7 days minimum
3. **Approval**: Unanimous consent of core maintainers
4. **Documentation**: Update this document with version bump

### Versioning Policy

Constitution version follows semantic versioning:
- **MAJOR**: Backward incompatible governance/principle removals or redefinitions
- **MINOR**: New principle/section added or materially expanded guidance
- **PATCH**: Clarifications, wording, typo fixes, non-semantic refinements

### Immutable Principles

The following principles cannot be amended without creating a new project fork:
- Native Windows API mandate (Principle 2)
- User privacy guarantees (Section III, Data Handling)
- Library-first architecture (Principle 1)

---

## XI. Enforcement

Violations of this constitution MUST be addressed before merge:

1. **Automated**: CI/CD checks enforce compilation, testing, performance
2. **Code Review**: Human reviewers verify constitutional compliance
3. **Escalation**: Persistent violations require architecture review

### Compliance Review

Each pull request MUST include a constitutional compliance checklist:
- [ ] Library-first architecture maintained
- [ ] Native Windows APIs used (no forbidden dependencies)
- [ ] CLI functionality preserved/enhanced
- [ ] Tests written and passing (80%+ coverage)
- [ ] Code follows simplicity principles
- [ ] Integration tests validate real-world behavior
- [ ] Security and privacy constraints respected
- [ ] Performance benchmarks met
- [ ] Documentation updated

---

**Signed:**  
WinHKMon Project  
Version 1.0 - Initial Constitution
