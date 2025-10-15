# Third-Party Libraries

This directory contains third-party dependencies required for WinHKMon.

## LibreHardwareMonitorLib.dll

**Version:** 0.9.3+  
**Purpose:** Temperature sensor monitoring via C++/CLI wrapper  
**License:** MPL 2.0 (Mozilla Public License 2.0)  
**Source:** https://github.com/LibreHardwareMonitor/LibreHardwareMonitor

### What is LibreHardwareMonitor?

LibreHardwareMonitor is a fork of Open Hardware Monitor that provides access to temperature sensors, fan speeds, voltages, load and clock speeds of a computer. It's a .NET library that WinHKMon wraps using C++/CLI to integrate into our native C++ codebase.

### Obtaining the Library

**Option 1: Download from GitHub Releases (Recommended)**

1. Visit: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases
2. Download the latest release (e.g., `LibreHardwareMonitor-v0.9.3.zip`)
3. Extract `LibreHardwareMonitorLib.dll` from the archive
4. Place it in this `lib/` directory

**Option 2: Build from Source**

```bash
git clone https://github.com/LibreHardwareMonitor/LibreHardwareMonitor.git
cd LibreHardwareMonitor
# Open solution in Visual Studio 2022
# Build in Release configuration
# Copy bin/Release/net472/LibreHardwareMonitorLib.dll to WinHKMon/lib/
```

**Option 3: NuGet Package**

```powershell
# In Visual Studio Package Manager Console
Install-Package LibreHardwareMonitorLib
# Copy from packages directory to lib/
```

### Runtime Requirements

- **.NET Framework 4.7.2+** must be installed on the system
- **Administrator privileges** required for hardware sensor access
- **Embedded WinRing0 driver** (automatically extracted by library at runtime)

### Development Requirements

For building WinHKMon with temperature monitoring support:

- **.NET Framework 4.7.2+ SDK** (targeting pack)
  - Install via Visual Studio Installer → Individual Components
  - Search for ".NET Framework 4.7.2 targeting pack" or ".NET Framework 4.8 SDK"
- **C++/CLI support** in Visual Studio 2022
  - Install via: Visual Studio Installer → Desktop development with C++ → C++/CLI support

### License Compatibility

LibreHardwareMonitor is licensed under MPL 2.0, which is compatible with WinHKMon's GPLv3 license. The MPL 2.0 allows:
- Using the library in GPLv3 projects
- Distribution with proprietary or GPL-licensed software
- File-level copyleft (modifications to LibreHardwareMonitor files must be MPL 2.0)

**Important:** We do not modify LibreHardwareMonitor source files; we only wrap the DLL via C++/CLI, so MPL 2.0 file-level copyleft does not extend to WinHKMon code.

### Security Considerations

- LibreHardwareMonitor requires kernel driver (WinRing0) for hardware access
- Driver is embedded and extracted to `%TEMP%\LibreHardwareMonitor\`
- Driver must be signed for secure boot systems
- See `research/temperature-monitoring.md` for detailed security analysis

### Deployment

When distributing WinHKMon:
1. Include `LibreHardwareMonitorLib.dll` in the same directory as `WinHKMon.exe`
2. CMake automatically copies the DLL to build output directories
3. Users must run WinHKMon as Administrator for temperature monitoring

### Alternative Approaches Considered

Before selecting LibreHardwareMonitor, we evaluated:

1. **WMI (Windows Management Instrumentation)**
   - ❌ Unreliable sensor support
   - ❌ Requires manufacturer-specific MOF files

2. **Direct Hardware Access**
   - ❌ Extremely complex (ACPI, SMBus, PCI)
   - ❌ Chipset-specific implementations
   - ❌ High maintenance burden

3. **OpenHardwareMonitor**
   - ❌ Archived/unmaintained since 2016
   - ✅ LibreHardwareMonitor is active fork

**Decision:** LibreHardwareMonitor provides the best balance of reliability, maintenance, and ease of integration.

### References

- LibreHardwareMonitor GitHub: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor
- WinRing0 Driver: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/master/WinRing0
- MPL 2.0 License: https://www.mozilla.org/en-US/MPL/2.0/
- Temperature Monitoring Research: `../.specify/features/winhkmon-system-resource-monitoring/research/temperature-monitoring.md`

