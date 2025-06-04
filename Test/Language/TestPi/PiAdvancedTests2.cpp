#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Additional advanced Pi tests
struct PiAdvancedTests2 : kai::TestLangCommon {};

// Test 1: Complex stack rotations
TEST_F(PiAdvancedTests2, ComplexStackRotations) {
    RunPiCode("1 2 3 4 5 rot rot2 swap over");
    // Initial: 1 2 3 4 5
    // rot: 1 2 3 5 4
    // rot2: 1 2 5 4 3
    // swap: 1 2 5 3 4
    // over: 1 2 5 3 4 3
    auto stack = GetDataStack();
    ASSERT_EQ(stack->Size(), 6);
    ASSERT_EQ(stack->At(0).GetInt(), 1);
    ASSERT_EQ(stack->At(5).GetInt(), 3);
}

// Test 2: Nested block execution
TEST_F(PiAdvancedTests2, NestedBlockExecution) {
    RunPiCode("{ { 42 } & } & ");
    ExpectStackSize(1);
    ExpectStackTop<int>(42);
}

// Test 3: Block manipulation
TEST_F(PiAdvancedTests2, BlockManipulation) {
    RunPiCode("{ 1 + } 'inc # 5 inc & ");
    ExpectStackSize(1);
    ExpectStackTop<int>(6);
}

// Test 4: Advanced conditionals
TEST_F(PiAdvancedTests2, AdvancedConditionals) {
    RunPiCode("1 1 == { \"equal\" } { \"not equal\" } if & ");
    ExpectStackSize(1);
    ExpectStackTop<kai::String>("equal");
}

// Test 5: Stack depth operations
TEST_F(PiAdvancedTests2, StackDepthOperations) {
    RunPiCode("1 2 3 4 5 depth");
    ExpectStackSize(6);
    ExpectStackTop<int>(5);  // depth returns count of items before it
}

// Test 6: Type checking operations
TEST_F(PiAdvancedTests2, TypeCheckingOperations) {
    RunPiCode("42 type");
    ExpectStackSize(2);
    // Top should be type identifier for int

    RunPiCode("clear \"hello\" type");
    ExpectStackSize(2);
    // Top should be type identifier for string
}

// Test 7: Array slicing
TEST_F(PiAdvancedTests2, ArraySlicing) {
    RunPiCode("[] 1 + 2 + 3 + 4 + 5 + ");  // [1,2,3,4,5]
    ExpectStackSize(1);
    auto arr = GetStackTop();
    ASSERT_TRUE(arr.IsType<kai::Vector>());
    ASSERT_EQ(arr.GetClass()->Size(arr), 5);
}

// Test 8: Function composition
TEST_F(PiAdvancedTests2, FunctionComposition) {
    RunPiCode("{ 2 * } 'double # { 1 + } 'inc # 3 inc & double & ");
    // 3 -> 4 -> 8
    ExpectStackSize(1);
    ExpectStackTop<int>(8);
}

// Test 9: Error handling patterns
TEST_F(PiAdvancedTests2, ErrorHandlingPatterns) {
    // Test safe division
    RunPiCode(
        "{ dup 0 == { drop drop \"div by zero\" } { / } if } 'safediv # "
        "10 5 safediv & ");
    ExpectStackSize(1);
    ExpectStackTop<int>(2);

    RunPiCode("clear 10 0 safediv & ");
    ExpectStackSize(1);
    ExpectStackTop<kai::String>("div by zero");
}

// Test 10: Advanced looping constructs
TEST_F(PiAdvancedTests2, AdvancedLooping) {
    // Sum of squares from 1 to 5
    RunPiCode(
        "0 { dup * + } 'addsquare # "
        "1 addsquare & 2 addsquare & 3 addsquare & 4 addsquare & 5 addsquare "
        "& ");
    // 0 + 1^2 + 2^2 + 3^2 + 4^2 + 5^2 = 0 + 1 + 4 + 9 + 16 + 25 = 55
    ExpectStackSize(1);
    ExpectStackTop<int>(55);
}