#!/bin/bash

# Exit on error
set -e

# 1. Get the input filename
INPUT_FILE="$1"

if [ -z "$INPUT_FILE" ]; then
  echo "❌ Please provide the base filename (e.g., variables.tspp)"
  exit 1
fi

# compile the input file to LLVM IR
echo "=== Compiling $INPUT_FILE to LLVM IR ==="
./build/src/tspp "$INPUT_FILE"

# 2. Derive LLVM IR filename
LLVM_IR="${INPUT_FILE}.ll"



# 3. Remove target triple line to avoid clang warning (case/whitespace-insensitive)
sed -i '/^[[:space:]]*target triple[[:space:]]*=/Id' "$LLVM_IR"
# Diagnostic: print any remaining target triple lines
grep -ni 'target triple' "$LLVM_IR" || echo "No target triple line remains in $LLVM_IR"

# 4. Verify the LLVM IR is valid
llvm-as "$LLVM_IR" -o temp.bc



# 4. Compile to native executable
STD_LIB="./build/src/packages/tspp_std/libtspp_std.a"
if [ -f "$STD_LIB" ]; then
  clang "$LLVM_IR" "$STD_LIB" -o temp_exec -lstdc++ -fsanitize=address -no-pie
else
  echo "❌ Standard library not found at $STD_LIB. Did you build the project?"
  exit 1
fi


# 5. Run the compiled program
./temp_exec
echo "Exit code: $?"


# 6. (Optional) Run with LLVM interpreter (will fail for external symbols)
# lli "$LLVM_IR"
# echo "lli exit code: $?"


# 7. Generate assembly (optional)
# llc "$LLVM_IR" -o temp.s
# echo "✅ Assembly generated in temp.s"
# echo "First 20 lines of assembly:"
# head -20 temp.s
