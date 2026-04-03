# Continuation Control Operations in KAI

This document explains the crucial continuation control operations in KAI's execution model: Suspend, Resume, and Replace. These operations form the foundation for advanced flow control across all KAI languages.

## Introduction to Continuations

Before diving into the control operations, it's important to understand what continuations are in KAI:

A **continuation** is a first-class representation of program execution state. It contains:

- Code to be executed
- References to data
- Information about execution context

Continuations enable powerful control flow patterns, including:

- Non-local returns
- Coroutine-like behavior
- State saving and restoration
- Advanced loop control

## The Context Stack

KAI's execution model maintains two primary stacks:

1. **Data stack**: Holds values being operated on
2. **Context stack**: Manages the nested execution contexts (continuations)

The context stack is what enables advanced continuation operations. When continuations call other continuations, they form a chain of execution contexts on this stack.

## Core Continuation Control Operations

KAI provides three fundamental operations for manipulating the flow of continuations:

### 1. Suspend

**Purpose**: Temporarily pause the current continuation and switch to a different one.

**Behavior**:
- Pushes the current continuation onto the context stack
- Takes a continuation from the data stack and begins executing it
- Preserves the state of the suspended continuation for later resumption

**Syntax in Pi**:
```pi
// Suspends the current continuation and executes 'new_continuation'
new_continuation suspend
```

**Use Cases**:
- Cooperative multitasking
- Implementing generators
- Temporary diversion of control flow
- Pausing and resuming complex algorithms

**Example**:
```pi
// Main task that gets suspended
{
    "Starting main task" trace
    
    // Create a side task
    { "Executing side task" trace } suspend
    
    "Resuming main task" trace
} &
```

### 2. Resume

**Purpose**: Exit the current continuation and return to the most recently suspended continuation.

**Behavior**:
- Sets a break flag to exit from the current continuation
- Execution system then automatically continues with the top continuation from the context stack
- Does not preserve the current continuation

**Syntax in Pi**:
```pi
// Exits the current continuation and resumes the previously suspended one
resume
```

**Use Cases**:
- Early termination of loops
- Implementing exception-like behavior
- Non-local returns from nested functions
- Breaking out of complex algorithms when conditions are met

**Example**:
```pi
// Create a loop that runs from 0 to 9 but exits early at 5
0  // Initial counter
{
    dup 5 >=  // If counter >= 5
    { "Breaking out of loop" trace resume } if  // Exit early
    
    dup trace  // Display current value
    1 +        // Increment counter
    dup 10 <   // Check if still below 10
} while

"This won't execute if loop is exited with resume" trace
```

### 3. Replace

**Purpose**: Completely substitute the current continuation with a different one.

**Behavior**:
- Takes a new continuation from the data stack
- Replaces the current continuation with the new one
- Does not push anything to the context stack
- No way to return to the replaced continuation

**Syntax in Pi**:
```pi
// Replaces the current continuation with 'new_continuation'
new_continuation replace
```

**Use Cases**:
- Tail call optimization
- State machine transitions
- Implementing permanent redirection
- Resource-efficient continuation management

**Example**:
```pi
// A function that delegates to another function
{ 
    // Determine which continuation to use based on input
    dup 10 >
    { drop "Large value handler" trace { large_value_handler } }
    { "Small value handler" trace { small_value_handler } } 
    ife
    
    // Replace current continuation with the selected handler
    replace
    
    "This code never executes" trace
} 'delegate #

// Use the delegate function
15 'delegate @ &
```

## Combining Operations in Control Structures

These operations can be combined to create sophisticated control structures:

### For Loops with Early Exit

```pi
// For loop with early termination when a condition is met
// Pattern: init step limit { body } for

0 1 10 {   // Loop from 0 to 9 in steps of 1
    // If a condition is met, exit the loop
    dup 5 == { "Target found" trace resume } if
    
    // Regular loop body
    dup "Current value:" swap + trace
} for
```

