# Feature Specification: Windows Hardware Monitor (WinHKMon)

**Version:** 1.0
**Last Updated:** 2025-10-13
**Status:** Draft

WinHKMon is a lightweight, native Windows system monitoring tool that provides real-time visibility into system resources (CPU, memory, disk, network, temperature) with minimal performance overhead and maximum accuracy. It is designed for system administrators, developers, and power users who need accurate monitoring without the overhead of full performance suites.

---

## Executive Summary

WinHKMon addresses the need for a lightweight, scriptable system monitoring solution on Windows. Existing tools like Task Manager require GUI interaction, Performance Monitor is complex and resource-heavy, and third-party solutions often require licenses or include unnecessary features.

**Who Benefits:**
- System administrators managing remote Windows servers
- Software developers monitoring resource usage during testing
- Power users tracking system performance and thermals

**Primary Use Cases:**
1. Real-time system resource monitoring during development/testing
2. Server performance tracking and diagnostics
3. Automated monitoring scripts and alerts
4. Resource usage analysis for optimization
5. System health checks in production environments

---

## I. User Scenarios

### Scenario 1: Server Health Check via PowerShell

**Actor:** System Administrator (Sarah)

**Context:** Sarah manages 50+ Windows servers and needs to quickly check resource usage on a remote system without installing heavy monitoring software.

**Goal:** Verify server is healthy and identify any resource bottlenecks.

**Steps:**
1. Sarah opens PowerShell and SSH into the remote server
2. She runs: `WinHKMon CPU RAM DISK NET --format json`
3. System displays current resource metrics in structured format
4. She reviews CPU usage (18%), memory available (24 GB), disk I/O (moderate), network traffic (low)
5. She identifies no issues and moves to next server

**Success Outcome:** Sarah completes health check in under 30 seconds without GUI tools.

---

### Scenario 2: Development Workflow Integration

**Actor:** Software Developer (David)

**Context:** David is testing a high-performance application and needs to monitor resource usage without switching windows.

**Goal:** Track CPU, memory, and network usage while application runs.

**Steps:**
1. David opens terminal alongside VS Code
2. He runs: `WinHKMon CPU RAM NET --continuous --interval 2 --line`
3. System displays single-line updates every 2 seconds in terminal
4. He launches his application and observes real-time metrics
5. He identifies memory leak (RAM usage steadily climbing)
6. He presses Ctrl+C to stop monitoring

**Success Outcome:** David identifies performance issue without Task Manager context switching.

---

### Scenario 3: Thermal Monitoring During Stress Test

**Actor:** Power User (Emily)

**Context:** Emily has overclocked her CPU and wants to monitor temperatures during a stress test to ensure stability.

**Goal:** Continuously monitor CPU temperature and usage to detect overheating.

**Steps:**
1. Emily opens Command Prompt with administrator privileges
2. She runs: `WinHKMon CPU TEMP --continuous --interval 1`
3. System displays per-core CPU usage and temperature every second
4. She launches stress testing software
5. She watches temperature climb to 82°C and usage hit 100%
6. After 10 minutes of stable operation, she stops the test

**Success Outcome:** Emily confirms her overclock is stable with continuous thermal monitoring.

---

## II. Functional Requirements

### FR-1: CPU Monitoring

**What:** Report CPU usage statistics across all cores and per individual core.

**Why:** Users need to identify CPU bottlenecks, understand load distribution, and monitor CPU frequency scaling.

**Requirements:**

**FR-1.1** MUST report overall CPU usage percentage
- **Acceptance Criteria**: Single value (0-100%) representing total CPU utilization
- **Update Frequency**: Configurable, default 1 second
- **Why**: Quick system load assessment

**FR-1.2** MUST report per-core CPU usage percentage
- **Acceptance Criteria**: Individual percentage for each logical processor
- **Constraint**: Support 1-256 cores
- **Why**: Identify thread distribution and core-specific bottlenecks

**FR-1.3** MUST report current CPU frequency per core
- **Acceptance Criteria**: Frequency in MHz or GHz for each core
- **Fallback**: If per-core unavailable, report average frequency
- **Why**: Monitor frequency scaling and performance states

