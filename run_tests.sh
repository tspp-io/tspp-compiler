#!/bin/bash

# TSPP++ Minimal Test Runner
# Usage: ./run_tests.sh [tests/<subdir>] (optional)

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# Function to run all .tspp files in a directory
run_test_directory() {
  local dir="$1"
  local name=$(basename "$dir")

  echo -e "\n${CYAN}📂 Running tests in: $name${NC}"

  for file in "$dir"/*.tspp; do
    if [[ -f "$file" ]]; then
      echo -e "\n🧪 ${YELLOW}Running: $file${NC}"
      ./test.sh "$file"
    fi
  done
}

# Build step
echo -e "${YELLOW}🔨 Building TSPP++ project...${NC}"
if ./build.sh > /dev/null 2>&1; then
  echo -e "${GREEN}✅ Build successful!${NC}"
else
  echo -e "${RED}❌ Build failed. See build.sh output for details.${NC}"
  exit 1
fi

# If a directory is passed, run just that one
if [[ -n "$1" ]]; then
  if [[ -d "$1" ]]; then
    run_test_directory "$1"
  else
    echo -e "${RED}❌ Error: Directory '$1' not found.${NC}"
    exit 1
  fi
else
  # Run all test directories inside `tests/`
  for dir in tests/*; do
    if [[ -d "$dir" ]]; then
      run_test_directory "$dir"
    fi
  done
fi
