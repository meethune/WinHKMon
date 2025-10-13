# [FEATURE_NAME] Implementation Plan

**Version:** [VERSION]
**Last Updated:** [DATE]
**Status:** Draft
**Feature Branch:** `[BRANCH_NAME]`

This document translates requirements from `spec.md` into technical decisions and implementation strategy, following principles in `.specify/memory/constitution.md`.

---

## Constitution Check

**Pre-Implementation Gates** - Verify before proceeding:

- [ ] **Library-First Architecture** (Principle 1): [YES/NO] - [Explanation]
- [ ] **Native Windows API Mandate** (Principle 2): [YES/NO] - [Explanation]
- [ ] **CLI-First Interface** (Principle 3): [YES/NO] - [Explanation]
- [ ] **Test-First Development** (Principle 4): [YES/NO] - [Explanation]
- [ ] **Simplicity and Anti-Abstraction** (Principle 5): [YES/NO] - [Explanation]
- [ ] **Integration-First Testing** (Principle 6): [YES/NO] - [Explanation]

**Constitutional Compliance Summary:**
[✅/❌] [Summary of compliance status and any exceptions]

---

## I. System Architecture

### High-Level Architecture

```
[ASCII diagram of system components and their relationships]
```

**Rationale:**
- [Principle] - [Explanation]
- [Another principle] - [Explanation]

---

## II. Technology Stack

### Programming Language
- **Language**: [Language and version]
- **Rationale**: [Why chosen]

### Compiler
- **Compiler**: [Compiler and version]
- **Flags**: [Compilation flags]
- **Rationale**: [Why chosen]

### Build System
- **Build Tool**: [Tool and version]
- **Alternative**: [Alternative if any]
- **Rationale**: [Why chosen]

### Testing Framework
- **Unit Tests**: [Framework and version]
- **Rationale**: [Why chosen]

### Dependencies

**Core Library:**
- [Dependency 1]
- [Dependency 2]
- **No other dependencies** (if constitution-compliant)

**Optional Dependencies:**
- [Dependency] - [Purpose]

---

## III. Component Design

### Component 1: [ComponentName]

**Responsibility**: [What this component does]

**Technical Approach**: [High-level approach]

**Implementation**:
```cpp
// Class interface
class [ComponentName] {
public:
    [methods]
private:
    [members]
};
```

**APIs Used:**
- [API function 1] - [Purpose]
- [API function 2] - [Purpose]

**Data Structure:**
```cpp
struct [DataStructure] {
    [fields]
};
```

**Rationale**: [Why this approach]

**Alternative**: [Alternative approach and why not selected]

---

[Repeat Component section for each major component]

---

## IV. Command-Line Interface Design

### Argument Parsing Strategy

**Library**: [Library or approach]

**Data Structure**:
```cpp
struct CliOptions {
    [fields]
};
```

### Usage Examples

```bash
# Example 1
[command]

# Example 2
[command]
```

### Exit Codes

- **0**: Success
- **1**: [Error type]
- **2**: [Error type]

---

## V. Output Formatting

### Text Output Format

**Compact Mode** (default):
```
[Example output]
```

**Single-Line Mode**:
```
[Example output]
```

### JSON Output Format

```json
{
  [Example JSON structure]
}
```

### CSV Output Format

```csv
[Example CSV with headers]
```

**Implementation**:
```cpp
class OutputFormatter {
public:
    [methods]
};
```

---

## VI. [Technical Section]

[Add sections as needed for:
- Delta Calculation Strategy
- Error Handling Strategy
- State Management
- Security Considerations
- Performance Optimization
- etc.]

---

## VII. Testing Strategy

### Unit Tests

**Target Coverage**: [Percentage]% minimum

**Test Framework**: [Framework]

**Test Structure**:
```
tests/
├── [Test1].cpp
├── [Test2].cpp
└── ...
```

**Example Test**:
```cpp
TEST([TestSuite], [TestName]) {
    // Arrange
    // Act
    // Assert
}
```

**Mock Strategy**: [Approach to mocking]

### Integration Tests

**Test Scenarios**:
1. [Scenario 1]: [Description]
2. [Scenario 2]: [Description]

---

## VIII. Build and Deployment

### Build Configuration

**CMakeLists.txt** structure:
```cmake
cmake_minimum_required(VERSION [version])
project([ProjectName] VERSION [version])

[CMake configuration]
```

### Compiler Flags

**Release**: [flags]
**Debug**: [flags]

### Deployment Package

```
[PackageName]-[version]-[arch].zip
├── [executable]
├── [dependencies]
└── [documentation]
```

**Distribution**: [Where and how distributed]

---

## IX. Security Considerations

### Input Validation

[Describe input validation approach]

### File Permissions

[Describe file permission handling]

### Privilege Management

[Describe privilege requirements and handling]

---

## X. Performance Optimization

### Strategies

1. **[Strategy 1]**: [Description]
2. **[Strategy 2]**: [Description]

**Benchmark Targets**:
- [Metric 1]: [Target]
- [Metric 2]: [Target]

---

## XI. Implementation Timeline

**Phase 1: [Phase Name]** ([Duration])
- [Task 1]
- [Task 2]

**Phase 2: [Phase Name]** ([Duration])
- [Task 1]
- [Task 2]

[Continue for all phases]

**Total**: [Total Duration]

---

## XII. Risks and Mitigations

| Risk | Impact | Mitigation | Likelihood |
|------|--------|------------|------------|
| [Risk 1] | [Impact] | [Mitigation] | [Likelihood] |
| [Risk 2] | [Impact] | [Mitigation] | [Likelihood] |

---

## XIII. Success Criteria

Implementation is successful if:

1. **[Criterion 1]**: [Description]
2. **[Criterion 2]**: [Description]
3. **[Criterion 3]**: [Description]

---

## XIV. Open Technical Decisions

**TD-1: [Decision Topic]**
- **Option A**: [Description] - [Pros/Cons]
- **Option B**: [Description] - [Pros/Cons]
- **Recommendation**: [Recommendation]

---

**Document Control:**
- **Author**: [Author/Team]
- **Reviewers**: [Reviewers or TBD]
- **Approval Date**: [Date or Pending]
- **Next Phase**: [Next phase action]
- **Related Documents**:
  - [Specification](./spec.md)
  - [Research Findings](./research.md)
  - [Data Model](./specs/data-model.md)
  - [Contracts](./contracts/)

---

## Template Usage Notes

### Mandatory Sections
- Constitution Check
- System Architecture
- Technology Stack
- Component Design (at least 1 component)
- Testing Strategy
- Build and Deployment
- Success Criteria

### Optional Sections
- Command-Line Interface (if applicable)
- Output Formatting (if applicable)
- Implementation Timeline (recommended)
- Risks and Mitigations (recommended)
- Open Technical Decisions (if any)

### Writing Guidelines
- Focus on HOW to implement requirements from spec.md
- Include technical rationale for all major decisions
- Reference constitution principles when making architectural choices
- Provide code examples for complex components
- Document alternatives considered and why rejected
- Keep consistent with project constitution