**FR-1.4** MUST report CPU usage breakdown by type
- **Components**: User time, system time, idle time, I/O wait (if available)
- **Acceptance Criteria**: Percentage breakdown totaling 100%
- **Why**: Identify if load is from user applications vs. kernel

**FR-1.5** SHOULD report CPU usage since system boot
- **Acceptance Criteria**: Cumulative percentages since boot time
- **Why**: Understand historical load patterns

---

### FR-2: Memory Monitoring

**What:** Report physical and virtual memory usage statistics.

**Why:** Users need to detect memory leaks, optimize application memory usage, and avoid out-of-memory conditions.

**Requirements:**

**FR-2.1** MUST report total physical memory
- **Acceptance Criteria**: Total RAM in MB or GB
- **Precision**: 1 MB
- **Why**: Provide system capacity context

**FR-2.2** MUST report available physical memory
- **Acceptance Criteria**: Memory available for new allocations (not just free)
- **Why**: Windows caches data; "available" is more useful than "free"

**FR-2.3** MUST report memory usage breakdown
- **Components**: Used memory (total - available), Cached memory (file cache), Buffers, Committed memory
- **Acceptance Criteria**: Clear breakdown showing memory allocation
- **Why**: Understand what is consuming memory

**FR-2.4** MUST report swap/page file usage
- **Acceptance Criteria**: Total page file size, page file usage, percentage used
- **Why**: Identify if system is swapping (performance degradation indicator)

**FR-2.5** SHOULD report memory usage by process (top N processes)
- **Acceptance Criteria**: Top 5-10 processes by memory consumption with process names
- **Why**: Quickly identify memory hogs

---

### FR-3: Disk I/O Monitoring

**What:** Report disk read/write statistics for physical drives.

**Why:** Users need to identify I/O bottlenecks, monitor disk health, and track data transfer rates.

**Requirements:**

**FR-3.1** MUST report disk device names
- **Acceptance Criteria**: List of physical disks (e.g., "PhysicalDrive0", "nvme0n1")
- **Constraint**: Exclude partitions, show only physical devices
- **Why**: Clear device identification

**FR-3.2** MUST report total bytes read per disk
- **Acceptance Criteria**: Cumulative bytes read since boot
- **Precision**: Display in appropriate units (MB, GB, TB)
- **Why**: Long-term usage tracking

**FR-3.3** MUST report total bytes written per disk
- **Acceptance Criteria**: Cumulative bytes written since boot
- **Precision**: Display in appropriate units (MB, GB, TB)
- **Why**: Track write endurance (SSD health)

**FR-3.4** MUST report read/write transfer rates
- **Acceptance Criteria**: Current transfer rate in MB/s or GB/s
- **Calculation**: Delta between current and previous sample / time elapsed
- **Why**: Real-time performance monitoring

**FR-3.5** MUST report disk size
- **Acceptance Criteria**: Total disk capacity in GB or TB
- **Why**: Provide context for usage statistics

**FR-3.6** SHOULD report disk I/O time / busy percentage
- **Acceptance Criteria**: Percentage of time disk is servicing requests (0-100%)
- **Why**: Identifies if disk is saturated

---

### FR-4: Network Monitoring

**What:** Report network traffic statistics for network interfaces.

**Why:** Users need to monitor bandwidth usage, identify network bottlenecks, and track data transfer.

**Requirements:**

**FR-4.1** MUST report all active network interfaces
- **Acceptance Criteria**: List of interface names (e.g., "Ethernet", "Wi-Fi", "vEthernet")
- **Constraint**: Exclude loopback interfaces by default
- **Why**: Support multi-homed systems

**FR-4.2** MUST report bytes received per interface
- **Acceptance Criteria**: Cumulative bytes received since boot
- **Precision**: Display in appropriate units (MB, GB, TB)
- **Why**: Track total data received

**FR-4.3** MUST report bytes sent per interface
- **Acceptance Criteria**: Cumulative bytes sent since boot
- **Precision**: Display in appropriate units (MB, GB, TB)
- **Why**: Track total data transmitted

**FR-4.4** MUST report receive/transmit rates
- **Acceptance Criteria**: Current transfer rate in Kbps, Mbps, or Gbps
- **Unit Preference**: User-configurable (bits/sec or bytes/sec)
- **Calculation**: Delta between samples / time elapsed
- **Why**: Real-time bandwidth monitoring

