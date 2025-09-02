#!/bin/bash

# Exit on error
set -e

# 1. Get the input filename
INPUT_FILE="$1"

if [ -z "$INPUT_FILE" ]; then
  echo "❌ Please provide the base filename (e.g., variables.tspp)"
  exit 1
fi

# locate compiler binary
BIN="./build/src/tspp"
if [ ! -x "$BIN" ]; then
  if [ -x "./src/tspp" ]; then
    BIN="./src/tspp"
  fi
fi
if [ ! -x "$BIN" ]; then
  echo "❌ Compiler binary not found at build/src/tspp or src/tspp. Run ./build.sh first."
  exit 1
fi

# compile the input file to LLVM IR
echo "=== Compiling $INPUT_FILE to LLVM IR ==="
"$BIN" "$INPUT_FILE"

# 2. Derive LLVM IR filename
LLVM_IR="${INPUT_FILE}.ll"



# 3. Remove target triple line to avoid clang warning (case/whitespace-insensitive)
sed -i '/^[[:space:]]*target triple[[:space:]]*=/Id' "$LLVM_IR"
# Diagnostic: print any remaining target triple lines
grep -ni 'target triple' "$LLVM_IR" || echo "No target triple line remains in $LLVM_IR"

# 3.5. Ensure executable entrypoint returns int: wrap void @main() if present
# Match lines like: "define dso_local void @main() ..." as well as plain ones.
if grep -Eq '^\s*define\s+.*\bvoid\b.*@main\s*\(' "$LLVM_IR"; then
  # Rename the user's void main to @main_tspp and add a tiny wrapper int main()
  # Only modify the definition line, keep attributes as-is.
  sed -Ei '/^\s*define\s+.*\bvoid\b.*@main\s*\(/ s/@main\s*\(/@main_tspp(/' "$LLVM_IR"
  cat >> "$LLVM_IR" <<'EOF'

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
EOF
fi

# 3.6. If there's no main at all, append a trivial one (avoid duplicate redefinition)
if ! grep -Eq '^\s*define\s+.*@main\s*\(' "$LLVM_IR"; then
  cat >> "$LLVM_IR" <<'EOF'

define i32 @main() {
entry:
  ret i32 0
}
EOF
fi

# 4. Verify the LLVM IR is valid
llvm-as "$LLVM_IR" -o temp.bc



# 4. Compile to native executable
STD_LIB="./build/src/packages/tspp_std/libtspp_std.a"

# Decide sanitizer flags by reading the build configuration
SAN_FLAGS=""
if [ -f "CMakeCache.txt" ]; then
  BUILD_TYPE=$(grep -E '^CMAKE_BUILD_TYPE:' CMakeCache.txt | sed 's/.*=//')
  ENABLE_SAN=$(grep -E '^ENABLE_SANITIZERS:BOOL=' CMakeCache.txt | sed 's/.*=//')
  if [ "$ENABLE_SAN" = "ON" ] || [ "$BUILD_TYPE" = "Debug" ]; then
    SAN_FLAGS="-fsanitize=address"
  fi
fi
if [ -f "$STD_LIB" ]; then
  # Stdlib is built with AddressSanitizer; link with it to satisfy symbols.
  # Suppress harmless LLVM warning about overriding module target triple
  clang "$LLVM_IR" "$STD_LIB" -o temp_exec -lstdc++ -lgc $SAN_FLAGS -no-pie -Wno-override-module
else
  echo "❌ Standard library not found at $STD_LIB. Did you build the project?"
  exit 1
fi




# 5. Run the compiled program with sanitizer settings that avoid non-zero exits
export ASAN_OPTIONS="detect_leaks=0:halt_on_error=0:abort_on_error=0:exitcode=0"
export LSAN_OPTIONS="suppressions=$(pwd)/lsan.supp:report_objects=0:exitcode=0"

# Run and capture output without failing the script if there's no stdout
set +e
PROGRAM_OUTPUT=$(./temp_exec 2>&1)
RUN_EXIT=$?
set -e

# Filter sanitizer noise and print any remaining output (if any)
FILTERED_OUTPUT=$(printf "%s\n" "$PROGRAM_OUTPUT" | grep -v 'LeakSanitizer' | grep -v 'SUMMARY: AddressSanitizer' || true)
if [ -n "$FILTERED_OUTPUT" ]; then
  printf "%s\n" "$FILTERED_OUTPUT"
fi
echo "Exit code: $RUN_EXIT"


# 6. (Optional) Run with LLVM interpreter (will fail for external symbols)
# lli "$LLVM_IR"
# echo "lli exit code: $?"


# 7. Generate assembly (optional)
# llc "$LLVM_IR" -o temp.s
# echo "✅ Assembly generated in temp.s"
# echo "First 20 lines of assembly:"
# head -20 temp.s
