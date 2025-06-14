#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Rho/Rho.h>
#include <gtest/gtest.h>

class RhoBacktickShellTest : public ::testing::Test {
   protected:
    kai::Console console_;

    void SetUp() override { console_.SetLanguage(kai::Language::Rho); }
};

// Basic arithmetic with shell commands
TEST_F(RhoBacktickShellTest, SimpleAddition) {
    const char* code = R"(
        result = 1 + `echo 2`
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(RhoBacktickShellTest, MultipleShellCommands) {
    const char* code = R"(
        sum = `echo 10` + `echo 20`
        sum
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
}

TEST_F(RhoBacktickShellTest, SubtractionWithShell) {
    const char* code = R"(
        diff = `echo 100` - `echo 25`
        diff
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 75);
}

TEST_F(RhoBacktickShellTest, MultiplicationWithShell) {
    const char* code = R"(
        product = 5 * `echo 3`
        product
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

TEST_F(RhoBacktickShellTest, DivisionWithShell) {
    const char* code = R"(
        quotient = `echo 20` / `echo 4`
        quotient
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// String operations with shell commands
TEST_F(RhoBacktickShellTest, StringConcatenation) {
    const char* code = R"(
        greeting = "Hello " + `echo World`
        greeting
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello World");
}

TEST_F(RhoBacktickShellTest, StringFromShellCommand) {
    const char* code = R"(
        text = `echo "test string"`
        text
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "test string");
}

// Complex expressions
TEST_F(RhoBacktickShellTest, NestedArithmetic) {
    const char* code = R"(
        result = (`echo 5` + `echo 3`) * `echo 2`
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 16);  // (5+3)*2
}

TEST_F(RhoBacktickShellTest, ModuloWithShell) {
    const char* code = R"(
        remainder = `echo 17` % `echo 5`
        remainder
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

// Boolean operations
TEST_F(RhoBacktickShellTest, BooleanComparison) {
    const char* code = R"(
        equal = `echo 10` == `echo 10`
        equal
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickShellTest, LessThanComparison) {
    const char* code = R"(
        less = `echo 5` < `echo 10`
        less
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickShellTest, GreaterThanComparison) {
    const char* code = R"(
        greater = `echo 20` > `echo 15`
        greater
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Control structures with shell commands
TEST_F(RhoBacktickShellTest, IfWithShellCondition) {
    const char* code = R"(
        if (`echo 5` > `echo 3`) {
            result = "yes"
        } else {
            result = "no"
        }
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "yes");
}

TEST_F(RhoBacktickShellTest, ForLoopWithShellLimit) {
    const char* code = R"(
        sum = 0
        for i = 1; i <= `echo 5`; i = i + 1
            sum = sum + i
        sum
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5
}

// Complex shell command outputs
TEST_F(RhoBacktickShellTest, PipelineCommand) {
    const char* code = R"(
        word_count = `echo "one two three" | wc -w`
        word_count
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(RhoBacktickShellTest, FunctionWithShellParams) {
    const char* code = R"(
        fun add_shell(x) {
            return x + `echo 10`
        }
        result = add_shell(`echo 5`)
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

// Math expressions
TEST_F(RhoBacktickShellTest, MathExpressionEval) {
    const char* code = R"(
        calc = `echo $((2+3*4))`
        calc
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 14);
}

TEST_F(RhoBacktickShellTest, ComplexCalculation) {
    const char* code = R"(
        result = (`echo 2` + `echo 3`) * `echo 4` + `echo 5`
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 25);  // ((2+3)*4)+5
}

// Array operations with shell commands
TEST_F(RhoBacktickShellTest, ArrayWithShellElements) {
    const char* code = R"(
        arr = [`echo 1`, `echo 2`, `echo 3`]
        sum = 0
        for i = 0; i < 3; i = i + 1
            sum = sum + arr[i]
        sum
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 1+2+3
}

// Mixed operations
TEST_F(RhoBacktickShellTest, MixedStringAndNumber) {
    const char* code = R"(
        count = `echo 5` + `echo 3`
        message = to_string(count) + " items"
        message
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "8 items");
}