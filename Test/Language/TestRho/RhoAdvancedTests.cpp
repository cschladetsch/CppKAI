#include "TestLangCommon.h"

// Advanced test suite for Rho language features
struct RhoAdvancedTest : kai::TestLangCommon {};

// Test range-based for loop in pi blocks
TEST_F(RhoAdvancedTest, RangeBasedForLoopInPiBlock) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Sum numbers from 1 to 5
    console_.Execute(
        "result = 0\n"
        "for i = 1; i <= 5; i = i + 1\n"
        "    result = result + i\n"
        "result",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              15);  // 1+2+3+4+5 = 15

    exec->GetDataStack()->Clear();

    // Test 2: Factorial using Rho loop
    console_.Execute(
        "factorial = 1\n"
        "for i = 1; i <= 5; i = i + 1\n"
        "    factorial = factorial * i\n"
        "factorial",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              120);  // 5! = 120

    exec->GetDataStack()->Clear();

    // Test 3: Sum 1..10
    console_.Execute(
        "sum = 0\n"
        "for i = 1; i <= 10; i = i + 1\n"
        "    sum = sum + i\n"
        "sum",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              55);  // Sum of 1 to 10 = 55
}

// Test complex Pi blocks with multiple operations
TEST_F(RhoAdvancedTest, ComplexPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Nested arithmetic operations
    console_.Execute("(2 + 3) * 4");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1)
        << "Pi block should leave one value on stack";
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 20);
    exec->GetDataStack()->Clear();

    // Test simple assignment first
    console_.Execute("x = 10");
    ASSERT_EQ(exec->GetDataStack()->Size(), 0)
        << "Simple assignment should not leave values on stack. Stack size: "
        << exec->GetDataStack()->Size();

    // Now test assignment
    console_.Execute("result = (2 + 3) * 4");

    // Assignment should not leave values on stack
    ASSERT_EQ(exec->GetDataStack()->Size(), 0)
        << "Assignment should not leave values on stack. Stack size: "
        << exec->GetDataStack()->Size();

    console_.Execute("result + 0", kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              20);  // (2+3)*4 = 20

    exec->GetDataStack()->Clear();

    // Test 2: Rho expression calculation
    console_.Execute("x = 10 + 5 * 6 - 20");
    console_.Execute("x + 0", kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              20);  // 10 + 30 - 20 = 20

    exec->GetDataStack()->Clear();

    // Test 3: Array operations
    console_.Execute("arr = [1, 2, 3]");
    console_.Execute("arr[2]", kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 3);
}

// Test Rho functions with pi blocks
TEST_F(RhoAdvancedTest, FunctionsWithPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Define a function that sums a range
    console_.Execute(
        "fun sumRange(start, end)\n"
        "    sum = 0\n"
        "    for i = start; i <= end; i = i + 1\n"
        "        sum = sum + i\n"
        "    sum");

    // Call the function
    console_.Execute("result = sumRange(1, 5)");
    console_.Execute("result + 0", kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              15);  // Sum of 1 to 5

    exec->GetDataStack()->Clear();

    // Function factorial in Rho
    console_.Execute(
        "fun factorial(n)\n"
        "    result = 1\n"
        "    for i = 1; i <= n; i = i + 1\n"
        "        result = result * i\n"
        "    result");
    console_.Execute("fact5 = factorial(5)");
    console_.Execute("fact5 + 0", kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 120);  // 5!
}

// Test advanced control flow with pi blocks
TEST_F(RhoAdvancedTest, ControlFlowWithPiBlocks) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: If-else conditions
    console_.Execute(
        "x = 10\n"
        "if x > 5\n"
        "    result = 100\n"
        "else\n"
        "    result = 200\n"
        "result",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              100);  // x > 5 is true

    exec->GetDataStack()->Clear();

    // Test 2: While loop
    console_.Execute(
        "counter = 0\n"
        "while counter < 3\n"
        "    counter = counter + 1\n"
        "counter",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()), 3);
}

// Test error handling in pi blocks
TEST_F(RhoAdvancedTest, PiBlockErrorHandling) {
    console_.SetLanguage(kai::Language::Rho);

    // Test 1: Empty pi block
    EXPECT_THROW(console_.Execute("empty = pi{ }"), kai::Exception::Base);
}

// Test performance-critical operations with pi blocks
TEST_F(RhoAdvancedTest, PerformanceOperations) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Test 1: Large range sum
    console_.Execute(
        "largeSum = 0\n"
        "for i = 1; i <= 100; i = i + 1\n"
        "    largeSum = largeSum + i\n"
        "largeSum",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              5050);  // Sum of 1 to 100

    exec->GetDataStack()->Clear();

    // Test 2: Nested loops
    console_.Execute(
        "nested = 0\n"
        "for i = 1; i <= 10; i = i + 1\n"
        "    for j = 1; j <= 10; j = j + 1\n"
        "        nested = nested + j\n"
        "nested",
        kai::Structure::Program);
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(exec->GetDataStack()->Top()),
              550);  // Sum of 10*(1+2+...+10)
}
