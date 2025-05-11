#!/bin/bash

# This script modifies the Executor.cpp file to fix the type mismatch issues with Continuations

# Path to Executor.cpp
EXECUTOR_PATH="/home/xian/local/KAI/Source/Library/Executor/Source/Executor.cpp"

# Create backup
cp "$EXECUTOR_PATH" "${EXECUTOR_PATH}.bak"

# Find the Perform function implementation and modify it
SEARCH_PATTERN="Object Executor::Perform(Operation::Type op, Stack<Value> &stack)"
INSERT_AFTER_PERFORM="    // First check if we need to unwrap any Continuations in the stack
    for (int i = 0; i < stack.Size(); ++i) {
        Object &val = stack.At(i);
        if (val.IsType<Continuation>()) {
            Auto<Continuation> cont = val;
            // Check if this is a simple Continuation with a single operation
            if (cont->GetCode()->Size() == 3) {
                // If it's a typical binary operation pattern (arg1, arg2, op)
                Object op = cont->GetCode()->At(2);
                if (op.IsType<Operation>()) {
                    // This is a binary operation - we should unwrap it
                    KAI_TRACE_3() << \"Unwrapping Continuation with binary operation: \" << cont;
                    // Replace the Continuation in the stack with its first argument
                    stack.At(i) = cont->GetCode()->At(0);
                    // Add the second argument
                    if (i+1 < stack.Size()) {
                        // Shift everything down
                        for (int j = stack.Size(); j > i+1; --j) {
                            stack.At(j) = stack.At(j-1);
                        }
                    }
                    stack.At(i+1) = cont->GetCode()->At(1);
                    // Now we'll continue with normal execution and let the operation process
                    // the unwrapped arguments
                }
            }
        }
    }
"

# Apply the change
awk -v pattern="$SEARCH_PATTERN" -v insert="$INSERT_AFTER_PERFORM" '
$0 ~ pattern {
    print $0;
    print "{";
    print insert;
    getline; # Skip the opening brace we just added
} 
{
    print $0;
}' "$EXECUTOR_PATH".bak > "$EXECUTOR_PATH"

echo "Executor.cpp modified with continuation unwrapping logic."
echo "You need to rebuild the project for these changes to take effect."