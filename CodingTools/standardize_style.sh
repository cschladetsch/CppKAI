#!/bin/bash
# Standardize code style in KAI project
# This script performs automated style fixes on the codebase

# Create a directory for the script if it doesn't exist
mkdir -p "$(dirname "$0")"

# Define color codes for pretty output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}KAI Code Style Standardization Tool${NC}"
echo -e "${YELLOW}This will modify source files to conform to the style guide.${NC}"
echo

# Ask for confirmation to proceed
read -p "Do you want to proceed? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
  echo -e "${RED}Operation cancelled.${NC}"
  exit 1
fi

# Define the root directory (assuming the script is in /CodingTools)
ROOT_DIR=$(dirname "$(cd "$(dirname "$0")" && pwd)")
echo -e "${BLUE}Working in directory: ${ROOT_DIR}${NC}"

# Function to recursively process files
process_files() {
  local dir=$1
  local pattern=$2
  local transform_command=$3
  local description=$4
  
  echo -e "${YELLOW}Applying $description...${NC}"
  
  # Find all files matching the pattern in the directory
  find "$dir" -name "$pattern" -type f | while read file; do
    # Skip files in the Ext directory
    if [[ "$file" == *"/Ext/"* ]]; then
      continue
    fi
    
    # Skip temporary files
    if [[ "$file" == *".tmp"* ]] || [[ "$file" == *"~"* ]]; then
      continue
    }
    
    # Apply the transformation
    eval "$transform_command" "$file"
    
    echo -e "  ${GREEN}Processed:${NC} $file"
  done
}

# 1. Fix member variable underscore convention (prefix to postfix)
fix_underscore_prefix() {
  local file=$1
  # Replace _variable with variable_ but only for member variables
  # This is a simple approach and might need manual review
  sed -i -E 's/\b_([a-zA-Z0-9]+)\b/\1_/g' "$file"
}

# 2. Standardize method case (ensure PascalCase for public methods)
fix_method_case() {
  local file=$1
  # This is a complex transformation that requires manual review
  # Here we're just logging the file for manual inspection
  echo "  ${YELLOW}Needs manual review:${NC} $file"
}

# 3. Fix include headers (ensure proper ordering and style)
fix_includes() {
  local file=$1
  # This could be implemented using a more sophisticated approach
  # For now, we're just ensuring the #pragma once is at the top for header files
  if [[ "$file" == *".h" ]]; then
    if ! grep -q "#pragma once" "$file"; then
      sed -i '1i#pragma once\n' "$file"
      echo "  ${GREEN}Added #pragma once:${NC} $file"
    fi
  fi
}

# 4. Fix NULL to nullptr conversion
fix_null_to_nullptr() {
  local file=$1
  # Replace NULL with nullptr
  sed -i 's/\bNULL\b/nullptr/g' "$file"
  # Replace 0 with nullptr for pointer assignments
  # This is a bit risky and might need manual review for some cases
  sed -i -E 's/([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*0;/\1 = nullptr;/g' "$file"
}

# 5. Fix braces (this is better done with a real formatter)
# Simply logging files that might need attention
identify_brace_style_issues() {
  local file=$1
  # Look for functions that use Allman style (braces on new lines)
  if grep -q "^{" "$file"; then
    echo "  ${YELLOW}Possible brace style issue:${NC} $file"
  fi
}

# Execute the standardization processes
echo -e "${BLUE}Starting automated style standardization...${NC}"

# Process C++ files for various style issues
process_files "$ROOT_DIR/Include" "*.h" fix_underscore_prefix "Member variable underscore convention (headers)"
process_files "$ROOT_DIR/Source" "*.cpp" fix_underscore_prefix "Member variable underscore convention (source)"
process_files "$ROOT_DIR/Include" "*.h" fix_null_to_nullptr "NULL to nullptr conversion (headers)"
process_files "$ROOT_DIR/Source" "*.cpp" fix_null_to_nullptr "NULL to nullptr conversion (source)"
process_files "$ROOT_DIR/Include" "*.h" fix_includes "Include standardization (headers)"
process_files "$ROOT_DIR/Source" "*.cpp" identify_brace_style_issues "Identify brace style issues (source)"
process_files "$ROOT_DIR/Include" "*.h" identify_brace_style_issues "Identify brace style issues (headers)"

echo -e "${GREEN}Style standardization completed!${NC}"
echo
echo -e "${YELLOW}Note: Some changes require manual review. Please check the modified files.${NC}"
echo -e "${YELLOW}For more complex style fixes, consider using a tool like clang-format with a custom configuration.${NC}"