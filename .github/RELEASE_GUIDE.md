# Release Guide for WinHKMon

This guide explains how to create releases for WinHKMon, including building from existing tags.

---

## 📋 Table of Contents

1. [Creating a New Release](#creating-a-new-release)
2. [Building from Existing Tag](#building-from-existing-tag)
3. [Manual Binary Build (Local)](#manual-binary-build-local)
4. [Release Checklist](#release-checklist)

---

## 🆕 Creating a New Release

### Step 1: Prepare the Release

```bash
# 1. Ensure you're on main branch with latest changes
git checkout main
git pull origin main

# 2. Update version numbers
# - Update CMakeLists.txt: project(WinHKMon VERSION x.y.z)
# - Update README.md badges
# - Update any version constants in code

# 3. Update CHANGELOG.md (create if doesn't exist)
# Add release notes for this version

# 4. Commit changes
git add .
git commit -m "chore: prepare release v1.0.0"
git push origin main
```

### Step 2: Create and Push Tag

```bash
# Create annotated tag
git tag -a v1.0.0 -m "Release v1.0.0: <brief description>"

# Push tag to trigger automatic build
git push origin v1.0.0
```

### Step 3: Monitor Build

1. Go to GitHub → Actions → Release Build
2. Watch workflow execution (~5-10 minutes)
3. Verify all tests pass (123 tests)
4. Wait for release to be published

### Step 4: Finalize Release

1. Go to Releases page
2. Edit release notes if needed
3. Verify checksums
4. Announce release!

---

## 🔄 Building from Existing Tag

If you already have a tag (e.g., `v0.6.0`) and want to build binaries for it:

### Method 1: GitHub Actions UI (Recommended)

1. **Navigate to Actions:**
   - Go to your repository on GitHub
   - Click "Actions" tab
   - Select "Release Build" from left sidebar

2. **Run Workflow:**
   - Click "Run workflow" button (top right)
   - Fill in the form:
     - **Use workflow from:** Select `main` (or branch containing the tag)
     - **Version tag:** Enter the existing tag, e.g., `v0.6.0`
     - **Create GitHub Release?** ✅ Check this (default: true)
   - Click "Run workflow"

3. **Monitor Progress:**
   - Workflow checks out the specified tag
   - Builds binaries
   - Runs all 123 tests
   - Creates release package
   - Publishes to GitHub Releases

4. **Result:**
   - Release created at: `https://github.com/YOUR_USERNAME/WinHKMon/releases/tag/v0.6.0`
   - Contains: `WinHKMon-v0.6.0-x64.zip` and checksums

### Method 2: GitHub CLI

```bash
# Install GitHub CLI: https://cli.github.com/

# Trigger workflow with existing tag
gh workflow run release.yml \
  --ref main \
  -f version=v0.6.0 \
  -f create_release=true

# Monitor workflow
gh run watch

# View releases
gh release list
```

### Method 3: Just Build Artifacts (No Release)

If you want binaries but not a public release:

1. Follow Method 1 above
2. **Uncheck** "Create GitHub Release?"
3. Download artifacts from workflow run (90-day retention)

---

## 💻 Manual Binary Build (Local)

For testing or local development:

### Prerequisites
- Windows 10/11
- Visual Studio 2022
- CMake 3.20+

### Build Steps

```bash
# Clone and checkout tag
git clone https://github.com/YOUR_USERNAME/WinHKMon.git
cd WinHKMon
git checkout v0.6.0

# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release --parallel

# Run tests
ctest -C Release --output-on-failure

# Binary location
# ./Release/WinHKMon.exe
```

### Package Release

```powershell
# Create release directory
$version = "v0.6.0"
$packageDir = "WinHKMon-$version-x64"
New-Item -ItemType Directory -Path $packageDir

# Copy files
Copy-Item "build/Release/WinHKMon.exe" -Destination $packageDir/
Copy-Item "README.md" -Destination $packageDir/
Copy-Item "LICENSE" -Destination $packageDir/ -ErrorAction SilentlyContinue
Copy-Item "CHANGELOG.md" -Destination $packageDir/ -ErrorAction SilentlyContinue

# Create archive
Compress-Archive -Path $packageDir -DestinationPath "$packageDir.zip"

# Generate checksums
(Get-FileHash "$packageDir.zip" -Algorithm SHA256).Hash | Out-File "$packageDir-SHA256.txt"
(Get-FileHash "$packageDir.zip" -Algorithm SHA512).Hash | Out-File "$packageDir-SHA512.txt"
```

---

## ✅ Release Checklist

Use this checklist before creating any release:

### Pre-Release
- [ ] All changes merged to `main`
- [ ] All tests passing locally (`ctest -C Release`)
- [ ] Version numbers updated:
  - [ ] `CMakeLists.txt` - `project(WinHKMon VERSION x.y.z)`
  - [ ] `README.md` - Version badges
  - [ ] Any version constants in code
- [ ] `CHANGELOG.md` updated with release notes
- [ ] Documentation reviewed and updated
- [ ] No outstanding critical bugs

### Constitutional Compliance
- [ ] Library-first architecture maintained
- [ ] No forbidden dependencies added
- [ ] CLI functionality complete
- [ ] Test coverage ≥80%
- [ ] Performance targets met (<1% CPU, <10MB RAM)

### Build Validation
- [ ] Builds successfully on Windows 10
- [ ] Builds successfully on Windows 11
- [ ] All 123 tests pass
- [ ] Binary runs without errors
- [ ] `--help` and `--version` work

### Release Execution
- [ ] Tag created: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
- [ ] Tag pushed: `git push origin vX.Y.Z`
- [ ] GitHub Actions workflow completes successfully
- [ ] Release published on GitHub
- [ ] Checksums verified
- [ ] Download and test binary

### Post-Release
- [ ] Release announcement drafted
- [ ] Documentation site updated (if applicable)
- [ ] Social media/communication channels notified
- [ ] Close related issues/milestones
- [ ] Update project board/roadmap

---

## 🏷️ Current Tags

You can see existing tags with:

```bash
git tag -l
```

Current tags in this repository:
- `v0.6.0` - Phase 4 Complete (CPU, RAM, DISK, NET)
- `v0.5.0-mvp` - MVP Release (CPU, RAM)
- `v0.1-foundation` - Initial Foundation

---

## 🔧 Troubleshooting

### "Tag already exists"
If you get this error, the tag is already in your local repo:
```bash
# Delete local tag
git tag -d v1.0.0

# Recreate if needed
git tag -a v1.0.0 -m "Release v1.0.0"

# Force push (be careful!)
git push origin v1.0.0 --force
```

### "Tests failed" in workflow
1. Check workflow logs for specific test failures
2. Run tests locally: `ctest -C Release --output-on-failure`
3. Fix issues and create new tag

### "Binary not found" error
- Verify CMakeLists.txt is correct
- Check build logs in workflow
- Ensure all source files are committed

### Workflow not triggering
- Verify tag format matches `v*.*.*` pattern
- Check GitHub Actions is enabled
- Verify workflow file syntax (YAML validation)

---

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Semantic Versioning](https://semver.org/)
- [Keep a Changelog](https://keepachangelog.com/)
- [WinHKMon Specification](./../.specify/features/winhkmon-system-resource-monitoring/spec.md)
- [Implementation Status](../IMPLEMENTATION_STATUS.md)

---

**Last Updated:** 2025-10-14  
**Workflow Version:** 1.0

