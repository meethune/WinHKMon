# WinHKMon CLI Interface Contract

**Phase:** 1 (Design & Contracts)
**Date:** 2025-10-13
**Status:** Complete
**Version:** 1.0

This document defines the complete command-line interface contract for WinHKMon, including all arguments, flags, output formats, and exit codes.

---

## Command Syntax

```
WinHKMon [METRICS...] [OPTIONS...] [INTERFACE]
```

---

## Metric Selectors

Specify one or more metrics to monitor. If no metrics specified, show help.

| Argument | Description | Requirement |
|----------|-------------|-------------|
| `CPU` | Monitor CPU usage and frequency | FR-8.1 |
| `RAM` | Monitor memory (RAM and page file) | FR-8.1 |
| `DISK` | Monitor disk I/O for all physical disks | FR-8.1 |
| `NET` | Monitor network traffic (auto-select interface) | FR-8.1 |
| `TEMP` | Monitor temperature sensors (requires admin) | FR-8.1 |
| `IO` | Alias for `DISK` | FR-8.1 |

**Case Insensitive**: Metrics are case-insensitive (`CPU`, `cpu`, `Cpu` all valid)

**Examples**:
```bash
WinHKMon CPU                  # CPU only
WinHKMon CPU RAM              # CPU and memory
WinHKMon CPU RAM DISK NET TEMP  # All metrics
WinHKMon cpu ram              # Case insensitive
```

---

## Network Interface Selection

| Argument | Description | Requirement |
|----------|-------------|-------------|
| `"<interface_name>"` | Monitor specific network interface | FR-8.2 |
| `--interface <name>` | Alternative syntax for interface selection | FR-8.2 |

**Interface Name Rules**:
- Must be quoted if contains spaces: `"Ethernet 2"`
- Must match exact interface name (case-sensitive)
- If interface not found, error message lists available interfaces

**Auto-Selection Algorithm** (when no interface specified):
1. Exclude loopback interfaces
2. Select interface with highest total traffic (InOctets + OutOctets)
3. If tie, prefer Ethernet, then Wi-Fi
4. Fallback: first non-loopback interface

**Examples**:
```bash
WinHKMon "Ethernet"                    # Specific interface with NET implied
WinHKMon NET "Wi-Fi"                   # Explicit NET with specific interface
WinHKMon --interface "Ethernet 2" NET  # Alternative syntax
```

---

## Output Format Options

| Flag | Short | Description | Default | Requirement |
|------|-------|-------------|---------|-------------|
| `--format <fmt>` | `-f <fmt>` | Set output format: `text`, `json`, `csv` | `text` | FR-8.3 |
| `--line` | `-l` | Single-line compact output | false | FR-8.6 |
| `LINE` | | Alternative to `--line` flag | false | FR-8.6 |

**Format Details**:

**`text`** (default): Human-readable multi-line output
```
CPU:  23.5%  2.4 GHz
RAM:  8192M available
NET:  eth0 ↑ 2.1 Mbps  ↓ 15.3 Mbps
```

**`json`**: Structured JSON for programmatic consumption
```json
{
  "version": "1.0",
  "timestamp": "2025-10-13T14:32:15Z",
  "cpu": {"totalUsagePercent": 23.5, ...}
}
```

**`csv`**: Comma-separated values with headers
```csv
timestamp,cpu_percent,cpu_mhz,ram_available_mb,...
2025-10-13T14:32:15Z,23.5,2400,8192,...
```

**`--line` / `LINE`**: All metrics on single line (status bar mode)
```
23.5% 2.4GHz  8192M  eth0:2.1↑15.3↓Mbps  45°C
```

**Examples**:
```bash
WinHKMon CPU RAM --format json  # JSON output
WinHKMon CPU RAM -f csv        # CSV output
WinHKMon CPU RAM --line        # Single line
WinHKMon CPU RAM LINE          # Single line (alternative)
```

---

## Monitoring Mode Options

