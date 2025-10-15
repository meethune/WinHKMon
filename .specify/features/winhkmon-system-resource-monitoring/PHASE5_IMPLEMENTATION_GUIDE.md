# Phase 5 Implementation Guide: Temperature Monitoring

**Status:** T016 Complete (Research) | T017-T019 Ready for Windows Implementation  
**Date:** 2025-10-14  
**Environment Required:** Windows 10/11 + Visual Studio 2022 + .NET Desktop Development

---

## Overview

This document provides complete implementation guidance for Phase 5 (US3 - Thermal Monitoring). **T016 (Research) is complete**. This guide covers T017-T019 which require Windows environment.

**What's Ready:**
- ✅ Research complete (`research/temperature-monitoring.md`)
- ✅ Header file created (`include/WinHKMonLib/TempMonitor.h`)
- ✅ Stub implementation created (`src/WinHKMonLib/TempMonitor.cpp`)
- ✅ Test file created (`tests/TempMonitorTest.cpp`)
- ✅ CMake configuration prepared (`cmake/TempMonitorConfig.cmake`)
- ✅ Build script created (`scripts/build-with-temp-monitor.bat`)

**What Needs Windows:**
- ⏳ Complete C++/CLI implementation in TempMonitor.cpp
- ⏳ Integrate LibreHardwareMonitor library
- ⏳ Compile and test with MSVC
- ⏳ CLI integration (T018)
- ⏳ Full integration testing (T019)

---

## Prerequisites

### Software Requirements

1. **Windows 10 21H2+ or Windows 11**
2. **Visual Studio 2022**
   - Workload: Desktop development with C++
   - **Required Component**: .NET Framework 4.7.2 SDK (or 4.8)
     - Install via: Visual Studio Installer → Individual Components → ".NET Framework 4.7.2 targeting pack"
   - **Required Component**: C++/CLI support for v143 build tools
3. **CMake 3.20+**
4. **Git** (for version control)

**Why .NET Framework SDK?**  
TempMonitor uses C++/CLI to integrate LibreHardwareMonitor (.NET library) into native C++ code. The test executable becomes a mixed-mode assembly requiring CLR support and .NET Framework reference assemblies.

### Library Requirements

**LibreHardwareMonitorLib.dll**

**Option 1: NuGet (Recommended)**
```powershell
# In Visual Studio Package Manager Console
Install-Package LibreHardwareMonitorLib
```

**Option 2: Manual Download**
1. Visit: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases
2. Download latest release (e.g., LibreHardwareMonitor-net472.zip)
3. Extract `LibreHardwareMonitorLib.dll`
4. Place in `WinHKMon/lib/` directory

**Option 3: Build from Source**
```bash
git clone https://github.com/LibreHardwareMonitor/LibreHardwareMonitor.git
# Build with Visual Studio
# Copy LibreHardwareMonitorLib.dll to WinHKMon/lib/
```

---

## T017: Implement TempMonitor (3 days)

### Step 1: Set Up Environment (30 minutes)

1. **Open Developer Command Prompt for VS 2022**
   ```cmd
   "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
   cd path\to\WinHKMon
   ```

2. **Verify Prerequisites**
   ```powershell
   # Check CMake
   cmake --version
   
   # Check MSVC
   cl.exe
   
   # Check for LibreHardwareMonitorLib.dll
   dir lib\LibreHardwareMonitorLib.dll
   ```

3. **Configure Project**
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

### Step 2: Complete TempMonitor.cpp Implementation (2 days)

**File:** `src/WinHKMonLib/TempMonitor.cpp`

The stub file has extensive comments marked with `// TODO:` and template code marked with `/* Windows C++/CLI Implementation Template: */`.

**Implementation Checklist:**

- [ ] **Uncomment C++/CLI includes**
  ```cpp
  #using <LibreHardwareMonitorLib.dll>
  using namespace LibreHardwareMonitor::Hardware;
  ```

- [ ] **Implement `Impl::initialize()`**
  - Create `Computer^` managed object
  - Enable CPU sensors (`computer_->IsCpuEnabled = true`)
  - Optionally enable GPU (`computer_->IsGpuEnabled = true`)
  - Call `computer_->Open()`
  - Check for available sensors
  - Return appropriate `InitResult`

- [ ] **Implement `Impl::getCurrentStats()`**
  - Update all hardware (`hardware->Update()`)
  - Iterate through CPU sensors
  - Filter by `SensorType::Temperature`
  - Extract temperature values
  - Convert managed strings to `std::string`
  - Calculate min/max/avg
  - Populate `TempStats` structure

- [ ] **Implement `Impl::cleanup()`**
  - Call `computer_->Close()`
  - Release managed resources
  - Handle exceptions gracefully

