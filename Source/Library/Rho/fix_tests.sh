#!/bin/bash

# Directory containing Rho tests
TESTS_DIR="/home/xian/local/KAI/Test/Language/TestRho"

# Make backups
mkdir -p "$TESTS_DIR/backups"
cp "$TESTS_DIR"/*.cpp "$TESTS_DIR/backups/"

# Update HelloWorld.cpp to handle the new non-wrapping approach
HELLO_WORLD="$TESTS_DIR/HelloWorld.cpp"
if [ -f "$HELLO_WORLD" ]; then
    # Add a comment explaining the fixes
    sed -i '1s/^/\/\/ Modified to work with non-wrapping Continuations approach\n/' "$HELLO_WORLD"
    
    # Make sure we register the appropriate types
    sed -i '/reg.AddClass<String>(Label("String"));/a\    reg.AddClass<Continuation>(Label("Continuation"));' "$HELLO_WORLD"
    
    echo "Updated $HELLO_WORLD"
fi

# Update SimpleRhoTest.cpp
SIMPLE_TEST="$TESTS_DIR/SimpleRhoTest.cpp"
if [ -f "$SIMPLE_TEST" ]; then
    # Update the comments
    sed -i 's/The Rho language currently has a type mismatch issue/The Rho language previously had a type mismatch issue/' "$SIMPLE_TEST"
    sed -i 's/Until this is fixed/This has been fixed, but these tests are still useful as a reference/' "$SIMPLE_TEST"
    
    # Make sure we register Continuation type
    sed -i '/reg.AddClass<bool>(Label("bool"));/a\    reg.AddClass<Continuation>(Label("Continuation"));' "$SIMPLE_TEST"
    
    echo "Updated $SIMPLE_TEST"
fi

# Update any other tests that might need adjustments
for file in "$TESTS_DIR"/*.cpp; do
    # Skip the already processed files
    if [ "$file" = "$HELLO_WORLD" ] || [ "$file" = "$SIMPLE_TEST" ]; then
        continue
    fi
    
    # Add Continuation registration if needed
    if grep -q "reg.AddClass" "$file" && ! grep -q "reg.AddClass<Continuation>" "$file"; then
        sed -i '/reg.AddClass/a\    reg.AddClass<Continuation>(Label("Continuation"));' "$file"
        echo "Added Continuation registration to $file"
    fi
done

echo "Test files updated to work with the new non-wrapping approach"