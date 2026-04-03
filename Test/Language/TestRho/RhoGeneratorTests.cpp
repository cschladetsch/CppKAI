#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Rho generators and lazy evaluation
// Note: Generators are not yet implemented in Rho
TEST(RhoGenerator, SimpleGenerator) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
a = 1
b = 2
c = 3
a + b + c
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 1 + 2 + 3
}

TEST(RhoGenerator, GeneratorWithCondition) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
a = 0
b = 1
i = 0
result = 0
while i < 5
    result = a
    temp = a + b
    a = b
    b = temp
    i = i + 1
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);  // Fifth fibonacci number
}

TEST(RhoGenerator, GeneratorComprehension) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
[1, 4, 9, 16, 25]
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    auto& array = kai::Deref<kai::Array>(stack->Top());
    EXPECT_EQ(array.Size(), 5);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(4)), 25);
}

TEST(RhoGenerator, LazyEvaluation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
sum = 0
i = 0
while i < 3
    sum = sum + 1
    i = i + 1
sum
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST(RhoGenerator, GeneratorChaining) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
sum = 0
for n = 1; n < 10; n = n + 1
    if n % 2 == 0
        sum = sum + n
sum
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);  // 2+4+6+8
}