- [ ] **Update CMakeLists.txt**
  ```cmake
  # Add after WinHKMonLib target
  set_source_files_properties(
      src/WinHKMonLib/TempMonitor.cpp
      PROPERTIES COMPILE_FLAGS "/clr"
  )
  
  # Copy DLL to output
  add_custom_command(TARGET WinHKMon POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/lib/LibreHardwareMonitorLib.dll"
          "$<TARGET_FILE_DIR:WinHKMon>"
  )
  ```

### Step 3: Build and Test (1 day)

**Build:**
```cmd
cd build
cmake --build . --config Release --parallel
```

**Run Tests (as Administrator):**
```cmd
# Right-click Command Prompt → Run as administrator
cd build
ctest -C Release --output-on-failure
```

**Expected Results:**
- All existing tests (123) pass
- New TempMonitor tests:
  - ✅ Admin privilege check works
  - ✅ Initialize succeeds with admin
  - ✅ Temperature values realistic (20-100°C typical)
  - ✅ Multiple reads work
  - ✅ Cleanup doesn't crash

**Run Tests (without Administrator):**
```cmd
# Normal Command Prompt
cd build
ctest -C Release --output-on-failure
```

**Expected Results:**
- Admin-requiring tests skip gracefully
- Non-admin tests pass
- No crashes or errors

**Acceptance Criteria (T017):**
- [ ] All tests pass with admin
- [ ] Graceful failure without admin
- [ ] Temperature values realistic (0-150°C range)
- [ ] No memory leaks (test with continuous operation)
- [ ] Handles missing sensors (test in VM)

---

## T018: CLI Integration (0.5 days)

### Modify main.cpp

**File:** `src/WinHKMon/main.cpp`

**Implementation:**

1. **Add TempMonitor include**
   ```cpp
   #include "TempMonitor.h"
   ```

2. **Initialize TempMonitor when requested**
   ```cpp
   std::unique_ptr<TempMonitor> tempMonitor;
   
   if (options.showTemp) {
       tempMonitor = std::make_unique<TempMonitor>();
       auto result = tempMonitor->initialize();
       
       if (result != TempMonitor::InitResult::SUCCESS) {
           std::cerr << "Temperature monitoring unavailable: "
                     << TempMonitor::getInitResultMessage(result) << std::endl;
           options.showTemp = false;  // Disable for this run
       }
   }
   ```

3. **Collect temperature in monitoring loop**
   ```cpp
   if (options.showTemp && tempMonitor) {
       metrics.temperature = tempMonitor->getCurrentStats();
   }
   ```

4. **Cleanup on exit**
   ```cpp
   if (tempMonitor) {
       tempMonitor->cleanup();
   }
   ```

**Test Commands:**

```cmd
# With admin (should show temperature)
WinHKMon.exe CPU RAM TEMP

# Without admin (should show informative message)
WinHKMon.exe CPU RAM TEMP

# Temperature only
WinHKMon.exe TEMP

# All metrics
WinHKMon.exe CPU RAM DISK IO NET TEMP
```

**Acceptance Criteria (T018):**
- [ ] `WinHKMon TEMP` works with admin
- [ ] Clear error message without admin
- [ ] Temperature in all output formats (text, JSON, CSV)
- [ ] Graceful degradation (other metrics work)
- [ ] No crashes in any scenario

---

## T019: Integration Testing (1 day)

### Test Scenario 1: Temperature Accuracy (Admin Required)

**Setup:**
- Run as administrator
- Have CPU load monitoring tool ready (e.g., HWiNFO, Core Temp)

**Test:**
```cmd
# Idle temperature
WinHKMon.exe CPU TEMP --format json

# Compare with HWiNFO or Core Temp
# Values should be within ±2°C
```

**Acceptance:**
- [ ] Temperatures match other tools (±2-3°C)
- [ ] All CPU cores detected
- [ ] Temperature values realistic for idle (typical 30-60°C)

### Test Scenario 2: Privilege Handling

**Test (without admin):**
```cmd
WinHKMon.exe TEMP
```

**Expected Output:**
```
Temperature monitoring unavailable: Administrator privileges required for temperature monitoring.
Right-click WinHKMon and select 'Run as administrator'
```

**Acceptance:**
- [ ] Clear, actionable error message
- [ ] No crash or exception
- [ ] Program exits gracefully

### Test Scenario 3: Stress Test (Admin Required)

**Setup:**
- Run stress test tool (e.g., Prime95, AIDA64)
- Monitor temperature

**Test:**
```cmd
# In admin command prompt
WinHKMon.exe CPU TEMP --continuous --interval 1

# Run stress test in another window
# Watch temperature increase
```

**Acceptance:**
- [ ] Temperature increases under load
- [ ] Values stay realistic (<100°C normally)
- [ ] No crashes during continuous monitoring
- [ ] Max temperature updates correctly

### Test Scenario 4: Virtual Machine / No Sensors

**Setup:**
- Test in VM or system without sensors

**Test (as admin):**
```cmd
WinHKMon.exe TEMP
```

