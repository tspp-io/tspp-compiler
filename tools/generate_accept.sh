#!/bin/bash

# === Config ===
HEADER_DIR="src/parser/nodes"
OUTPUT_DIR="$HEADER_DIR/gen_visitors"
OUTPUT_SUFFIX="_accept.cpp"
BASE_VISITOR_INCLUDE="parser/visitors/base_visitor.h"
BASE_CLASSES="Expr|Stmt|Decl|TypeNode|BaseNode"

# === Setup ===
mkdir -p "$OUTPUT_DIR"
echo "🔄 Generating accept() method implementations into: $OUTPUT_DIR"

# === Process each header file ===
find "$HEADER_DIR" -maxdepth 1 -name "*.h" | while read -r header; do
  header_file=$(basename "$header")
  cpp_file="${header_file%.h}${OUTPUT_SUFFIX}"
  output_path="$OUTPUT_DIR/$cpp_file"

  echo "📄 $header_file → gen_visitors/$cpp_file"

  echo "#include \"../$header_file\"" > "$output_path"
  echo "#include \"$BASE_VISITOR_INCLUDE\"" >> "$output_path"
  echo "" >> "$output_path"

  # Extract class names inheriting from AST base types
  grep -Po "class\s+\K\w+(?=\s*:\s*public\s+($BASE_CLASSES))" "$header" | while read -r classname; do
    echo "void $classname::accept(ASTVisitor& visitor) {" >> "$output_path"
    echo "    visitor.visit(*this);" >> "$output_path"
    echo "}" >> "$output_path"
    echo "" >> "$output_path"
  done
done

echo "✅ All accept() methods generated in $OUTPUT_DIR."
