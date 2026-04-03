#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Fixed function tests with correct expectations
struct RhoFunctionTestsFixed : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

// Test simple function calls work
TEST_F(RhoFunctionTestsFixed, SimpleFunctionCalls) {
    // Test a simple function that adds 1
    console_.Execute(
        "fun addOne(n)\n"
        "    return n + 1");

    data_->Clear();
    console_.Execute("addOne(5)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 6);

    // Test a simple function that doubles
    console_.Execute(
        "fun double(n)\n"
        "    return n * 2");

    data_->Clear();
    console_.Execute("double(7)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 14);
}

// Test nested function calls
TEST_F(RhoFunctionTestsFixed, NestedFunctionCalls) {
    // Define functions
    console_.Execute(
        "fun addOne(n)\n"
        "    return n + 1");

    console_.Execute(
        "fun double(n)\n"
        "    return n * 2");

    // Test nested call: double(addOne(5))
    // addOne(5) = 6, double(6) = 12
    data_->Clear();
    console_.Execute("double(addOne(5))");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 12);

    // Test deeper nesting: addOne(double(addOne(3)))
    // addOne(3) = 4, double(4) = 8, addOne(8) = 9
    data_->Clear();
    console_.Execute("addOne(double(addOne(3)))");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 9);
}

// Test the specific case from the failing test
TEST_F(RhoFunctionTestsFixed, DoubleIncrementTest) {
    // Define increment and double functions
    console_.Execute(
        "fun increment(n)\n"
        "    return n + 1");

    console_.Execute(
        "fun double(n)\n"
        "    return n * 2");

    // Test: double(increment(increment(double(2))))
    // Step by step:
    // double(2) = 4
    // increment(4) = 5
    // increment(5) = 6
    // double(6) = 12
    data_->Clear();
    console_.Execute("double(increment(increment(double(2))))");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 12);
}

// Test simple non-recursive functions with multiple parameters
TEST_F(RhoFunctionTestsFixed, MultiParameterFunctions) {
    // Test function with two parameters
    console_.Execute(
        "fun add(a, b)\n"
        "    return a + b");

    data_->Clear();
    console_.Execute("add(3, 4)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 7);

    // Test function with three parameters
    console_.Execute(
        "fun sum3(a, b, c)\n"
        "    return a + b + c");

    data_->Clear();
    console_.Execute("sum3(10, 20, 30)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 60);
}

// Test function local variables
TEST_F(RhoFunctionTestsFixed, FunctionLocalVariables) {
    // Global variable
    console_.Execute("globalX = 100");

    // Function that uses local variable
    console_.Execute(
        "fun testLocal(x)\n"
        "    localY = x * 2\n"
        "    return localY + 1");

    data_->Clear();
    console_.Execute("testLocal(5)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 11);  // 5*2 + 1 = 11

    // Check global is unchanged
    data_->Clear();
    console_.Execute("globalX");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 100);
}

// Test that our fix allows proper continuation
TEST_F(RhoFunctionTestsFixed, ContinuationAfterFunctionCall) {
    console_.Execute(
        "fun addTen(n)\n"
        "    return n + 10");

    // Test that execution continues after function call
    data_->Clear();
    console_.Execute("x = addTen(5) + 3");  // Should be 15 + 3 = 18
    console_.Execute("x");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 18);
}