| Flag | Short | Description | Default | Requirement |
|------|-------|-------------|---------|-------------|
| `--continuous` | `-c` | Continuous monitoring (until Ctrl+C) | false | FR-8.4 |
| `--interval <sec>` | `-i <sec>` | Update interval in seconds | 1 | FR-8.5 |

**Interval Rules**:
- Range: 0.1 to 3600 seconds (0.1 sec to 1 hour)
- Fractional values supported: `0.5`, `1.5`, `2.0`
- Invalid values show error and usage

**Examples**:
```bash
WinHKMon CPU RAM --continuous                    # Update every 1 sec (default)
WinHKMon CPU RAM -c --interval 5                 # Update every 5 seconds
WinHKMon CPU RAM -c -i 0.5                      # Update every 500ms
WinHKMon CPU TEMP --continuous --interval 2     # Temperature every 2 seconds
```

---

## Network Unit Options

| Flag | Description | Default | Requirement |
|------|-------------|---------|-------------|
| `--net-units bits` | Display network speeds in bits/sec (Mbps, Gbps) | default | FR-8.9 |
| `--net-units bytes` | Display network speeds in bytes/sec (MB/s, GB/s) | | FR-8.9 |

**Examples**:
```bash
WinHKMon NET                         # Default: Mbps
WinHKMon NET --net-units bits        # Explicit: Mbps, Gbps
WinHKMon NET --net-units bytes       # MB/s, GB/s
```

---

## Help and Version Options

| Flag | Short | Description | Requirement |
|------|-------|-------------|-------------|
| `--help` | `-h` | Display usage information and examples | FR-8.8 |
| `--version` | `-v` | Display version and build information | FR-8.7 |

**Help Output Format**:
```
WinHKMon v1.0 - Windows Hardware Monitor

USAGE:
  WinHKMon [METRICS...] [OPTIONS...] [INTERFACE]

METRICS:
  CPU           Monitor CPU usage and frequency
  RAM           Monitor memory (RAM and page file)
  DISK, IO      Monitor disk I/O
  NET           Monitor network traffic
  TEMP          Monitor temperature (requires admin)

OPTIONS:
  --format, -f <fmt>     Output format: text, json, csv (default: text)
  --line, -l, LINE       Single-line compact output
  --continuous, -c       Continuous monitoring (until Ctrl+C)
  --interval, -i <sec>   Update interval (default: 1, range: 0.1-3600)
  --net-units <unit>     Network units: bits or bytes (default: bits)
  --interface <name>     Specific network interface
  --help, -h             Show this help
  --version, -v          Show version

EXAMPLES:
  WinHKMon CPU RAM                  # Single sample of CPU and memory
  WinHKMon NET "Ethernet"           # Network stats for specific interface
  WinHKMon CPU RAM -c -i 5          # Continuous monitoring, 5 sec intervals
  WinHKMon CPU TEMP --format json   # JSON output
  WinHKMon CPU RAM LINE             # Single-line output for status bars

For more information: https://github.com/yourorg/WinHKMon
```

**Version Output Format**:
```
WinHKMon v1.0.0
Build Date: 2025-10-13
Architecture: x64
```

**Examples**:
```bash
WinHKMon --help
WinHKMon -h
WinHKMon --version
WinHKMon -v
```

---

## Exit Codes

| Code | Meaning | Description |
|------|---------|-------------|
| 0 | Success | Command completed successfully |
| 1 | Invalid Arguments | Invalid command-line arguments or usage error |
| 2 | Runtime Error | API failure, permissions denied, or other runtime error |
| 3 | State File Error | State file corruption (non-fatal but significant) |

**Exit Code Details**:

**Exit 0**: Normal operation
- All requested metrics collected successfully
- Output generated without errors
- State file saved (if applicable)

**Exit 1**: Invalid arguments
- No metrics specified (show help instead)
- Invalid metric name
- Invalid option value (e.g., interval out of range)
- Invalid interface name
- Conflicting options

