# Do-While Combined Tests Description

This document describes 10 comprehensive tests that combine do-while loops with for loops and continuations in Rho. These tests are designed to verify complex interactions between different control structures and continuation state preservation.

## Test 1: DoWhileWithContinuationCapture
- Creates continuations inside a do-while loop
- Each continuation captures the current loop counter value
- Verifies that continuations preserve their creation-time state

## Test 2: NestedDoWhileForWithSharedContinuations
- Nests a for loop inside a do-while loop
- Creates continuations that capture variables from both loops
- Tests that nested loop variables are properly captured

## Test 3: DoWhileModifyingForContinuations
- Creates continuations in a for loop first
- Uses do-while to iterate through and execute them
- Accumulates results from continuation execution

## Test 4: ForInsideDoWhileWithContinuations
- Places a for loop inside do-while
- Creates and immediately executes continuations
- Tests interaction between loop multipliers and immediate execution

## Test 5: DoWhileConditionalContinuations
- Creates different continuations based on conditions (even/odd)
- Stores them in separate arrays
- Executes only selected continuations afterward

## Test 6: ContinuationBreakFromForInDoWhile
- Uses nested loops to search for a value
- Breaks from both loops when found
- Tests complex control flow with continuations

## Test 7: DoWhileGeneratingForLoopContinuations
- Creates continuations that themselves contain for loops
- Each continuation calculates a sum when executed
- Tests continuations containing control structures

## Test 8: NestedContinuationsInterleaved
- Creates continuations within continuations
- Interleaves do-while and for loop execution
- Tests deep nesting of continuation contexts

## Test 9: DoWhileContinuationAccumulator
- Builds a chain of continuations using do-while
- Each continuation wraps the previous one
- Tests continuation composition and chaining

## Test 10: ComplexControlFlowWithContinuations
- Creates a matrix of continuations using nested loops
- Executes them in specific patterns (diagonal, anti-diagonal)
- Tests complex addressing and execution patterns

## Key Concepts Tested

1. **State Preservation**: Continuations capture loop variable values at creation time
2. **Nested Contexts**: Variables from multiple loop levels are accessible
3. **Dynamic Creation**: Continuations can be created conditionally
4. **Deferred Execution**: Continuations execute with their captured state
5. **Control Flow**: Break/continue interact properly with continuations
6. **Composition**: Continuations can contain other continuations or loops
7. **Collection Management**: Arrays of continuations can be manipulated
8. **Pattern Execution**: Continuations can be executed in various orders

## Expected Behavior

When do-while is fully implemented, these tests verify:
- Do-while executes its body at least once
- Condition is checked after each iteration
- Continuations created in do-while loops capture correct state
- Complex nesting and control flow work as expected
- Memory and state management remain consistent