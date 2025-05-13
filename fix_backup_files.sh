#!/bin/bash

# Create a backup directory if it doesn't exist
mkdir -p /home/xian/local/KAI/Test/Language/TestRho/old_backups

# Move the problematic backup files to the old_backups directory
mv /home/xian/local/KAI/Test/Language/TestRho/backups/*.cpp /home/xian/local/KAI/Test/Language/TestRho/old_backups/

# Update CMakeLists.txt to exclude the backups directory
TMP_FILE=$(mktemp)
cat > $TMP_FILE << 'EOF'
    file(GLOB_RECURSE sources ${source_path}/*.cpp)
    # Exclude backups directory
    list(FILTER sources EXCLUDE REGEX ".*/backups/.*")
EOF

# Replace the relevant line in CMakeLists.txt
sed -i 's|file(GLOB_RECURSE sources ${source_path}/\*.cpp)|'"$(cat $TMP_FILE)"'|' /home/xian/local/KAI/Test/Language/CMakeLists.txt

# Clean up
rm $TMP_FILE

echo "Backup files excluded from build"