**Exit 2**: Runtime errors
- PDH initialization failure
- Network interface not found
- Temperature monitoring requires admin (if --temp specified)
- Critical API failure

**Exit 3**: State file issues
- State file corrupted (continues with fresh state)
- Unable to write state file (continues anyway)
- Version mismatch (overwrites with new version)

**Examples**:
```bash
WinHKMon CPU RAM          # Exit 0 (success)
WinHKMon INVALID          # Exit 1 (invalid metric)
WinHKMon TEMP             # Exit 2 if not admin
WinHKMon CPU              # Exit 0 even if state file corrupted (logs warning)
```

---

## Argument Parsing Rules

### Priority Order
1. `--help` or `-h`: Show help and exit (ignore all other arguments)
2. `--version` or `-v`: Show version and exit (ignore all other arguments)
3. Metrics: Parse all uppercase tokens (CPU, RAM, DISK, NET, TEMP, IO, LINE)
4. Interface name: Any quoted string or string without dashes
5. Options: All flags starting with `--` or `-`

### Case Sensitivity
- **Metrics**: Case-insensitive (`CPU`, `cpu`, `Cpu` all valid)
- **Flags**: Case-sensitive (`--format` valid, `--Format` invalid)
- **Interface names**: Case-sensitive (must match exact name)
- **Format values**: Case-insensitive (`json`, `JSON`, `Json` all valid)

### Validation Rules
1. At least one metric required (unless --help or --version)
2. Interval must be numeric in range [0.1, 3600]
3. Format must be one of: `text`, `json`, `csv`
4. Network units must be one of: `bits`, `bytes`
5. Interface name must match existing interface (validated at runtime)

### Error Messages
All error messages follow format: `[ERROR] <description>` to stderr

**Invalid metric**:
```
[ERROR] Invalid metric 'INVALID'. Valid metrics: CPU, RAM, DISK, NET, TEMP, IO
```

**Invalid interval**:
```
[ERROR] Interval must be between 0.1 and 3600 seconds. Got: 5000
```

**Interface not found**:
```
[ERROR] Network interface 'NonExistent' not found.
Available interfaces:
  - Ethernet
  - Wi-Fi
```

**Admin required**:
```
[ERROR] Temperature monitoring requires Administrator privileges.
Please run as Administrator or remove TEMP from metrics.
```

---

## Output Specifications

### Text Output Format

**Compact Mode** (default):
```
CPU:  23.5%  2.4 GHz
RAM:  8192M available (50.0% used)
DISK: 0 C: ↑ 15.3 MB/s  ↓ 2.1 MB/s  (12.5% busy)
NET:  Ethernet ↑ 2.1 Mbps  ↓ 15.3 Mbps  (1 Gbps link)
TEMP: CPU 45°C  (max: 52°C)
```

**Single-Line Mode** (`--line` or `LINE`):
```
CPU:23.5%@2.4GHz RAM:8192M DISK:0:15.3↑2.1↓MB/s NET:Ethernet:2.1↑15.3↓Mbps TEMP:45°C
```

**Continuous Mode** (with `--continuous`):
- Clear screen before each update (optional, based on terminal capability)
- Timestamp each sample
- Update in place for smooth display

### JSON Output Schema

**Schema Version**: 1.0

**Root Object**:
```json
{
  "schemaVersion": "1.0",
  "timestamp": "<ISO 8601 timestamp>",
  "cpu": { ... },           // If CPU requested
  "memory": { ... },        // If RAM requested
  "disks": [ ... ],         // If DISK requested
  "network": [ ... ],       // If NET requested
  "temperature": { ... }    // If TEMP requested
}
```

**CPU Object** (if present):
```json
"cpu": {
  "totalUsagePercent": 23.5,
  "averageFrequencyMhz": 2400,
  "userPercent": 18.2,      // Optional
  "systemPercent": 3.1,     // Optional
  "idlePercent": 78.7,      // Optional
  "cores": [
    {"id": 0, "usagePercent": 45.0, "frequencyMhz": 2800},
    {"id": 1, "usagePercent": 12.0, "frequencyMhz": 2100}
  ]
}
```