**FR-4.5** MUST auto-select primary network interface
- **Selection Criteria**: Interface with highest traffic volume
- **Fallback**: First non-loopback interface if no traffic detected
- **Why**: Simplify output when user doesn't specify interface

**FR-4.6** SHOULD allow user to specify interface
- **Acceptance Criteria**: CLI argument to select specific interface by name
- **Syntax**: `WinHKMon "Ethernet"` or `WinHKMon --interface "Wi-Fi"`
- **Why**: Multi-homed systems may have multiple active interfaces

**FR-4.7** SHOULD report interface link speed
- **Acceptance Criteria**: Maximum negotiated speed (e.g., "1 Gbps", "10 Gbps")
- **Why**: Provides context for current usage vs. capacity

---

### FR-5: Temperature Monitoring

**What:** Report hardware temperature readings from system sensors.

**Why:** Users need to monitor thermals for overheating prevention, cooling system validation, and overclocking stability.

**Requirements:**

**FR-5.1** MUST report CPU temperature
- **Acceptance Criteria**: Temperature in Celsius
- **Precision**: 1°C
- **Source**: CPU thermal sensors (via LibreHardwareMonitor or equivalent)
- **Why**: Critical for thermal management

**FR-5.2** SHOULD report per-core CPU temperature
- **Acceptance Criteria**: Individual temperature for each CPU core
- **Fallback**: Report package/average temperature if per-core unavailable
- **Why**: Hotspots may exist on specific cores

**FR-5.3** SHOULD report min/max/average for multi-core systems
- **Acceptance Criteria**: Statistics across all cores (minimum, maximum, average)
- **Why**: Quickly identify thermal envelope

**FR-5.4** MAY report GPU temperature
- **Acceptance Criteria**: GPU core temperature if sensors available
- **Why**: Gaming/rendering workloads generate significant heat

**FR-5.5** MAY report motherboard/chipset temperature
- **Acceptance Criteria**: Additional sensor readings if available
- **Why**: Complete thermal picture

**FR-5.6** MUST gracefully handle missing temperature sensors
- **Acceptance Criteria**: Skip temperature reporting if no sensors detected, inform user via message
- **Error Handling**: Do not crash or fail; continue monitoring other metrics
- **Why**: Not all systems expose temperature sensors to software

---

### FR-6: Output Formats

**What:** Provide multiple output formats for different use cases.

**Why:** Different users have different consumption needs (human-readable vs. machine-parseable).

**Requirements:**

**FR-6.1** MUST support compact text output
- **Format**: Multi-line text suitable for terminal display
- **Example**:
  ```
  CPU:  23.5%  2.4 GHz
  RAM:  8192M available
  NET:  15.3 Mbps ↓  2.1 Mbps ↑
  TEMP: 45°C
  ```
- **Why**: Quick visual inspection

**FR-6.2** MUST support detailed text output
- **Format**: Expanded view with per-component breakdowns
- **Trigger**: Verbose flag or expanded mode
- **Example**:
  ```
  CPU ⚙ 23.5% ≈ 2.4 GHz:
    18.2% user
     3.1% system
    78.7% idle
    Core 0: 45% @ 2.8 GHz
    Core 1: 12% @ 2.1 GHz
  ```
- **Why**: Debugging and detailed analysis

**FR-6.3** SHOULD support JSON output
- **Format**: Structured JSON for programmatic consumption
- **Schema**: Well-defined, versioned schema
- **Syntax**: `--format json` or `-f json`
- **Why**: Integration with monitoring systems, scripts, dashboards

**FR-6.4** SHOULD support CSV output
- **Format**: Comma-separated values with headers
- **Syntax**: `--format csv` or `-f csv`
- **Why**: Import into Excel, data analysis tools

**FR-6.5** MUST support single-line output mode
- **Format**: All metrics on one line (space-separated)
- **Syntax**: `--line` or `LINE`
- **Why**: Status bars, compact displays (tmux, conky equivalents)

---

### FR-7: State Persistence

**What:** Store previous monitoring state to calculate deltas and rates.

**Why:** Many metrics (network bandwidth, disk I/O) require delta calculations between samples.

**Requirements:**

