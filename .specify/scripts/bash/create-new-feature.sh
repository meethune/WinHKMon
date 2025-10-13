#!/bin/bash

# create-new-feature.sh
# Creates a new feature branch and initializes spec file structure
# Usage: ./create-new-feature.sh [--json] "feature description"

set -e

# Check if --json flag is present
JSON_OUTPUT=false
if [[ "$1" == "--json" ]]; then
    JSON_OUTPUT=true
    shift
fi

# Get feature description from arguments
FEATURE_DESC="$1"
if [[ -z "$FEATURE_DESC" ]]; then
    echo "Error: Feature description required" >&2
    exit 1
fi

# Get repository root (assuming script is in .specify/scripts/bash)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
cd "$REPO_ROOT"

# Generate branch name from feature description
# Convert to lowercase, replace spaces/special chars with hyphens, limit length
BRANCH_NAME=$(echo "$FEATURE_DESC" | \
    tr '[:upper:]' '[:lower:]' | \
    sed 's/[^a-z0-9]/-/g' | \
    sed 's/--*/-/g' | \
    sed 's/^-//;s/-$//' | \
    cut -c1-50)

# Add feature/ prefix
BRANCH_NAME="feature/${BRANCH_NAME}"

# Create feature directory structure
FEATURE_DIR="${REPO_ROOT}/.specify/features/${BRANCH_NAME#feature/}"
SPEC_FILE="${FEATURE_DIR}/spec.md"
PLAN_FILE="${FEATURE_DIR}/plan.md"
TASKS_FILE="${FEATURE_DIR}/tasks.md"
CHECKLIST_DIR="${FEATURE_DIR}/checklists"

# Create directories
mkdir -p "$FEATURE_DIR"
mkdir -p "$CHECKLIST_DIR"

# Initialize spec file with header
cat > "$SPEC_FILE" << 'EOF'
# Feature Specification: [FEATURE_NAME]

**Version:** 0.1
**Last Updated:** [DATE]
**Status:** Draft

[Feature description goes here]

---
EOF

# Create or checkout branch
CURRENT_BRANCH=$(git branch --show-current 2>/dev/null || echo "main")
if git show-ref --verify --quiet "refs/heads/${BRANCH_NAME}"; then
    # Branch exists, check it out
    git checkout "$BRANCH_NAME" 2>/dev/null || true
else
    # Create new branch
    git checkout -b "$BRANCH_NAME" 2>/dev/null || true
fi

# Output results
if [[ "$JSON_OUTPUT" == true ]]; then
    cat <<EOF
{
  "status": "success",
  "branch_name": "${BRANCH_NAME}",
  "spec_file": "${SPEC_FILE}",
  "plan_file": "${PLAN_FILE}",
  "tasks_file": "${TASKS_FILE}",
  "feature_dir": "${FEATURE_DIR}",
  "checklist_dir": "${CHECKLIST_DIR}"
}
EOF
else
    echo "Feature branch created: $BRANCH_NAME"
    echo "Spec file: $SPEC_FILE"
    echo "Plan file: $PLAN_FILE"
    echo "Tasks file: $TASKS_FILE"
fi

exit 0
