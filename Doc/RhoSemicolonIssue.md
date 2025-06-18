# Rho Semicolon Issue Analysis

## Problem
The error "Unrecognised ;" was appearing in Rho tests, but investigation shows that semicolons ARE properly recognized by the lexer.

## Root Causes

### 1. Format String Bug in Lexer
The error message was misleading. The lexer had:
```cpp
LexError("Unrecognised %c");
```
But wasn't properly passing the character argument. This has been fixed to:
```cpp
LexError("Unrecognised character");
```

### 2. Incorrect Test Syntax
Many tests are using C-style for loop syntax with parentheses:
```rho
for (i = 0; i < 5; i = i + 1)  // WRONG
```

But Rho uses Python-style syntax WITHOUT parentheses:
```rho
for i = 0; i < 5; i = i + 1    // CORRECT
```

## Rho For Loop Syntax

According to RhoParser.cpp, the correct syntax is:
- Semicolons are REQUIRED between the three parts
- Parentheses are NOT allowed
- Body can use indentation or braces

### Examples:

```rho
// Standard for loop
for i = 0; i < 5; i = i + 1
    print(i)

// With braces
for i = 0; i < 5; i = i + 1 {
    print(i)
}

// Empty sections (semicolons still required)
for ; i < 5; i = i + 1
    print(i)

for i = 0; ; i = i + 1
    if i >= 5
        break
    print(i)

for i = 0; i < 5;
    print(i)
    i = i + 1

// All empty
for ;;
    // infinite loop
    break
```

## Files That Need Fixing

Tests using incorrect parentheses syntax:
- Test/Language/TestRho/TestForLoopSemicolons.cpp
- Test/Language/TestRho/Scripts/ForLoopSemicolonTests.rho
- Many other test files using for loops

## Solution

1. Fixed the lexer error message (already done)
2. Update all test files to use correct Rho syntax (no parentheses in for loops)
3. Consider adding better error messages in the parser when parentheses are encountered after 'for'