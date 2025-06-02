# Closures vs Continuations

## Overview

This document explains the fundamental differences between closures and continuations, with specific focus on how Rho implements continuations rather than traditional closures.

## Closures

A **closure** is a function that captures and retains access to variables from its enclosing lexical scope, even after that scope has exited.

### Key Characteristics of Closures:
- **Variable Capture**: Closures "close over" variables from their defining environment
- **Persistent State**: Captured variables maintain their values between calls
- **Lexical Scoping**: Access is determined by where the function is defined, not where it's called

### Example (in JavaScript):
```javascript
function makeCounter() {
    let count = 0;
    return function() {
        count++;  // Captures and modifies 'count' from outer scope
        return count;
    };
}

const counter = makeCounter();
console.log(counter()); // 1
console.log(counter()); // 2
console.log(counter()); // 3
```

## Continuations

A **continuation** represents the rest of a computation from a given point in the program. It's essentially a snapshot of the program's execution state that can be resumed later.

### Key Characteristics of Continuations:
- **Control Flow Capture**: Captures the entire future of the computation
- **First-class Control**: Can be stored, passed around, and invoked multiple times
- **Stack-based**: Often implemented by saving the call stack state
- **No Implicit Variable Capture**: Values must be explicitly pushed to the stack

### Example (in Rho/Pi style):
```rho
// Rho uses continuations, not closures
x = 10
cont = { x }  // This creates a continuation that will push x when executed
              // But it doesn't capture x's value at definition time

x = 20
cont &        // Executes the continuation, pushes 20 (current value of x)
```

## Key Differences

| Aspect | Closures | Continuations |
|--------|----------|---------------|
| **Purpose** | Encapsulate state and behavior | Capture control flow and execution state |
| **Variable Binding** | Captures variables by reference or value at definition time | Accesses variables at execution time |
| **State Persistence** | Maintains private state between calls | No implicit state; uses stack for data |
| **Typical Use Cases** | Data hiding, factories, callbacks | Control flow manipulation, coroutines |
| **Memory Model** | Heap-allocated environment | Stack-based execution |

## Rho's Implementation

Rho implements **continuations** rather than closures, which has several implications:

### 1. No Variable Capture
```rho
// This doesn't work as expected in Rho
result = []
for i = 0; i < 3; i = i + 1
    f = fun() { i }  // All functions will see final value of i (3)
    result = result + [f]
```

### 2. Stack-Based Execution
```rho
// Rho continuations work with the stack
{ 1 2 + }  // A continuation that pushes 1, pushes 2, then adds
& // Execute: leaves 3 on the stack
```

### 3. Explicit Value Passing
To simulate closure-like behavior in Rho, you must explicitly pass values:
```rho
// Workaround for closure-like behavior
makeAdder = fun(x) {
    return fun(y) { x + y }  // This still won't capture x properly
}

// Better approach: use explicit parameters
add5 = fun(y) { 5 + y }  // Hardcode the value instead of capturing
```

## Practical Implications for Rho Programming

### 1. Loop Variables
Cannot create arrays of functions that capture loop variables:
```rho
// Won't work - all functions see final value
for i = 0; i < 5; i = i + 1
    funcs[i] = fun() { i }
```

### 2. Private State
Cannot create objects with private state using closures:
```rho
// Not possible in Rho
counter = makeCounter()  // No way to hide 'count' variable
```

### 3. Callbacks and Event Handlers
Must pass all needed data explicitly:
```rho
// Instead of capturing context, pass it as parameters
handler = fun(event, context) { 
    // Use event and context parameters
}
```

## Advantages of Rho's Approach

1. **Simplicity**: Easier to reason about - no hidden variable captures
2. **Performance**: Stack-based execution is typically faster
3. **Predictability**: No surprises from captured variables changing
4. **Memory Efficiency**: No need to maintain closure environments

## Disadvantages

1. **Less Expressive**: Some patterns are difficult without closures
2. **More Verbose**: Must explicitly pass all needed values
3. **No Information Hiding**: Cannot easily create private state
4. **Limited Functional Programming**: Many FP patterns rely on closures

## Conclusion

Rho's use of continuations instead of closures reflects its stack-based execution model and its roots in concatenative programming languages. While this limits some programming patterns common in languages with closures, it provides a simpler and more predictable execution model that aligns well with Rho's goals as an embedded scripting language.

For developers coming from closure-based languages, the key adjustment is thinking in terms of explicit stack manipulation and value passing rather than implicit variable capture.