**FR-7.1** MUST store previous sample data
- **Location**: User-specific temporary directory (AppData\Local\Temp or similar)
- **Filename**: `WinHKMon.dat` or similar
- **Contents**: Previous sample values and timestamp
- **Why**: Enable rate calculations

**FR-7.2** MUST handle missing or corrupt state files
- **Behavior**: Initialize with current sample, no delta calculation for first run
- **Error Handling**: Log warning, do not crash
- **Why**: State file may be deleted or corrupted

**FR-7.3** MUST use monotonic timestamps
- **Requirement**: Clock source unaffected by system time changes
- **Implementation**: Use performance counter or monotonic clock API
- **Why**: Prevent negative deltas if user changes system time

**FR-7.4** MUST validate state file version
- **Behavior**: Ignore state file if version mismatch
- **Why**: Schema changes between versions

**FR-7.5** SHOULD use secure file permissions
- **Requirement**: State file readable/writable only by current user
- **Why**: Prevent tampering or information disclosure

---

### FR-8: Command-Line Interface

**What:** Provide a flexible CLI for invoking the monitor.

**Why:** Enables scripting, automation, and integration with other tools.

**Requirements:**

**FR-8.1** MUST support selective metric monitoring
- **Syntax**: `WinHKMon [NET] [CPU] [RAM] [TEMP] [IO]`
- **Behavior**: Only monitor specified metrics
- **Default**: If no metrics specified, show usage help
- **Why**: User may only need specific metrics

**FR-8.2** MUST support specific network interface selection
- **Syntax**: `WinHKMon <interface_name>` or `WinHKMon --interface "Ethernet"`
- **Example**: `WinHKMon "Ethernet"`
- **Behavior**: Monitor specified interface instead of auto-selection
- **Why**: Multi-homed systems need explicit selection

**FR-8.3** MUST support output format selection
- **Syntax**: `--format json` or `-f csv`
- **Options**: `text`, `json`, `csv`
- **Default**: `text`
- **Why**: Machine-parseable vs. human-readable

**FR-8.4** MUST support continuous monitoring mode
- **Syntax**: `--continuous` or `-c`
- **Behavior**: Output samples continuously until interrupted (Ctrl+C)
- **Update Rate**: Configurable via `--interval <seconds>`
- **Default**: Single sample (one-shot mode)
- **Why**: Long-running monitoring scenarios

**FR-8.5** MUST support interval configuration
- **Syntax**: `--interval 5` (5 second updates)
- **Range**: 0.1 - 3600 seconds
- **Default**: 1 second
- **Why**: Different use cases need different update rates

**FR-8.6** MUST support single-line output flag
- **Syntax**: `--line` or `LINE`
- **Behavior**: Output all metrics on single line
- **Why**: Status bar integration

**FR-8.7** MUST support version flag
- **Syntax**: `--version` or `-v`
- **Output**: Version number and build information
- **Why**: Troubleshooting and compatibility checks

**FR-8.8** MUST support help flag
- **Syntax**: `--help` or `-h`
- **Output**: Usage documentation with examples
- **Why**: User discoverability

**FR-8.9** SHOULD support network unit preference
- **Syntax**: `--net-units bits` or `--net-units bytes`
- **Behavior**: Display network speeds in bits/sec or bytes/sec
- **Default**: `bits` (Mbps, Gbps)
- **Why**: Different conventions in different contexts

**FR-8.10** MAY support output redirection options
- **Syntax**: `--output file.log`
- **Behavior**: Write to file instead of stdout
- **Note**: User can use shell redirection (`>`) as alternative

---

## III. Non-Functional Requirements

### NFR-1: Performance

**Why:** Monitoring tool must not significantly degrade system performance.

**Requirements:**

**NFR-1.1** CPU overhead MUST be < 1% on modern CPUs
- **Measurement**: Average CPU usage over 60 seconds
- **Test System**: 4+ core CPU, 8+ GB RAM
- **Exception**: First sample may be higher (initialization overhead)

**NFR-1.2** Memory footprint MUST be < 10 MB
- **Measurement**: Resident Set Size (RSS) in steady state
- **Why**: Lightweight tool should have minimal memory impact

**NFR-1.3** Disk I/O MUST be < 1 KB/sec average
- **Measurement**: Bytes written to state file / time
- **Why**: Avoid disk thrashing on systems with slow storage