### Coroutine-Like Behavior

```pi
// Two tasks that cooperatively yield control to each other
{ 'task_a #  // Save the continuation for future suspension
    
    "Task A - Step 1" trace
    'task_b @ suspend  // Suspend to task B
    
    "Task A - Step 2" trace
    'task_b @ suspend  // Suspend to task B again
    
    "Task A - Complete" trace
} 'task_a #

{ 'task_b #  // Save the continuation for future suspension
    
    "Task B - Step 1" trace
    'task_a @ suspend  // Suspend back to task A
    
    "Task B - Step 2" trace
    'task_a @ suspend  // Suspend back to task A again
    
    "Task B - Complete" trace
} 'task_b #

// Start the cooperative execution
'task_a @ &
```

### State Machine Implementation

```pi
// A state machine using Replace for transitions
{ 'state_idle #
    "State: IDLE" trace
    
    // Transition to the next state
    'state_working @ replace
} 'state_idle #

{ 'state_working #
    "State: WORKING" trace
    
    // Simulate work
    "Processing data..." trace
    
    // Transition to the done state
    'state_done @ replace
} 'state_working #

{ 'state_done #
    "State: DONE" trace
    
    // We could transition back to idle for a cycle
    // 'state_idle @ replace
} 'state_done #

// Start the state machine
'state_idle @ &
```

## Operations in Different Language Contexts

### In Pi (Stack-Based)

In Pi, these operations are used explicitly with continuations on the stack:

```pi
// Create two tasks
{ 
    "First half of Task A" trace
    { "Task B running" trace } suspend  // Switch to Task B
    "Second half of Task A" trace
} 'task_a #

// Execute Task A
'task_a @ &
```

### In Rho (Infix Syntax)

Rho provides more familiar syntax for using these operations:

```rho
// Create a function with early return using Resume
function search_array(array, target) {
    for (i = 0; i < array.length; i++) {
        if (array[i] == target) {
            // Found the target, return early
            return i;  // Translates to a Resume operation
        }
    }
    return -1;  // Not found
}
```

## Implementation Considerations

When using these operations, keep in mind:

1. **Stack Discipline**: Be careful about what's on the stack when suspending or replacing
2. **Context Stack Management**: The context stack grows with each Suspend operation
3. **Resource Management**: Ensure resources are properly released when using Resume or Replace
4. **Debugging Complexity**: These operations can make execution flow harder to follow

## Best Practices

1. **Use Resume for Early Termination**: Prefer Resume when you need to exit early from a computation
2. **Use Suspend for Temporary Diversion**: Use Suspend when you plan to come back to the current context
3. **Use Replace for State Transitions**: Use Replace when you want to permanently transition to a new execution context
4. **Document Control Flow**: Clearly document when and how these operations are used
5. **Test Thoroughly**: Create tests specifically for these operations in your code

## Common Pitfalls

1. **Forgotten Context Stack**: Failing to resume suspended continuations leads to memory leaks
2. **Stack Corruption**: Not managing the data stack properly during continuation switches
3. **Infinite Loops**: Cyclical suspend patterns without proper termination conditions
4. **Lost Resources**: Replacing continuations without proper resource cleanup
5. **Mixed Operation Confusion**: Mixing Suspend/Resume/Replace operations without clear organization

## Conclusion

The Suspend, Resume, and Replace operations provide powerful control flow capabilities in KAI's continuation-based execution model. Understanding these operations is essential for writing advanced programs in KAI, especially when implementing complex control structures or cooperative multitasking patterns.

For practical examples, explore the [Continuation Control Tests](../Test/Language/TestRho/ContinuationControlTests.cpp) which demonstrate these operations in various scenarios.

## Related Documentation

- [Pi Language Tutorial](PiTutorial.md)
- [Rho Language Tutorial](RhoTutorial.md)
- [Common Language System Architecture](CommonLanguageSystem.md)