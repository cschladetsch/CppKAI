#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Rho pi{} block syntax
TEST(RhoPiBlock, BasicPiExpression) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Basic Pi arithmetic in a pi block
    console.Execute("result = pi { 2 3 + }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST(RhoPiBlock, PiBlockAsStatement) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi block as a standalone statement
    console.Execute("pi { 10 20 + }");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
}

TEST(RhoPiBlock, PiBlockWithStringOperations) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // String operations in Pi block
    console.Execute(R"(greeting = pi { "Hello " "World" + }; greeting)");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello World");
}

TEST(RhoPiBlock, PiBlockWithComparison) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Comparison operations in Pi block
    console.Execute("result = pi { 5 3 > }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}

TEST(RhoPiBlock, PiBlockInCondition) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi block used in if condition
    console.Execute(R"(
        if pi { 10 5 > }
            result = 100
        else
            result = 200
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

TEST(RhoPiBlock, PiBlockWithStackManipulation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Stack manipulation in Pi block
    console.Execute("result = pi { 1 2 3 rot + + }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 1 + 2 + 3
}

TEST(RhoPiBlock, NestedPiBlocks) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Nested pi blocks (pi block result used in another expression)
    console.Execute(R"(
        a = pi { 5 5 + }
        b = pi { 3 3 * }
        result = a + b
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 19);  // 10 + 9
}

TEST(RhoPiBlock, PiBlockInArray) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi blocks as array elements
    console.Execute(R"(
        arr = [pi { 1 1 + }, pi { 2 2 * }, pi { 3 3 - }]
        arr[1]
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);  // 2 * 2
}

TEST(RhoPiBlock, PiBlockWithFloats) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Float operations in Pi block
    console.Execute("result = pi { 3.14 2.0 * }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 6.28f);
}

TEST(RhoPiBlock, PiBlockWithLogicalOps) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Logical operations in Pi block
    console.Execute("result = pi { true false or }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}

TEST(RhoPiBlock, PiBlockInFunctionArgument) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi block as function argument
    console.Execute(R"(
        fun double(x)
            x * 2
        
        result = double(pi { 7 8 + })
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);  // (7 + 8) * 2
}

TEST(RhoPiBlock, PiBlockWithDup) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Dup operation in Pi block
    console.Execute("result = pi { 42 dup * }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 1764);  // 42 * 42
}

TEST(RhoPiBlock, PiBlockWithSwap) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Swap operation in Pi block
    console.Execute("result = pi { 10 3 swap / }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()),
              0);  // 3 / 10 (integer division)
}

TEST(RhoPiBlock, PiBlockInWhileCondition) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi block in while loop condition
    console.Execute(R"(
        counter = 0
        while pi { counter 5 < }
            counter = counter + 1
        counter
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST(RhoPiBlock, PiBlockWithModulo) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Modulo operation in Pi block
    console.Execute("result = pi { 17 5 % }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST(RhoPiBlock, PiBlockReturnValue) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Pi block as return value
    console.Execute(R"(
        fun getPiResult()
            return pi { 6 7 * }
        
        result = getPiResult()
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(RhoPiBlock, PiBlockWithNegation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Negation in Pi block
    console.Execute("result = pi { 25 neg }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -25);
}

TEST(RhoPiBlock, PiBlockChainedOperations) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Multiple chained operations in Pi block
    console.Execute("result = pi { 2 3 + 4 * 5 - }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // ((2 + 3) * 4) - 5
}

TEST(RhoPiBlock, PiBlockWithEquality) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Equality test in Pi block (verifying == conversion)
    console.Execute("result = pi { 10 10 == }; result");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}

TEST(RhoPiBlock, PiBlockComplexExpression) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Complex expression mixing Rho and Pi
    console.Execute(R"(
        x = 5
        y = 10
        result = x * pi { 3 4 + } + y
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 45);  // 5 * 7 + 10
}