**NFR-1.4** Startup time SHOULD be < 200ms
- **Measurement**: Time from process start to first output
- **Why**: Responsive user experience

---

### NFR-2: Reliability

**Why:** Monitoring tool must be trustworthy and stable.

**Requirements:**

**NFR-2.1** MUST NOT crash on API failures
- **Behavior**: Log error, continue monitoring other metrics
- **Why**: Partial functionality better than complete failure

**NFR-2.2** MUST handle systems with unusual configurations
- **Examples**: 256 cores, 100 network interfaces, no temperature sensors
- **Behavior**: Scale gracefully, truncate output if necessary

**NFR-2.3** Uptime SHOULD exceed 24 hours in continuous mode
- **Measurement**: No crashes or hangs over 24+ hour test
- **Why**: Long-running monitoring scenarios

**NFR-2.4** Accuracy MUST be within 5% of Task Manager
- **Measurement**: Compare CPU%, RAM, network stats with Task Manager readings
- **Why**: Users expect consistency with built-in tools

---

### NFR-3: Compatibility

**Why:** Tool must work on variety of Windows systems.

**Requirements:**

**NFR-3.1** MUST support Windows 10 21H2 and later
- **Test Coverage**: Windows 10 21H2, Windows 11 21H2, Windows 11 23H2

**NFR-3.2** MUST support x64 architecture
- **Priority**: Primary target

**NFR-3.3** SHOULD support ARM64 architecture
- **Priority**: Secondary target (Windows on ARM)
- **Testing**: Best-effort on available hardware

**NFR-3.4** SHOULD support x86 architecture
- **Priority**: Low (legacy systems)
- **Note**: May be dropped in future versions

**NFR-3.5** MUST work on physical and virtual machines
- **Test Coverage**: VMware, Hyper-V, VirtualBox

---

### NFR-4: Security

**Why:** Monitoring tool should not introduce security risks.

**Requirements:**

**NFR-4.1** MUST run with standard user privileges
- **Exception**: Temperature monitoring MAY require elevation (document clearly)
- **Why**: Minimize attack surface

**NFR-4.2** MUST NOT transmit any data over network
- **Verification**: Network traffic analysis shows zero outbound connections
- **Why**: User privacy guarantee

**NFR-4.3** MUST validate all API inputs
- **Examples**: Buffer sizes, array bounds, string lengths
- **Why**: Prevent buffer overflows and injection attacks

**NFR-4.4** State files MUST be user-specific and secured
- **Permissions**: Read/write only by current user
- **Location**: User's temp directory or AppData
- **Why**: Prevent information disclosure

---

### NFR-5: Usability

**Why:** Tool should be easy to use and understand.

**Requirements:**

**NFR-5.1** Error messages MUST be clear and actionable
- **Bad Example**: "Error 0x80070005"
- **Good Example**: "Access denied reading temperature sensors. Run as Administrator or disable TEMP monitoring."

**NFR-5.2** Default behavior SHOULD be intuitive
- **Example**: Running without arguments shows help, not cryptic error
- **Example**: Auto-select primary network interface

**NFR-5.3** Output SHOULD use Unicode symbols where appropriate
- **Examples**: ↓↑ for network, ⚙ for CPU, ⚡ for disk I/O
- **Fallback**: ASCII alternatives if console doesn't support Unicode

**NFR-5.4** Documentation MUST include examples
- **Coverage**: Common use cases, troubleshooting, scripting integration

---

### NFR-6: Maintainability

**Why:** Code should be maintainable over time.

**Requirements:**

**NFR-6.1** Code MUST compile without warnings
- **Compiler Flags**: MSVC `/W4`
- **Why**: Catch potential bugs early

**NFR-6.2** Public API MUST be documented
- **Format**: Doxygen-compatible comments
- **Coverage**: All public functions and classes

**NFR-6.3** Breaking changes MUST bump major version
- **Example**: Changing CLI argument format, JSON schema
- **Why**: Semantic versioning expectations

---

## IV. Success Criteria

**Criterion 1: Performance Impact**
- **Target**: CPU overhead < 1% on 4-core systems
- **Target**: Memory usage < 10 MB resident
- **Measurement**: Resource Monitor readings during 60-second continuous monitoring
- **Rationale**: Tool must be truly lightweight to not impact monitored system

