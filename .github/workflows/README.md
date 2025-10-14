# GitHub Actions Workflows

This directory contains automated CI/CD workflows for WinHKMon.

## Workflows

### 🚀 Release Build (`release.yml`)

**Triggers:**
- Push tags matching `v*.*.*` (e.g., `v1.0.0`, `v0.5.0-beta`)
- Manual workflow dispatch with version input

**Purpose:** Build and publish release binaries to GitHub Releases

**Process:**
1. ✅ Checkout repository
2. ✅ Setup build environment (CMake, MSVC)
3. ✅ Configure CMake for Release build
4. ✅ Build project with optimizations
5. ✅ Run all 123 tests (must pass)
6. ✅ Verify binary integrity
7. ✅ Create release package with:
   - `WinHKMon.exe`
   - `README.md`
   - `LICENSE`
   - `CHANGELOG.md` (if exists)
   - `BUILD_INFO.txt`
8. ✅ Generate SHA256/SHA512 checksums
9. ✅ Upload artifacts
10. ✅ Create GitHub Release

**Platforms:**
- Windows x64 (primary)
- Windows ARM64 (optional, uncomment in matrix)

**Outputs:**
- `WinHKMon-v{VERSION}-{ARCH}.zip` - Release package
- `WinHKMon-v{VERSION}-{ARCH}-checksums.txt` - Verification checksums

---

### 🔄 Continuous Integration (`ci.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop`
- Manual workflow dispatch

**Purpose:** Validate code quality and functionality on every change

**Jobs:**

#### 1. Build & Test
- Builds in both Debug and Release configurations
- Runs full test suite (123 tests)
- Validates executable functionality
- Tests help and version outputs
- Uploads test results as artifacts

#### 2. Code Quality Checks
- Scans for TODO/FIXME comments
- Detects hardcoded paths
- Verifies namespace usage
- Reports issues (non-blocking)

#### 3. Constitutional Compliance Check
- Verifies library-first architecture
- Checks for forbidden dependencies
- Confirms CLI presence
- Validates test coverage

**Matrix:**
- Build Types: Debug, Release
- Architectures: x64

---

## Usage

### Creating a Release

**Method 1: Git Tag (Recommended)**
```bash
# Tag the release
git tag -a v1.0.0 -m "Release v1.0.0"

# Push tag to trigger workflow
git push origin v1.0.0
```

**Method 2: Manual Workflow Dispatch**
1. Go to Actions → Release Build
2. Click "Run workflow"
3. Enter version (e.g., `v1.0.0`)
4. Click "Run workflow"

### Pre-Release Checklist

Before creating a release tag:

- [ ] All tests passing on main branch
- [ ] CHANGELOG.md updated
- [ ] Version numbers updated in code
- [ ] Documentation reviewed
- [ ] Constitutional compliance verified
- [ ] Performance benchmarks met

### Release Workflow Output

After successful release:
1. Check [Releases page](../../releases) for new release
2. Verify checksums match
3. Test downloaded binary
4. Update documentation if needed

---

## Workflow Configuration

### Secrets Required

None currently. Workflows use:
- `GITHUB_TOKEN` (automatically provided)

### Environment Variables

Set in workflow files:
- `VERSION` - Release version from tag/input
- `ARCH` - Target architecture
- `BUILD_DIR` - Build output directory
- `PACKAGE_NAME` - Release package name

---

## Troubleshooting

### Release Build Fails

**Tests Fail:**
- Check test logs in workflow output
- Verify all tests pass locally on Windows
- Review recent code changes

**Binary Not Found:**
- Check CMake configuration
- Verify build completed successfully
- Check build output directory

**Packaging Fails:**
- Ensure README.md exists in repo root
- Check file paths in packaging step

### CI Build Fails

**Configuration Errors:**
- Verify CMakeLists.txt is valid
- Check MSVC version compatibility
- Review CMake output logs

**Test Timeouts:**
- Tests have 300-second timeout
- Check for infinite loops or hangs
- Review test execution logs

---

## Maintenance

### Updating Workflows

When modifying workflows:
1. Test changes in a fork first
2. Create PR with workflow changes
3. Review workflow syntax
4. Test with workflow_dispatch
5. Monitor first automated run

### Adding ARM64 Support

To enable ARM64 builds, uncomment in `release.yml`:
```yaml
strategy:
  matrix:
    architecture: [x64, ARM64]  # Uncomment this line
    # architecture: [x64]        # Comment this line
```

---

## Related Documentation

- [Implementation Status](../../IMPLEMENTATION_STATUS.md)
- [Specification](../../.specify/features/winhkmon-system-resource-monitoring/spec.md)
- [Build Instructions](../../README.md#build-instructions)
- [Constitution](../../.specify/memory/constitution.md)

---

**Last Updated:** 2025-10-14  
**Maintained By:** WinHKMon Project Team

