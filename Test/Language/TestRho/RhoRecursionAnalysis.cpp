#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Deep analysis of recursion issue
struct RhoRecursionAnalysis : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }

    void PrintExecutorState(const std::string& label) {
        auto exec = console_.GetExecutor();
        auto dataStack = exec->GetDataStack();
        auto contextStack = exec->GetContextStack();

        std::cout << "\n=== " << label << " ===" << std::endl;
        std::cout << "Data Stack Size: " << dataStack->Size() << std::endl;
        std::cout << "Context Stack Size: " << contextStack->Size()
                  << std::endl;

        if (dataStack->Size() > 0) {
            std::cout << "Data Stack Top: ";
            try {
                auto top = dataStack->Top();
                if (top.IsType<int>()) {
                    std::cout << kai::ConstDeref<int>(top);
                } else {
                    std::cout << top.GetTypeNumber().ToInt();
                }
            } catch (...) {
                std::cout << "(error reading)";
            }
            std::cout << std::endl;
        }
    }
};

// Test 1: Simplest possible recursion
TEST_F(RhoRecursionAnalysis, SimplestRecursion) {
    // Test the absolute simplest recursive case
    console_.Execute(
        "fun count(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return 1 + count(n - 1)");

    // Test count(1) - should return 1
    data_->Clear();
    PrintExecutorState("Before count(1)");
    console_.Execute("count(1)");
    PrintExecutorState("After count(1)");

    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "count(1) returned: " << result1 << " (expected 1)"
              << std::endl;
    EXPECT_EQ(result1, 1);

    // Test count(2) - should return 2
    data_->Clear();
    PrintExecutorState("Before count(2)");
    console_.Execute("count(2)");
    PrintExecutorState("After count(2)");

    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "count(2) returned: " << result2 << " (expected 2)"
              << std::endl;
    EXPECT_EQ(result2, 2);
}

// Test 2: Analyze what happens with intermediate values
TEST_F(RhoRecursionAnalysis, IntermediateValueAnalysis) {
    // Function that shows intermediate calculations
    console_.Execute(
        "fun debug_sum(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        prev = debug_sum(n - 1)\n"
        "        result = n + prev\n"
        "        return result");

    // Test with manual intermediate checks
    data_->Clear();
    console_.Execute("debug_sum(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "debug_sum(3) returned: " << result << " (expected 6)"
              << std::endl;
    // It returns 5, suggesting one recursive call is lost
}

// Test 3: Test if the issue is with return value handling
TEST_F(RhoRecursionAnalysis, ReturnValueHandling) {
    // Test if return values are properly passed back
    console_.Execute(
        "fun ret_test(n)\n"
        "    if n <= 0\n"
        "        return 100\n"  // Distinctive value
        "    else\n"
        "        return ret_test(n - 1)");

    data_->Clear();
    console_.Execute("ret_test(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "ret_test(3) returned: " << result << " (expected 100)"
              << std::endl;
    EXPECT_EQ(result, 100);  // This should pass if return values work
}

// Test 4: Analyze execution pattern
TEST_F(RhoRecursionAnalysis, ExecutionPattern) {
    // Function that accumulates a pattern to show execution order
    console_.Execute("pattern = 0");

    console_.Execute(
        "fun track_exec(n)\n"
        "    pattern = pattern * 10 + n\n"
        "    if n <= 1\n"
        "        return pattern\n"
        "    else\n"
        "        return track_exec(n - 1)");

    data_->Clear();
    console_.Execute("track_exec(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "Execution pattern: " << result << std::endl;
    std::cout << "Expected pattern: 321 (shows order: 3->2->1)" << std::endl;

    // Check the pattern variable too
    data_->Clear();
    console_.Execute("pattern");
    auto pattern = kai::ConstDeref<int>(data_->Top());
    std::cout << "Pattern variable: " << pattern << std::endl;
}

// Test 5: Test if it's a scoping issue
TEST_F(RhoRecursionAnalysis, ScopingInRecursion) {
    // Each recursive call should have its own 'local' value
    console_.Execute(
        "fun scope_test(n)\n"
        "    local = n * 10\n"
        "    if n <= 1\n"
        "        return local\n"
        "    else\n"
        "        inner = scope_test(n - 1)\n"
        "        return local + inner");

    data_->Clear();
    console_.Execute("scope_test(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "scope_test(3) returned: " << result << std::endl;
    std::cout << "Expected: 30 + 20 + 10 = 60" << std::endl;
}

// Test 6: Direct continuation manipulation test
TEST_F(RhoRecursionAnalysis, ContinuationBehavior) {
    // Test how continuations behave in recursive context
    std::cout << "\n=== CONTINUATION BEHAVIOR TEST ===" << std::endl;

    // Enable tracing for this test
    kai::Process::trace = 1;

    console_.Execute(
        "fun cont_test(n)\n"
        "    if n <= 0\n"
        "        return 999\n"
        "    else\n"
        "        temp = cont_test(n - 1)\n"
        "        return temp");

    data_->Clear();
    std::cout << "\nExecuting cont_test(1)..." << std::endl;
    console_.Execute("cont_test(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\ncont_test(1) returned: " << result << " (expected 999)"
              << std::endl;
    EXPECT_EQ(result, 999);
}

// Test 7: Multiple recursive calls in one function
TEST_F(RhoRecursionAnalysis, MultipleRecursiveCalls) {
    // Test Fibonacci-style multiple recursion
    console_.Execute(
        "fun fib(n)\n"
        "    if n <= 1\n"
        "        return n\n"
        "    else\n"
        "        a = fib(n - 1)\n"
        "        b = fib(n - 2)\n"
        "        return a + b");

    // Test fib(3) = fib(2) + fib(1) = (fib(1) + fib(0)) + 1 = (1 + 0) + 1 = 2
    data_->Clear();
    console_.Execute("fib(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "fib(3) returned: " << result << " (expected 2)" << std::endl;
}