**Criterion 2: Accuracy**
- **Target**: All metrics within 5% of Task Manager readings
- **Target**: Zero incorrect readings (negative percentages, impossible values)
- **Measurement**: Side-by-side comparison with Task Manager over 100 samples
- **Rationale**: Users need to trust the tool's measurements

**Criterion 3: Reliability**
- **Target**: Zero crashes in 24-hour continuous monitoring test
- **Target**: Graceful degradation if individual metrics fail
- **Measurement**: Automated stress testing on various Windows configurations
- **Rationale**: Monitoring tool must be more stable than applications it monitors

**Criterion 4: Usability**
- **Target**: New users can run first command within 1 minute of reading help
- **Target**: 90% of users complete common tasks without consulting documentation
- **Measurement**: User testing sessions with 10+ participants
- **Rationale**: Tool should be immediately useful without steep learning curve

**Criterion 5: Adoption**
- **Target**: 100+ GitHub stars within 6 months of release
- **Target**: 1000+ downloads within 1 year
- **Measurement**: GitHub analytics and download statistics
- **Rationale**: Indicates tool fills genuine user need

**Criterion 6: User Satisfaction**
- **Target**: Positive feedback from 80%+ of users
- **Target**: < 5 bug reports per month after initial release period
- **Measurement**: GitHub issues, discussions, surveys
- **Rationale**: Users prefer this tool over alternatives

---

## V. Key Entities

**Entity: SystemMetrics**
- **Description**: Container for all system resource measurements at a point in time
- **Key Attributes**:
  - Timestamp: Monotonic time when metrics were captured
  - CpuMetrics: CPU usage and frequency data
  - MemoryMetrics: RAM and page file data
  - DiskMetrics: Disk I/O statistics
  - NetworkMetrics: Network traffic data
  - TemperatureMetrics: Thermal sensor readings (optional)
- **Relationships**: Composed of individual metric categories

**Entity: CpuMetrics**
- **Description**: CPU usage and performance data
- **Key Attributes**:
  - OverallUsage: Total CPU percentage (0-100)
  - PerCoreUsage: Array of per-core percentages
  - PerCoreFrequency: Array of per-core frequencies (MHz)
  - UserTime: Percentage in user mode
  - SystemTime: Percentage in kernel mode
  - IdleTime: Percentage idle
- **Relationships**: Part of SystemMetrics

**Entity: MemoryMetrics**
- **Description**: Physical and virtual memory statistics
- **Key Attributes**:
  - TotalPhysical: Total RAM (bytes)
  - AvailablePhysical: Available RAM (bytes)
  - UsedPhysical: Used RAM (bytes)
  - CachedMemory: File cache size (bytes)
  - PageFileTotal: Total page file size (bytes)
  - PageFileUsed: Used page file (bytes)
- **Relationships**: Part of SystemMetrics

**Entity: DiskMetrics**
- **Description**: Disk I/O statistics per physical drive
- **Key Attributes**:
  - DeviceName: Physical disk identifier
  - TotalBytesRead: Cumulative reads since boot (bytes)
  - TotalBytesWritten: Cumulative writes since boot (bytes)
  - ReadRate: Current read throughput (bytes/sec)
  - WriteRate: Current write throughput (bytes/sec)
  - DiskSize: Total capacity (bytes)
  - BusyPercentage: Disk active time (0-100)
- **Relationships**: Multiple instances per SystemMetrics (one per disk)

**Entity: NetworkMetrics**
- **Description**: Network traffic statistics per interface
- **Key Attributes**:
  - InterfaceName: Network adapter name
  - BytesReceived: Cumulative bytes received since boot
  - BytesSent: Cumulative bytes sent since boot
  - ReceiveRate: Current download speed (bytes/sec)
  - TransmitRate: Current upload speed (bytes/sec)
  - LinkSpeed: Maximum interface speed (bytes/sec)
  - IsActive: Whether interface is currently transmitting
- **Relationships**: Multiple instances per SystemMetrics (one per interface)