**Memory Object** (if present):
```json
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
```

**Full JSON Schema**: See `contracts/json-schema.json`

### CSV Output Format

**Header Row** (first line only):
```csv
timestamp,cpu_percent,cpu_mhz,ram_available_mb,ram_percent,disk_name,disk_read_mbps,disk_write_mbps,net_interface,net_recv_mbps,net_sent_mbps,temp_celsius
```

**Data Rows** (one per sample):
```csv
2025-10-13T14:32:15Z,23.5,2400,8192,50.0,"0 C:",2.1,15.3,Ethernet,15.3,2.1,52
```

**CSV Rules**:
- RFC 4180 compliant
- Fields containing commas or quotes are quoted
- Timestamps in ISO 8601 format
- Numeric values without thousands separators
- Missing values represented as empty string

---

## Environment Variables

**None**: WinHKMon does not read environment variables (security by design).

All configuration via command-line arguments only.

---

## Standard Input/Output

**stdin**: Not used (no interactive input)
**stdout**: All normal output (metrics, JSON, CSV)
**stderr**: Error messages, warnings, debug output (with `--debug` if implemented)

**Piping Support**:
```bash
# Redirect output
WinHKMon CPU RAM > output.txt

# Pipe to other tools
WinHKMon CPU RAM --format json | jq '.cpu.totalUsagePercent'

# Continuous monitoring to file
WinHKMon CPU RAM --continuous --format csv > monitoring.csv
```

---

## Signal Handling

**Windows Signals**:
- **Ctrl+C (SIGINT)**: Gracefully stop continuous monitoring, save state, exit 0
- **Ctrl+Break**: Immediate termination (no state save)

**Example**:
```bash
$ WinHKMon CPU RAM --continuous
CPU:  23.5%  2.4 GHz
RAM:  8192M available
[User presses Ctrl+C]
^C
Stopping... state saved.
$ echo $?
0
```

---

## Compatibility

**Windows Versions**:
- Windows 10 21H2+
- Windows 11 (all versions)
- Windows Server 2022+

**Architectures**:
- x64 (primary)
- ARM64 (secondary)
- x86 (legacy, may be dropped in future)

**Terminal Compatibility**:
- Windows Terminal (full Unicode support)
- Command Prompt (CMD.exe) (ASCII fallback for symbols)
- PowerShell (full Unicode support)
- SSH sessions (terminal capability detection)

---

## Testing Contract

**Unit Tests Must Verify**:
1. All argument combinations parse correctly
2. Invalid arguments produce exit code 1
3. Help/version flags work with any other arguments
4. Interval validation (range checking)
5. Format validation (text/json/csv)
6. Case-insensitive metric parsing
7. Quoted interface name parsing

**Integration Tests Must Verify**:
1. Single-shot mode produces valid output
2. Continuous mode updates correctly
3. JSON output is valid JSON
4. CSV output is RFC 4180 compliant
5. Exit codes match specification
6. Ctrl+C handling works correctly
7. Admin requirement for TEMP enforced

---

## Future Extensions (Out of Scope for v1.0)

The following may be added in future versions but are NOT part of v1.0 contract:

- `--output <file>`: Write to file instead of stdout (use shell redirection)
- `--config <file>`: Read default options from config file
- `--alert <condition>`: Trigger alerts on thresholds
- `--verbose` or `--debug`: Debug output
- `--quiet`: Suppress non-essential output
- `--filter <expr>`: Filter specific cores/disks/interfaces

---

**Document Control:**
- **Completed By**: WinHKMon Design Team
- **Review Date**: 2025-10-13
- **Status**: Complete - CLI contract frozen for v1.0
- **Breaking Changes**: Any changes to this contract require MAJOR version bump per SemVer
- **Related Documents**: [Specification](../spec.md), [Data Model](../specs/data-model.md)

