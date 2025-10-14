# Disk I/O Diagnostic - Verifying Per-Disk vs Total

**Question**: Are disk I/O rates truly per-disk, or is PDH returning totals?

---

## Expected Behavior

PDH `\\PhysicalDisk(*)\\Disk Read Bytes/sec` returns **per-disk** values:

- `\\PhysicalDisk(0 C:)\\Disk Read Bytes/sec` → C: drive activity
- `\\PhysicalDisk(1 D:)\\Disk Write Bytes/sec` → D: drive activity  
- `\\PhysicalDisk(_Total)\\Disk Read Bytes/sec` → **SUM** of all physical disks

**If you have only 1 physical disk:**
```
C: = _Total  (because Total = sum of 1 disk)
```

**If you have 2+ physical disks:**
```
_Total = C: + D: + ...  (Total should be LARGER than individual disks)
```

---

## Verification from User Output

Looking at the terminal output, we see:
```
IO:   C: < 13.4 MB/s  > 7.8 MB/s  (83.5% busy)
IO:   _Total < 13.4 MB/s  > 7.8 MB/s  (83.5% busy)
```

**Observation**: C: and _Total are IDENTICAL

**Possible Explanations:**
1. ✅ **CORRECT**: User has only 1 physical disk (most likely)
2. ❌ **BUG**: We're reading the same counter twice (unlikely given code review)
3. ❌ **PDH ISSUE**: PDH is returning totals for both (unlikely)

---

## How to Verify

### Test 1: Check Disk Count
```cmd
.\Release\WinHKMon.exe IO --format json
```

Look at the `disks` array - how many entries?
```json
{
  "disks": [
    {"deviceName": "C:", ...},
    {"deviceName": "_Total", ...}
  ]
}
```

**Expected**: 2 entries (C: + _Total) if you have 1 physical disk

### Test 2: Multiple Disks
If you have multiple physical drives (C:, D:, etc.), check if they show different values:
```cmd
.\Release\WinHKMon.exe IO --continuous --interval 1
```

**Expected with 2 disks:**
```
IO:   C: < 5.0 MB/s  > 2.0 MB/s
IO:   D: < 1.0 MB/s  > 0.5 MB/s
IO:   _Total < 6.0 MB/s  > 2.5 MB/s  (Total = C: + D:)
```

### Test 3: Targeted Activity
Write to a specific drive and verify only that drive shows activity:

**Write to C: drive:**
```cmd
# In one terminal:
echo test > C:\testfile.txt
copy C:\Windows\System32\cmd.exe C:\largefile.exe

# In another terminal:
.\Release\WinHKMon.exe IO --continuous --interval 1
```

**Expected**: C: should show write activity, D: should show ~0

**Write to D: drive (if you have one):**
```cmd
copy C:\largefile.exe D:\largefile.exe

# Watch:
.\Release\WinHKMon.exe IO --continuous --interval 1
```

**Expected**: D: should show write activity, C: should show minimal

### Test 4: PDH Query Verification
Check what PDH instances are actually available:

```cmd
typeperf -q "PhysicalDisk"
```

**Expected output:**
```
\\Computer\PhysicalDisk(0 C:)\Current Disk Queue Length
\\Computer\PhysicalDisk(0 C:)\% Disk Time
\\Computer\PhysicalDisk(0 C:)\Disk Read Bytes/sec
\\Computer\PhysicalDisk(0 C:)\Disk Write Bytes/sec
\\Computer\PhysicalDisk(_Total)\Current Disk Queue Length
\\Computer\PhysicalDisk(_Total)\% Disk Time
\\Computer\PhysicalDisk(_Total)\Disk Read Bytes/sec
\\Computer\PhysicalDisk(_Total)\Disk Write Bytes/sec
```

**If you have multiple disks:**
```
\\Computer\PhysicalDisk(0 C:)\...
\\Computer\PhysicalDisk(1 D:)\...
\\Computer\PhysicalDisk(_Total)\...
```