**Entity: TemperatureMetrics**
- **Description**: Thermal sensor readings
- **Key Attributes**:
  - CpuTemperature: Overall CPU temperature (Celsius)
  - PerCoreTemperature: Array of per-core temperatures (optional)
  - MinTemperature: Minimum across all sensors
  - MaxTemperature: Maximum across all sensors
  - AverageTemperature: Average across all sensors
  - GpuTemperature: GPU temperature if available (optional)
- **Relationships**: Part of SystemMetrics (optional, may be absent)

**Entity: MonitorState**
- **Description**: Persistent state for delta calculations
- **Key Attributes**:
  - Version: State file format version
  - LastSampleTime: Timestamp of previous sample
  - LastNetworkCounters: Previous network byte counts
  - LastDiskCounters: Previous disk byte counts
- **Relationships**: Serialized to state file between runs

---

## VI. Edge Cases and Error Scenarios

**Edge Case 1: System with 256 CPU Cores**
- **Condition**: High-core-count server or workstation
- **Expected Behavior**: Report all cores but may truncate display in compact mode; full data available in JSON/CSV
- **Why Important**: Tool must scale to modern high-core systems

**Edge Case 2: Multiple Network Interfaces with Equal Traffic**
- **Condition**: Multi-homed system with balanced network load
- **Expected Behavior**: Auto-select based on lexicographic order if traffic volumes are equal
- **Why Important**: Deterministic behavior for scripting

**Edge Case 3: No Temperature Sensors Available**
- **Condition**: Virtual machine or system without accessible thermal sensors
- **Expected Behavior**: Skip temperature output with informational message ("TEMP: sensors unavailable")
- **Why Important**: Tool should work on all Windows systems including VMs

**Edge Case 4: System Time Change During Monitoring**
- **Condition**: User changes system clock or daylight saving time transition
- **Expected Behavior**: Use monotonic clock for deltas; no negative values or incorrect rates
- **Why Important**: Reliable metrics regardless of wall clock changes

**Edge Case 5: Disk with Zero I/O**
- **Condition**: Idle disk with no read/write activity
- **Expected Behavior**: Report 0 MB/s rates and 0% busy time
- **Why Important**: Distinguish idle from error condition

**Error Scenario 1: State File Corruption**
- **Trigger**: Power loss during state file write, disk errors
- **User Experience**: First sample shows no delta/rate calculations; warning in verbose mode
- **Recovery**: Tool recreates state file on next run

**Error Scenario 2: Permission Denied on Temperature Sensors**
- **Trigger**: Running as standard user when sensor access requires elevation
- **User Experience**: "TEMP: Access denied. Run as Administrator to enable temperature monitoring."
- **Recovery**: User can either elevate or continue without temperature monitoring

**Error Scenario 3: Invalid Network Interface Name**
- **Trigger**: User specifies non-existent interface: `WinHKMon "NonExistent"`
- **User Experience**: Clear error: "Interface 'NonExistent' not found. Available: Ethernet, Wi-Fi"
- **Recovery**: User corrects interface name or omits for auto-selection

**Error Scenario 4: Extremely High Metric Values**
- **Trigger**: Counter overflow, API bug, or calculation error
- **User Experience**: Cap values at reasonable maximums (100% for percentages, physical limits for rates)
- **Recovery**: Log anomaly, display capped value, continue monitoring

**Error Scenario 5: API Failure for Individual Metric**
- **Trigger**: Windows API returns error for CPU or memory query
- **User Experience**: Display error for failed metric, continue showing other metrics
- **Recovery**: Retry on next sample; partial data is better than no data

---

## VII. Out of Scope

**OS-1** GUI Application
- **Rationale**: CLI-first approach per project constitution
- **Future**: May add in v2.0 as separate optional component

**OS-2** Historical Data Logging and Graphing
- **Rationale**: Adds complexity; many existing tools do this well
- **Workaround**: User can redirect output to file and process separately with existing tools

**OS-3** Alerting and Notifications
- **Rationale**: User can script alerts based on output
- **Future**: May add threshold flags in v1.1 (e.g., `--warn-cpu 80`)

**OS-4** Process-Level Monitoring
- **Rationale**: Task Manager already provides comprehensive process monitoring
- **Exception**: Top N processes by memory (FR-2.5) is included for quick identification

**OS-5** Remote Monitoring
- **Rationale**: Adds security complexity and network requirements
- **Workaround**: User can SSH/RDP and run locally

