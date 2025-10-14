# WinHKMon Release Workflow

Complete guide for creating and publishing releases with automated binary builds.

**Last Updated:** 2025-10-14  
**Workflow Version:** 1.0

---

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Detailed Release Process](#detailed-release-process)
4. [What Happens Automatically](#what-happens-automatically)
5. [Release Checklist](#release-checklist)
6. [Troubleshooting](#troubleshooting)
7. [Examples](#examples)

---

## Overview

WinHKMon uses **automated GitHub Actions workflows** to build and publish releases. When you push a version tag, the system:

✅ Builds optimized binaries  
✅ Runs all 123 tests  
✅ Creates release packages  
✅ Generates checksums  
✅ Publishes to GitHub Releases  
✅ Generates release notes automatically  

**You only need to:**
- Update CHANGELOG.md
- Create and push a version tag

---

## Quick Start

### For Impatient Developers

```bash
# 1. Update changelog
vim CHANGELOG.md
# Add [X.Y.Z] section

# 2. Commit changelog
git add CHANGELOG.md
git commit -m "docs: update changelog for vX.Y.Z"
git push origin main

# 3. Create and push tag
git tag -a vX.Y.Z -m "Release vX.Y.Z: Brief summary"
git push origin vX.Y.Z

# 4. Done! Wait 5-10 minutes
# Check: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/vX.Y.Z
```

---

## Detailed Release Process

### Step 1: Prepare the Release

#### 1.1 Ensure Code is Ready

```bash
# Switch to main branch
git checkout main
git pull origin main

# Verify all tests pass locally (if on Windows)
cd build
ctest -C Release --output-on-failure

# Verify clean working directory
git status
```

#### 1.2 Update Version Numbers

If you have version constants in code, update them:

```cpp
// Example: src/version.h or similar
#define WINHKMON_VERSION "1.0.0"
```

Or in `CMakeLists.txt`:
```cmake
project(WinHKMon VERSION 1.0.0)
```

#### 1.3 Update CHANGELOG.md

**Edit `CHANGELOG.md` and add your version section:**

```markdown
## [1.0.0] - 2025-10-15

### Added
- New feature: Temperature monitoring with LibreHardwareMonitor
- New CLI flag: `--verbose` for detailed output
- ARM64 architecture support

### Changed
- Improved network rate calculation accuracy
- Updated output format for better readability
- Optimized CPU monitoring performance (0.3% overhead)

### Fixed
- Fixed memory leak in continuous monitoring mode
- Fixed race condition in state file management
- Fixed incorrect disk space calculation on large drives

### Deprecated
- Old `--format text` syntax (use `--format json` instead)

### Security
- Updated dependency X to version Y.Z (CVE-XXXX-YYYY)
```

**Format Guide:**
- **Added**: New features
- **Changed**: Changes to existing functionality
- **Deprecated**: Soon-to-be removed features
- **Removed**: Features removed in this version
- **Fixed**: Bug fixes
- **Security**: Vulnerability fixes

#### 1.4 Update Links at Bottom

Add your version to the comparison links at the end of CHANGELOG.md:

```markdown
[unreleased]: https://github.com/YOUR_USERNAME/WinHKMon/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v1.0.0
[0.6.0]: https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v0.6.0
```

### Step 2: Commit Changes

```bash
# Stage changelog and version files
git add CHANGELOG.md
# git add CMakeLists.txt  # if you updated version there
# git add src/version.h   # if you have version constants

# Commit with clear message
git commit -m "docs: update changelog for v1.0.0"

# Push to main
git push origin main
```

### Step 3: Create Release Tag

#### 3.1 Create Annotated Tag

**Good tag message example:**
```bash
git tag -a v1.0.0 -m "Release v1.0.0: First stable release

Key Features:
- Complete system monitoring (CPU, RAM, Disk, Network)
- Temperature monitoring with LibreHardwareMonitor
- Multiple output formats (text, JSON, CSV)
- Continuous and single-shot monitoring modes

Quality:
- 150+ tests passing
- Performance: <0.5% CPU overhead
- Memory footprint: <8 MB
- Windows 10/11 support

Breaking Changes:
- Removed deprecated --old-flag
- Changed default output format

See CHANGELOG.md for full details"
```

**Minimal tag message example:**
```bash
git tag -a v1.0.0 -m "Release v1.0.0: First stable release"
```

**Note:** Your tag message will appear in the release notes!

#### 3.2 Push Tag to Trigger Build

```bash
# Push the tag
git push origin v1.0.0

# Verify tag was pushed
git ls-remote --tags origin
```

### Step 4: Monitor Workflow

#### Via GitHub UI

1. Go to: `https://github.com/YOUR_USERNAME/WinHKMon/actions`
2. Click on the "Release Build" workflow run
3. Watch the progress (~5-10 minutes)

**Workflow Steps:**
1. ⏳ Checkout code at tag
2. ⏳ Setup build environment (CMake, MSVC)
3. ⏳ Configure CMake for Release
4. ⏳ Build binaries
5. ⏳ Run 123 tests (must pass!)
6. ⏳ Verify binary
7. ⏳ Create release package
8. ⏳ Generate checksums
9. ⏳ Generate release notes
10. ⏳ Create GitHub Release
11. ✅ Done!

#### Via GitHub CLI

```bash
# Watch workflow in terminal
gh run watch

# Or list recent runs
gh run list --workflow=release.yml
```

### Step 5: Verify Release

Once the workflow completes:

#### 5.1 Check Release Page

```bash
# Open release in browser
gh release view v1.0.0 --web

# Or view in terminal
gh release view v1.0.0
```

Verify:
- ✅ Release title is correct
- ✅ Release notes include your tag message
- ✅ Release notes include CHANGELOG section
- ✅ Binary ZIP file attached
- ✅ Checksums file attached

#### 5.2 Download and Test Binary

```bash
# Download release
gh release download v1.0.0

# Extract
unzip WinHKMon-v1.0.0-x64.zip

# Test (on Windows)
cd WinHKMon-v1.0.0-x64
./WinHKMon.exe --version
./WinHKMon.exe CPU RAM
```

#### 5.3 Verify Checksums

```bash
# Windows PowerShell
$hash = (Get-FileHash WinHKMon-v1.0.0-x64.zip -Algorithm SHA256).Hash
Write-Host $hash

# Compare with checksums file
cat WinHKMon-v1.0.0-x64-checksums.txt
```

### Step 6: Post-Release

#### 6.1 Update Documentation (Optional)

- Update README.md with new version badges
- Update project documentation if needed
- Update website/docs if applicable

#### 6.2 Announce Release

- GitHub Discussions
- Social media
- Mailing lists
- Discord/Slack

#### 6.3 Close Milestone (If Using)

```bash
gh issue milestone --close v1.0.0
```

---

## What Happens Automatically

When you push a tag matching `v*.*.*`:

### Automated Workflow Tasks

| Task | Duration | Can Fail? |
|------|----------|-----------|
| Code checkout | 10s | No |
| Environment setup | 30s | No |
| CMake configuration | 20s | Yes* |
| Build compilation | 2-3min | Yes* |
| Test execution (123 tests) | 1-2min | Yes* |
| Binary verification | 5s | Yes* |
| Package creation | 10s | No |
| Checksum generation | 5s | No |
| Release note generation | 5s | No |
| GitHub Release creation | 10s | No |

**\* If these fail, the workflow stops and no release is created.**

### What Gets Built

**Release Package:** `WinHKMon-vX.Y.Z-x64.zip`
```
├── WinHKMon.exe          # Optimized Release build
├── README.md             # Latest from main branch
├── LICENSE               # License file
├── CHANGELOG.md          # Complete changelog
└── BUILD_INFO.txt        # Build metadata (commit, date, platform)
```

**Checksums:** `WinHKMon-vX.Y.Z-x64-checksums.txt`
```
SHA256: <64-character-hash>
SHA512: <128-character-hash>
```

### Release Notes Generation

The workflow **intelligently combines** three sources:

**1. Your Tag Message** (if present)
```markdown
### 📝 Release Notes

[Your complete tag annotation here]
```

**2. CHANGELOG.md Section** (if present)
```markdown
### 📋 Changelog

[Extracted section for this version]
```

**3. Standard Template** (always included)
```markdown
### 📦 Release Package
[Package contents, installation, etc.]

### 🖥️ Platform Support
[OS, architecture, compiler info]

### ✅ Quality Assurance
[Test results, performance metrics]
```

---

## Release Checklist

Use this before creating any release:

### Pre-Release Checklist

- [ ] All changes merged to `main`
- [ ] All tests passing locally (if on Windows)
- [ ] No outstanding critical bugs
- [ ] Version numbers updated in code (if applicable)
- [ ] **CHANGELOG.md updated** with version section
- [ ] CHANGELOG.md links updated at bottom
- [ ] Documentation reviewed and updated
- [ ] Breaking changes documented
- [ ] Migration guide written (if needed)

### Constitutional Compliance

- [ ] Library-first architecture maintained
- [ ] No forbidden dependencies added
- [ ] CLI functionality complete
- [ ] Test coverage ≥80%
- [ ] Performance targets met (<1% CPU, <10MB RAM)
- [ ] No security vulnerabilities

### Tagging Checklist

- [ ] Tag follows semantic versioning: `vMAJOR.MINOR.PATCH`
- [ ] Tag message is descriptive (not just "Release vX.Y.Z")
- [ ] Tag message includes key features
- [ ] Tag message mentions breaking changes (if any)
- [ ] Tag pushed to origin

### Post-Release Checklist

- [ ] GitHub Release created successfully
- [ ] Binary ZIP file present
- [ ] Checksums file present
- [ ] Release notes look good
- [ ] Downloaded and tested binary (on Windows)
- [ ] Checksums verified
- [ ] Documentation updated (if needed)
- [ ] Release announced

---

## Troubleshooting

### Workflow Fails: "Tests Failed"

**Symptom:** Workflow fails at test execution step

**Solution:**
1. Check workflow logs for specific test failures
2. Run tests locally: `ctest -C Release --output-on-failure`
3. Fix failing tests
4. Commit fixes
5. Delete and recreate tag:
   ```bash
   git tag -d v1.0.0
   git push origin :refs/tags/v1.0.0
   git tag -a v1.0.0 -m "Release v1.0.0"
   git push origin v1.0.0
   ```

### Workflow Fails: "Build Error"

**Symptom:** Compilation fails

**Solution:**
1. Check for compile errors in workflow logs
2. Verify code compiles locally on Windows
3. Check CMakeLists.txt for issues
4. Fix and recreate tag (same as above)

### Release Notes Missing CHANGELOG Section

**Symptom:** Release notes don't include your changelog

**Possible Causes:**
- CHANGELOG.md not committed before tagging
- Version format mismatch (e.g., `[1.0.0]` vs `## 1.0.0`)
- CHANGELOG.md not following format

**Solution:**
1. Verify CHANGELOG.md format:
   ```markdown
   ## [X.Y.Z] - YYYY-MM-DD
   ```
2. Ensure it's committed and pushed
3. Edit release notes manually on GitHub if needed

### Wrong Tag Pushed

**Symptom:** Accidentally pushed wrong tag

**Solution:**
```bash
# Delete remote tag
git push origin :refs/tags/v1.0.0

# Delete local tag
git tag -d v1.0.0

# Delete GitHub Release (if created)
gh release delete v1.0.0 --yes

# Create correct tag
git tag -a v1.0.0 -m "Correct message"
git push origin v1.0.0
```

### Workflow Not Triggering

**Symptom:** Tag pushed but no workflow runs

**Possible Causes:**
- Tag format doesn't match `v*.*.*` pattern
- Workflow file not on main branch
- GitHub Actions disabled

**Solution:**
1. Verify tag format: `git tag -l`
2. Check workflow exists on GitHub
3. Try manual dispatch (see Building from Existing Tags)

---

## Examples

### Example 1: Patch Release (Bug Fix)

```bash
# Update changelog
cat >> CHANGELOG.md << 'EOF'

## [0.6.1] - 2025-10-16

### Fixed
- Fixed memory leak in continuous monitoring mode
- Fixed crash when no network interfaces available
- Fixed incorrect disk space on drives >2TB
EOF

# Commit
git add CHANGELOG.md
git commit -m "docs: update changelog for v0.6.1"
git push origin main

# Tag
git tag -a v0.6.1 -m "Release v0.6.1: Bug fix release

Fixes:
- Memory leak in continuous mode
- Crash with no network interfaces
- Disk space calculation for large drives"
git push origin v0.6.1
```

### Example 2: Minor Release (New Features)

```bash
# Update changelog
vim CHANGELOG.md
# Add [0.7.0] section with new features

git add CHANGELOG.md
git commit -m "docs: update changelog for v0.7.0"
git push origin main

# Tag with detailed message
git tag -a v0.7.0 -m "Release v0.7.0: Temperature monitoring

New Features:
- Temperature monitoring (CPU, GPU)
- LibreHardwareMonitor integration
- Per-core temperature display
- --verbose flag for detailed output

Requires administrator privileges for TEMP monitoring.

See CHANGELOG.md for full details."
git push origin v0.7.0
```

### Example 3: Major Release (Breaking Changes)

```bash
# Update changelog with breaking changes section
vim CHANGELOG.md
# Add [1.0.0] section

git add CHANGELOG.md
git commit -m "docs: update changelog for v1.0.0"
git push origin main

# Tag with migration notes
git tag -a v1.0.0 -m "Release v1.0.0: First stable release

🎉 WinHKMon 1.0.0 is here!

New Features:
- Complete system monitoring
- Temperature support
- Enhanced output formatting

Breaking Changes:
- Removed --old-syntax flag (use --new-syntax)
- Changed default output format to JSON
- Renamed DISK metric to STORAGE

Migration Guide:
- Replace 'WinHKMon --old-syntax' with 'WinHKMon --new-syntax'
- Add '--format text' to maintain old output
- Update scripts using DISK to use STORAGE

See CHANGELOG.md for complete details."
git push origin v1.0.0
```

---

## Semantic Versioning Guide

WinHKMon follows [Semantic Versioning](https://semver.org/):

**Format:** `vMAJOR.MINOR.PATCH`

### When to Increment

**MAJOR** (v1.0.0 → v2.0.0):
- Breaking changes to CLI arguments
- Breaking changes to library API
- Removed features
- Incompatible behavior changes

**MINOR** (v0.6.0 → v0.7.0):
- New features (backward compatible)
- New CLI flags or options
- New library functions
- Deprecations (not removals)

**PATCH** (v0.6.0 → v0.6.1):
- Bug fixes only
- Security patches
- Documentation updates
- No new features

### Pre-Release Versions

- **Alpha:** `v1.0.0-alpha.1` - Early testing
- **Beta:** `v1.0.0-beta.1` - Feature complete, testing
- **RC:** `v1.0.0-rc.1` - Release candidate

---

## Additional Resources

- [GitHub Actions Workflows](../.github/workflows/README.md) - Workflow documentation
- [CHANGELOG.md](../CHANGELOG.md) - Version history
- [Release Guide](../.github/RELEASE_GUIDE.md) - Comprehensive release guide
- [Keep a Changelog](https://keepachangelog.com/) - Changelog format
- [Semantic Versioning](https://semver.org/) - Versioning standard

---

## Summary

### You Do (Manual)
1. ✏️ Update CHANGELOG.md
2. 💾 Commit and push
3. 🏷️ Create and push tag

### Automation Does (Automatic)
4. 🔨 Build binaries
5. ✅ Run tests
6. 📦 Create packages
7. 🔒 Generate checksums
8. 📝 Generate release notes
9. 🚀 Publish to GitHub Releases

**Time Investment:** ~5-10 minutes of manual work → Professional release with binaries!

---

**Questions or issues?** Check [Troubleshooting](#troubleshooting) or open a GitHub Discussion.

