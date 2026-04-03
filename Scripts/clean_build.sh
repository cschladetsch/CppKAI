#!/bin/bash
# Script to clean CMake artifacts from source tree and set up a proper build directory

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Cleaning CMake artifacts from source tree...${NC}"

# Remove CMake artifacts from root directory
root_artifacts=(
  "CMakeCache.txt"
  "cmake_install.cmake"
  "Makefile"
  "compile_commands.json"
)

for file in "${root_artifacts[@]}"; do
  if [ -f "$file" ]; then
    rm -v "$file"
    echo -e "${GREEN}Removed $file${NC}"
  fi
done

# Remove generated directories
if [ -d "CMakeFiles" ]; then
  rm -rf CMakeFiles
  echo -e "${GREEN}Removed CMakeFiles/ directory${NC}"
fi

# Check for existing build directory
if [ -d "build" ]; then
  echo -e "${YELLOW}Build directory already exists.${NC}"
else
  mkdir -p build
  echo -e "${GREEN}Created build/ directory${NC}"
fi

echo -e "${YELLOW}Setting up build directory...${NC}"

# Create a build script that properly uses the build directory
cat > build.sh << 'EOF'
#!/bin/bash
# Script to build KAI using proper out-of-source build

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Create build directory if it doesn't exist
mkdir -p build
cd build || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. "$@" || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . || { echo -e "${RED}Build failed${NC}"; exit 1; }

echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables can be found in the Bin/ directory${NC}"
EOF

chmod +x build.sh
echo -e "${GREEN}Created build.sh script${NC}"

# Instructions
echo
echo -e "${GREEN}Done! CMake artifacts have been cleaned from the source tree.${NC}"
echo -e "${YELLOW}To build the project:${NC}"
echo -e "  1. Run ${GREEN}./build.sh${NC}"
echo -e "  2. All build artifacts will be in the ${GREEN}build/${NC} directory"
echo -e "  3. Executables will be in the ${GREEN}Bin/${NC} directory"
echo
echo -e "${YELLOW}For more detailed build instructions, see Doc/BUILD.md${NC}"