**OS-6** Plugin System
- **Rationale**: Premature complexity for v1.0
- **Future**: May add if strong use case emerges with community demand

**OS-7** Custom Sensor Polling Beyond Standard APIs
- **Rationale**: Requires kernel drivers, significant security risk
- **Exception**: Use LibreHardwareMonitor for sensors it already supports

**OS-8** GPU Usage Monitoring (Utilization Percentage)
- **Rationale**: Vendor-specific APIs (NVIDIA, AMD, Intel) add significant complexity
- **Note**: GPU temperature MAY be included if accessible via LibreHardwareMonitor
- **Future**: May add in v1.2 if demand is high

**OS-9** Configuration Files
- **Rationale**: CLI arguments are sufficient for v1.0; adds complexity
- **Future**: May add `WinHKMon.ini` support if users request persistent settings

**OS-10** Battery Status Monitoring
- **Rationale**: Not core to system resource monitoring focus
- **Future**: May add as optional module for laptop users

**OS-11** Localization (Non-English Languages)
- **Rationale**: English-only for v1.0 reduces testing complexity
- **Future**: Revisit based on international user demand

---

## VIII. Dependencies and Assumptions

### Dependencies

**D-1:** Windows SDK (Win32 API)
- **Type**: System API
- **Impact if Unavailable**: Cannot compile or run
- **Mitigation**: Windows SDK is standard with Visual Studio

**D-2:** LibreHardwareMonitor Library
- **Type**: Third-party library
- **Impact if Unavailable**: Temperature monitoring unavailable
- **Mitigation**: Make temperature feature optional; degrade gracefully

**D-3:** MSVC Compiler (Visual Studio 2022+)
- **Type**: Build toolchain
- **Impact if Unavailable**: Cannot compile
- **Mitigation**: Document compiler requirements clearly

**D-4:** Windows Performance Data Helper (PDH) API
- **Type**: System API
- **Impact if Unavailable**: Some metrics may require alternative APIs
- **Mitigation**: Hybrid approach using native APIs as fallback

### Assumptions

**A-1:** Users have basic command-line knowledge
**A-2:** Users have Windows 10 21H2 or later
**A-3:** Users can read English documentation
**A-4:** Target systems have at least 2 GB RAM, dual-core CPU
**A-5:** LibreHardwareMonitor library remains available and maintained
**A-6:** Windows APIs for performance data remain stable across versions
**A-7:** Users accept that temperature monitoring may require administrator privileges
**A-8:** Systems have Unicode-capable console for optimal display (fallback to ASCII available)

---

## IX. Constraints

**C-1:** Must use Windows SDK APIs (no Linux/macOS code)
**C-2:** Must compile with MSVC (Visual Studio 2022+)
**C-3:** Must not require third-party licenses (GPL/LGPL acceptable)
**C-4:** Must work without internet connection (fully offline capable)
**C-5:** Must not require installation (portable executable preferred)
**C-6:** Must maintain library-first architecture (core logic in reusable library)
**C-7:** Cannot use kernel drivers (userspace only per constitution)
**C-8:** Must not transmit any data over network (privacy guarantee)

---

## X. Glossary

**Term** | **Definition**
---------|---------------
**API** | Application Programming Interface - Windows SDK functions
**CLI** | Command-Line Interface - text-based interaction
**CPU** | Central Processing Unit - processor executing instructions
**Delta** | Difference between current and previous value
**I/O** | Input/Output - disk read/write operations
**Mbps** | Megabits per second - network speed unit
**PDH** | Performance Data Helper - Windows performance monitoring API
**RAM** | Random Access Memory - physical system memory
**RSS** | Resident Set Size - memory actually used by process
**Swap** | Virtual memory on disk (page file in Windows terminology)
**WMI** | Windows Management Instrumentation - management API
**Monotonic Clock** | Time source unaffected by system time changes
**LibreHardwareMonitor** | Open-source hardware sensor library
**Page File** | Windows term for swap space on disk
**Thermal Sensor** | Hardware component measuring temperature

---

**Document Control:**
- **Author**: WinHKMon Project Team
- **Reviewers**: TBD
- **Approval Date**: Pending
- **Next Review**: After planning phase and before implementation