---

## Code Verification

Looking at `DiskMonitor.cpp`, each disk gets **separate counter handles**:

```cpp
// Line 176: Loop over counters_ map (one entry per disk)
for (const auto& [diskName, counters] : counters_) {
    // diskName = "0 C:", "1 D:", "_Total", etc.
    
    // Line 183: Get THIS disk's read counter (unique handle)
    PdhGetFormattedCounterValue(counters.bytesRead, ...)
    
    // Line 197: Get THIS disk's write counter (unique handle)
    PdhGetFormattedCounterValue(counters.bytesWritten, ...)
}
```

Each `diskName` has its own:
- `counters.bytesRead` (unique PDH_HCOUNTER)
- `counters.bytesWritten` (unique PDH_HCOUNTER)
- `counters.percentBusy` (unique PDH_HCOUNTER)

The counter paths are:
- `\\PhysicalDisk(0 C:)\\Disk Read Bytes/sec` → C: read handle
- `\\PhysicalDisk(_Total)\\Disk Read Bytes/sec` → _Total read handle

**These are DIFFERENT counters** → Should return different values (IF multiple disks exist)

---

## Diagnosis Result

✅ **VERIFIED: WORKING CORRECTLY**

**User confirmed:** System has **only 1 physical disk** (C:), so:
- C: shows activity for the single physical disk
- _Total = C: (sum of 1 disk)
- **This is CORRECT behavior**

**Evidence:**
1. No D:, E:, etc. drives shown in output
2. C: and _Total values are identical (expected for 1-disk systems)
3. Code review shows separate counter handles per disk
4. Busy percentages also match (further evidence of 1 disk)
5. **User verification:** "there is indeed only one disk"

---

## How to Confirm

**If you have multiple physical drives**, they should show up as separate entries:

```cmd
.\Release\WinHKMon.exe IO
```

**Output with 1 disk (current):**
```
IO:   C: < 13.4 MB/s  > 7.8 MB/s  (83.5% busy)
IO:   _Total < 13.4 MB/s  > 7.8 MB/s  (83.5% busy)
```

**Output with 2 disks (if available):**
```
IO:   C: < 5.0 MB/s  > 2.0 MB/s  (50.0% busy)
IO:   D: < 8.4 MB/s  > 5.8 MB/s  (33.5% busy)
IO:   _Total < 13.4 MB/s  > 7.8 MB/s  (83.5% busy)
```

Note: `_Total` can have different busy % than individual disks because:
- Each disk's busy % is independent (disk can be 100% busy with small I/O)
- _Total busy % is an aggregate measure

---

## Recommendation

**To definitively verify:**

1. **Check disk count in Device Manager:**
   - `Win+X` → Device Manager → Disk drives
   - How many physical drives listed?

2. **Use `typeperf` to see what PDH reports:**
   ```cmd
   typeperf "\\PhysicalDisk(*)\\Disk Read Bytes/sec" -sc 1
   ```
   - How many instances listed? (Should be: `0 C:`, `_Total`, and others if multiple disks)

3. **Add debug output** (temporary):
   In `DiskMonitor::getCurrentStats()`, add after line 178:
   ```cpp
   std::cerr << "[DEBUG] Processing disk: " << diskName 
             << " → " << stats.deviceName << std::endl;
   ```
   
   This will show what instances PDH found:
   ```
   [DEBUG] Processing disk: 0 C: → C:
   [DEBUG] Processing disk: _Total → _Total
   ```

---

## Conclusion

Based on code review and output analysis, **the implementation is correct**. The identical values for C: and _Total are expected if you have only one physical disk.

**PDH DOES provide per-disk data** - we're using separate counter handles for each instance. The `_Total` counter is simply the sum, which equals C: when there's only one disk.

**No bug detected** - this is proper PDH behavior! ✅

