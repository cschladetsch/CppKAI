#!/bin/bash

# Script to rename underscore-prefixed fields to underscore-suffixed fields
# This is a simple approach - for a more comprehensive solution, use a proper C++ parser

# List of field names to replace (from field_mapping.md)
declare -A FIELD_MAP=(
    ["_agents"]="agents_"
    ["_astType"]="astType_"
    ["_black"]="black_"
    ["_callback"]="callback_"
    ["_children"]="children_"
    ["_classes"]="classes_"
    ["_connectionTimeout"]="connectionTimeout_"
    ["_current"]="current_"
    ["_deathRow"]="deathRow_"
    ["_discoveryPort"]="discoveryPort_"
    ["_grey"]="grey_"
    ["_instances"]="instances_"
    ["_isDiscovering"]="isDiscovering_"
    ["_isRunning"]="isRunning_"
    ["_methods"]="methods_"
    ["_name"]="name_"
    ["_netHandle"]="netHandle_"
    ["_nextId"]="nextId_"
    ["_path"]="path_"
    ["_properties"]="properties_"
    ["_proxies"]="proxies_"
    ["_retained"]="retained_"
    ["_servant"]="servant_"
    ["_str"]="str_"
    ["_string"]="string_"
    ["_token"]="token_"
    ["_value"]="value_"
    ["_white"]="white_"
    # Fields from Executor.h (already changed in header)
    ["_continuation"]="continuation_"
    ["_context"]="context_"
    ["_data"]="data_"
    ["_compiler"]="compiler_"
    ["_break"]="break_"
    ["_tree"]="tree_"
    ["_traceLevel"]="traceLevel_"
    ["_stepNumber"]="stepNumber_"
    # Fields from Object.h (already changed in header)
    ["_gcIndex"]="gcIndex_"
    ["_valid"]="valid_"
)

# Directory to search in
SEARCH_DIR="/home/xian/local/KAI"

# Process each field replacement
for OLD_FIELD in "${!FIELD_MAP[@]}"; do
    NEW_FIELD="${FIELD_MAP[$OLD_FIELD]}"
    
    echo "Replacing $OLD_FIELD with $NEW_FIELD..."
    
    # Find all files containing the field and process them
    # This uses grep to find files containing the field name
    grep -l "$OLD_FIELD" --include="*.cpp" --include="*.h" -r "$SEARCH_DIR" | while read -r FILE; do
        echo "  Processing $FILE..."
        
        # Use sed to perform the replacement
        # This sed command replaces the field name while preserving surrounding context
        # It avoids false positives by looking for word boundaries
        sed -i "s/\b$OLD_FIELD\b/$NEW_FIELD/g" "$FILE"
    done
done

echo "Field renaming complete!"