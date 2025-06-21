#include "TestLangCommon.h"

// Advanced test suite for Rho language features
struct RhoAdvancedTest : kai::TestLangCommon {};

// Test range-based for loop in pi blocks
TEST_F(RhoAdvancedTest, RangeBasedForLoopInPiBlock) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Sum numbers from 1 to 5 using pi block
    // For loop needs: init, test, update, body
    // This sums 1+2+3+4+5 by accumulating on stack
    console_.Execute(
        "result = pi{ 0 1 { dup 5 <= } { swap over + swap 1 + } while drop }");

    // Get the result from the variable 'result'
    console_.Execute("result");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              15);  // 1+2+3+4+5 = 15

    exec->GetDataStack()->Clear();

    // Test 2: Factorial using pi block
    // For loop needs: init, test, update, body
    // Stack: [product, counter] -> 5! = 1*2*3*4*5
    console_.Execute(
        "factorial = pi{ 1 1 { dup 5 <= } { swap over * swap 1 + } while drop "
        "}");
    console_.Execute("factorial");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              120);  // 5! = 120

    exec->GetDataStack()->Clear();

    // Test 3: Mixed Rho and Pi with for loop
    console_.Execute(
        "sum = 0; sum = sum + pi{ 0 1 { dup 10 <= } { swap over + swap 1 + } "
        "while drop }");
    console_.Execute("sum");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              55);  // Sum of 1 to 10 = 55
}

// Test complex Pi blocks with multiple operations
TEST_F(RhoAdvancedTest, ComplexPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Nested operations in pi block
    // First test just the pi block evaluation
    console_.Execute("pi{ 2 3 + 4 * }");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1)
        << "Pi block should leave one value on stack";
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 20);
    exec->GetDataStack()->Clear();

    // Test simple assignment first
    console_.Execute("x = 10");
    ASSERT_EQ(exec->GetDataStack()->Size(), 0)
        << "Simple assignment should not leave values on stack. Stack size: "
        << exec->GetDataStack()->Size();

    // Now test assignment with pi block
    console_.Execute("result = pi{ 2 3 + 4 * }");

    // Assignment should not leave values on stack
    ASSERT_EQ(exec->GetDataStack()->Size(), 0)
        << "Assignment should not leave values on stack. Stack size: "
        << exec->GetDataStack()->Size();

    console_.Execute("result");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              20);  // (2+3)*4 = 20

    exec->GetDataStack()->Clear();

    // Test 2: Using pi block for calculations in Rho expression
    console_.Execute("x = 10 + pi{ 5 6 * } - 20");
    console_.Execute("x");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              20);  // 10 + 30 - 20 = 20

    exec->GetDataStack()->Clear();

    // Test 3: Pi block with array operations
    console_.Execute("arr = pi{ [1 2 3] }");
    console_.Execute("pi{ arr @ size }");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 3);
}

// Test Rho functions with pi blocks
TEST_F(RhoAdvancedTest, FunctionsWithPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Define a function that uses pi blocks
    console_.Execute(
        "function sumRange(start, end) { return pi{ 0 start @ { dup end @ <= } "
        "{ swap over + swap 1 + } while drop "
        "}; }");

    // Call the function
    console_.Execute("result = sumRange(1, 5)");
    console_.Execute("result");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              15);  // Sum of 1 to 5

    exec->GetDataStack()->Clear();

    // Function with mixed Rho and Pi
    console_.Execute(
        "function factorial(n) { return pi{ 1 1 { dup n @ <= } { swap over * "
        "swap 1 + } while drop }; }");
    console_.Execute("fact5 = factorial(5)");
    console_.Execute("fact5");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 120);  // 5!
}

// Test advanced control flow with pi blocks
TEST_F(RhoAdvancedTest, ControlFlowWithPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: If-else with pi block conditions
    console_.Execute(
        "x = 10; result = if (pi{ x @ 5 > }) { pi{ 100 } } else { pi{ 200 } }");
    console_.Execute("result");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              100);  // x > 5 is true

    exec->GetDataStack()->Clear();

    // Test 2: While loop using pi blocks
    console_.Execute(
        "counter = 0; while (pi{ counter @ 3 < }) { counter = pi{ counter @ 1 "
        "+ }; }");
    console_.Execute("counter");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 3);
}

// Test error handling in pi blocks
TEST_F(RhoAdvancedTest, PiBlockErrorHandling) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Empty pi block
    console_.Execute("empty = pi{ }");
    EXPECT_TRUE(exec->GetDataStack()->Empty());

    // Test 2: Pi block leaving multiple values on stack
    console_.Execute("multi = pi{ 1 2 3 }");
    console_.Execute("multi");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              3);  // Last value
}

// Test performance-critical operations with pi blocks
TEST_F(RhoAdvancedTest, PerformanceOperations) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Large range sum
    console_.Execute(
        "largeSum = pi{ 0 1 { dup 100 <= } { swap over + swap 1 + } while drop "
        "}");
    console_.Execute("largeSum");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              5050);  // Sum of 1 to 100

    exec->GetDataStack()->Clear();

    // Test 2: Nested loops in pi block
    console_.Execute(
        "nested = pi{ 0 1 { dup 10 <= } { 0 1 { dup 10 <= } { swap over + swap "
        "1 + } while drop + 1 + } while drop }");
    console_.Execute("nested");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              550);  // Sum of 10*(1+2+...+10)
}