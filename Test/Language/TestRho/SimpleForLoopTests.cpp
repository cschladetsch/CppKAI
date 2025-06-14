#include "TestLangCommon.h"

// Simple, clean tests for Rho for loops
struct RhoForLoopTest : kai::TestLangCommon {};

TEST_F(RhoForLoopTest, BasicIncrement) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Basic for loop to sum 0 to 4
    const char* code = R"(
        sum = 0
        for i = 0; i < 5; i = i + 1
            sum = sum + i
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);  // 0+1+2+3+4
}

TEST_F(RhoForLoopTest, Decrement) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Count down from 5 to 1
    const char* code = R"(
        count = 0
        for i = 5; i > 0; i = i - 1
            count = count + 1
        count
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(RhoForLoopTest, MultiplicationTable) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Calculate 7 * 4 using repeated addition
    const char* code = R"(
        result = 0
        for i = 0; i < 4; i = i + 1
            result = result + 7
        result
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 28);
}

TEST_F(RhoForLoopTest, BreakStatement) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Break when sum exceeds 10
    const char* code = R"(
        sum = 0
        for i = 1; i <= 10; i = i + 1
            sum = sum + i
            if sum > 10
                break
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5
}

TEST_F(RhoForLoopTest, ContinueStatement) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Sum only odd numbers using continue
    const char* code = R"(
        sum = 0
        for i = 1; i <= 6; i = i + 1
            if i % 2 == 0
                continue
            sum = sum + i
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 9);  // 1+3+5
}

TEST_F(RhoForLoopTest, NestedLoops) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Simple nested loops
    const char* code = R"(
        sum = 0
        for i = 1; i <= 3; i = i + 1
            for j = 1; j <= 2; j = j + 1
                sum = sum + (i * j)
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    // (1*1 + 1*2) + (2*1 + 2*2) + (3*1 + 3*2) = 3 + 6 + 9 = 18
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 18);
}

TEST_F(RhoForLoopTest, EmptyBody) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Empty body - just counts iterations
    const char* code = R"(
        counter = 0
        for i = 0; i < 3; i = i + 1
            // Empty body
        counter = i  // i should be 3 after loop
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(RhoForLoopTest, ComplexIncrement) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Geometric progression: 1, 2, 4, 8
    const char* code = R"(
        sum = 0
        for i = 1; i < 10; i = i * 2
            sum = sum + i
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+4+8
}

TEST_F(RhoForLoopTest, ArrayIteration) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Build and sum array
    const char* code = R"(
        arr = [10, 20, 30, 40]
        sum = 0
        for i = 0; i < 4; i = i + 1
            sum = sum + arr[i]
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

TEST_F(RhoForLoopTest, NoIterations) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Condition false from start
    const char* code = R"(
        x = 42
        for i = 10; i < 5; i = i + 1
            x = 0  // Should never execute
        x
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST_F(RhoForLoopTest, MultipleStatements) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Multiple statements in loop body
    const char* code = R"(
        a = 0
        b = 0
        for i = 1; i <= 3; i = i + 1
            a = a + i
            b = b + (i * i)
        a + b  // 6 + 14 = 20
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);
}

TEST_F(RhoForLoopTest, FunctionCallInLoop) {
    console_.SetLanguage(kai::Language::Rho);
    auto exec = console_.GetExecutor();

    // Define and call function in loop
    const char* code = R"(
        fun double(n)
            return n * 2
        
        sum = 0
        for i = 1; i <= 4; i = i + 1
            sum = sum + double(i)
        sum
    )";

    console_.Execute(code);
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);  // 2+4+6+8
}