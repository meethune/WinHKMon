#!/bin/bash

# setup-plan.sh
# Sets up the planning phase for a feature by copying templates and preparing directories
# Usage: ./setup-plan.sh [--json]

set -e

# Check if --json flag is present
JSON_OUTPUT=false
if [[ "$1" == "--json" ]]; then
    JSON_OUTPUT=true
fi

# Get repository root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
cd "$REPO_ROOT"

# Get current branch
BRANCH=$(git branch --show-current 2>/dev/null || echo "main")

# Determine feature directory from branch name
if [[ "$BRANCH" == feature/* ]]; then
    FEATURE_NAME="${BRANCH#feature/}"
else
    # Fallback: use the most recently modified feature directory
    FEATURE_NAME=$(ls -t .specify/features/ 2>/dev/null | head -n1)
    if [[ -z "$FEATURE_NAME" ]]; then
        echo "Error: No feature branch or feature directory found" >&2
        exit 1
    fi
fi

FEATURE_DIR="${REPO_ROOT}/.specify/features/${FEATURE_NAME}"
SPEC_FILE="${FEATURE_DIR}/spec.md"
PLAN_FILE="${FEATURE_DIR}/plan.md"
SPECS_DIR="${FEATURE_DIR}/specs"
CONTRACTS_DIR="${FEATURE_DIR}/contracts"
RESEARCH_FILE="${FEATURE_DIR}/research.md"

# Create necessary directories
mkdir -p "$SPECS_DIR"
mkdir -p "$CONTRACTS_DIR"

# Copy plan template if plan doesn't exist
PLAN_TEMPLATE="${REPO_ROOT}/.specify/templates/plan-template.md"
if [[ ! -f "$PLAN_FILE" ]]; then
    if [[ -f "$PLAN_TEMPLATE" ]]; then
        cp "$PLAN_TEMPLATE" "$PLAN_FILE"
    else
        # Create minimal plan file
        cat > "$PLAN_FILE" << 'EOF'
# Implementation Plan

**Version:** 0.1
**Last Updated:** $(date +%Y-%m-%d)
**Status:** Draft

[Plan content goes here]
EOF
    fi
fi

# Output results
if [[ "$JSON_OUTPUT" == true ]]; then
    cat <<EOF
{
  "status": "success",
  "branch": "${BRANCH}",
  "feature_spec": "${SPEC_FILE}",
  "impl_plan": "${PLAN_FILE}",
  "specs_dir": "${SPECS_DIR}",
  "contracts_dir": "${CONTRACTS_DIR}",
  "research_file": "${RESEARCH_FILE}",
  "feature_dir": "${FEATURE_DIR}"
}
EOF
else
    echo "Planning setup complete"
    echo "Feature spec: $SPEC_FILE"
    echo "Implementation plan: $PLAN_FILE"
    echo "Specs directory: $SPECS_DIR"
    echo "Contracts directory: $CONTRACTS_DIR"
fi

exit 0