**Expected:**
```
Temperature monitoring unavailable: No temperature sensors detected on this system.
This is common in virtual machines or older hardware
```

**Acceptance:**
- [ ] Graceful handling of missing sensors
- [ ] Informative message
- [ ] No crash

### Test Scenario 5: 24-Hour Stability (Admin Required)

**Test:**
```cmd
WinHKMon.exe CPU RAM TEMP --continuous --interval 5
```

**Let run for 24 hours**

**Acceptance:**
- [ ] No crashes or hangs
- [ ] No memory leaks (check with Process Monitor)
- [ ] Consistent temperature readings
- [ ] Graceful Ctrl+C shutdown

---

## Validation Checklist

### Functional Testing

- [ ] Temperature monitoring works with admin
- [ ] Graceful degradation without admin
- [ ] Per-core temperatures displayed
- [ ] Min/max/avg calculations correct
- [ ] Values realistic (0-150°C range)
- [ ] Continuous monitoring stable
- [ ] All output formats work (text, JSON, CSV)

### Integration Testing

- [ ] Works with all other metrics (CPU, RAM, DISK, NET)
- [ ] Doesn't break existing functionality
- [ ] Matches other monitoring tools (±3°C)
- [ ] Handles VMs/no sensors gracefully

### Performance Testing

- [ ] CPU overhead <1% with TEMP enabled
- [ ] Memory usage <15 MB with TEMP enabled
- [ ] No memory leaks in 24-hour test
- [ ] Startup time <300ms with TEMP

### Error Handling

- [ ] Clear messages for all error conditions
- [ ] No crashes in any scenario
- [ ] Proper cleanup on exit
- [ ] Handles driver failures gracefully

### Constitutional Compliance

- [ ] Library-first architecture maintained
- [ ] Only Windows APIs + LHM (allowed exception)
- [ ] CLI interface complete
- [ ] Test coverage ≥80%
- [ ] No auto-elevation (user must run as admin)

---

## Troubleshooting

### Issue: "LibreHardwareMonitorLib.dll not found"

**Solution:**
```powershell
# Download and place in lib/
Invoke-WebRequest -Uri "https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/releases/download/v0.9.3/LibreHardwareMonitor-net472.zip" -OutFile lhm.zip
Expand-Archive lhm.zip -DestinationPath lib/lhm
Copy-Item lib/lhm/LibreHardwareMonitorLib.dll lib/
```

### Issue: "C++/CLI compilation failed"

**Solution:**
- Verify .NET Desktop Development workload installed
- Check `/clr` flag in CMakeLists.txt
- Ensure MSVC 2022 compiler

### Issue: "Driver failed to load"

**Solution:**
- Run as administrator
- Check if WinRing0.sys is blocked by antivirus
- Verify driver signing (may need to disable driver signature enforcement)

### Issue: "No temperature sensors detected"

**Possible Causes:**
- Running in VM (expected behavior)
- Old hardware without sensor support
- Sensors not exposed by BIOS

**Verification:**
- Test with HWiNFO or Core Temp
- If those also fail, hardware doesn't support it

---

## Completion Criteria

### T017 Complete When:
- [ ] TempMonitor.cpp fully implemented
- [ ] All unit tests pass (with admin)
- [ ] Graceful degradation works (without admin)
- [ ] No memory leaks
- [ ] Code review complete

### T018 Complete When:
- [ ] CLI integration working
- [ ] All output formats support TEMP
- [ ] Error messages clear and actionable
- [ ] Existing functionality unaffected

### T019 Complete When:
- [ ] All test scenarios pass
- [ ] 24-hour stability test complete
- [ ] Accuracy verified against other tools
- [ ] VM/no-sensor scenarios tested
- [ ] Documentation updated

### CHECKPOINT 5 (Phase 5 Complete) When:
- [ ] All T017-T019 tasks complete
- [ ] All acceptance criteria met
- [ ] 150+ tests passing (123 existing + ~27 new)
- [ ] Performance targets met
- [ ] Constitutional compliance verified
- [ ] Ready for Phase 6 (Polish)

---

## Next Phase Preview

**Phase 6: Polish & Cross-Cutting Concerns**
- T020: Error handling refinement
- T021: Performance optimization (now includes TEMP overhead)
- T022: Documentation updates
- T023: Security review
- T024: Compatibility testing

**Estimated Time:** 1 week after Phase 5 complete

---

## References

- Research: `research/temperature-monitoring.md`
- LibreHardwareMonitor: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor
- Admin Requirements: [HWiNFO Forum](https://www.hwinfo.com/forum/threads/make-hwinfo-runnable-under-a-limited-account-please.9725/)
- LibreHardwareService: https://github.com/epinter/LibreHardwareService

---

**Status:** Ready for Windows Implementation  
**Last Updated:** 2025-10-14  
**Next Action:** Execute T017 on Windows environment

