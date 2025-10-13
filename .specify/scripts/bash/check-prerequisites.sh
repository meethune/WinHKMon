#!/bin/bash

# check-prerequisites.sh
# Checks which design documents are available and reports feature directory
# Usage: ./check-prerequisites.sh [--json]

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

# Check for available documents
AVAILABLE_DOCS=()
[[ -f "${FEATURE_DIR}/spec.md" ]] && AVAILABLE_DOCS+=("spec.md")
[[ -f "${FEATURE_DIR}/plan.md" ]] && AVAILABLE_DOCS+=("plan.md")
[[ -f "${FEATURE_DIR}/research.md" ]] && AVAILABLE_DOCS+=("research.md")
[[ -f "${FEATURE_DIR}/specs/data-model.md" ]] && AVAILABLE_DOCS+=("data-model.md")
[[ -f "${FEATURE_DIR}/quickstart.md" ]] && AVAILABLE_DOCS+=("quickstart.md")

# Check for contracts directory
if [[ -d "${FEATURE_DIR}/contracts" ]]; then
    CONTRACT_FILES=($(find "${FEATURE_DIR}/contracts" -type f -name "*.md" 2>/dev/null))
    if [[ ${#CONTRACT_FILES[@]} -gt 0 ]]; then
        AVAILABLE_DOCS+=("contracts")
    fi
fi

# Output results
if [[ "$JSON_OUTPUT" == true ]]; then
    # Build JSON array of available docs
    DOCS_JSON="["
    for i in "${!AVAILABLE_DOCS[@]}"; do
        if [[ $i -gt 0 ]]; then
            DOCS_JSON+=","
        fi
        DOCS_JSON+="\"${AVAILABLE_DOCS[$i]}\""
    done
    DOCS_JSON+="]"
    
    cat <<EOF
{
  "status": "success",
  "feature_dir": "${FEATURE_DIR}",
  "available_docs": ${DOCS_JSON},
  "branch": "${BRANCH}"
}
EOF
else
    echo "Feature directory: $FEATURE_DIR"
    echo "Available documents:"
    for doc in "${AVAILABLE_DOCS[@]}"; do
        echo "  - $doc"
    done
fi

exit 0
