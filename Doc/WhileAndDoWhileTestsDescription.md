# While and Do-While Loop Tests Description

This document describes 15 comprehensive tests for while and do-while loops in Rho, covering various scenarios and edge cases.

## Basic Loop Tests

### Test 1: WhileLoopSum
- Tests basic while loop functionality
- Sums numbers from 1 to 5
- Verifies correct accumulation and loop termination

### Test 2: DoWhileLoopSum
- Tests basic do-while loop functionality
- Sums numbers from 1 to 5
- Verifies same result as while loop for this case

## Edge Case Tests

### Test 3: WhileLoopNeverExecutes
- Tests while loop with initially false condition
- Verifies that body never executes
- Counter remains at initial value

### Test 4: DoWhileExecutesOnceWithFalse
- Tests do-while with false condition
- Verifies body executes exactly once
- Demonstrates key difference from while loop

## Nested Loop Tests

### Test 5: NestedWhileLoops
- Tests nested while loops
- Calculates sum of multiplication table
- Verifies correct handling of inner and outer loop variables

### Test 6: NestedDoWhileLoops
- Tests nested do-while loops
- Counts total iterations
- Verifies both loops execute correctly

## Control Flow Tests

### Test 7: WhileLoopWithBreak
- Tests break statement in while loop
- Loop exits when counter reaches 5
- Verifies early termination works correctly

### Test 8: DoWhileWithContinue
- Tests continue statement in do-while loop
- Skips even numbers
- Sums only odd numbers from 1 to 5

## Mixed Usage Tests

### Test 9: MixedWhileAndDoWhile
- Uses both while and do-while in same program
- Accumulates values from both loops
- Tests that both loop types work in same context

### Test 10: ComplexConditionsInLoops
- Tests compound boolean conditions (AND/OR)
- While loop with `x < 5 && y > 5`
- Do-while loop with `a < 3 || b < 4`

## Continuation Integration Tests

### Test 11: WhileLoopWithContinuations
- Creates continuations inside while loop
- Each continuation captures loop variable
- Executes all continuations and sums results

### Test 12: DoWhileLoopWithContinuations
- Creates continuations inside do-while loop
- Each continuation computes square of loop variable
- Demonstrates state preservation in continuations

## Algorithm Implementation Tests

### Test 13: FactorialUsingWhile
- Implements factorial calculation using while loop
- Calculates 5! = 120
- Tests multiplication accumulation pattern

### Test 14: FibonacciUsingDoWhile
- Implements Fibonacci sequence using do-while
- Calculates 8th Fibonacci number (21)
- Tests variable swapping and accumulation

### Test 15: StringConcatenationInLoops
- Tests string operations in loops
- Builds strings using both while and do-while
- Concatenates results to form "AAABB"

## Key Concepts Tested

1. **Loop Initialization**: Proper setup of loop variables
2. **Condition Evaluation**: Both simple and complex boolean expressions
3. **Body Execution**: Single and multi-statement bodies
4. **State Modification**: Updating loop variables correctly
5. **Termination**: Proper exit conditions
6. **Control Flow**: Break and continue statements
7. **Nesting**: Multiple levels of loops
8. **Integration**: Loops with continuations and other language features
9. **Edge Cases**: Zero iterations, single iteration, etc.
10. **Real Algorithms**: Practical use cases like factorial and Fibonacci

## Expected Behavior

- **While loops**: Check condition before each iteration
- **Do-while loops**: Execute body at least once, then check condition
- Both loop types should handle:
  - Variable scoping correctly
  - Complex conditions
  - Nested structures
  - Control flow statements
  - Integration with continuations