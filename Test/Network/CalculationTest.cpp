#include <gtest/gtest.h>

#include "KAI/Console/Console.h"

namespace {

int PopInt(kai::Console& console) {
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_FALSE(stack->Empty()) << "Expected a result on the stack";
    auto top = stack->Pop();
    EXPECT_TRUE(top.IsType<int>()) << "Expected an integer result";
    return kai::ConstDeref<int>(top);
}

}  // namespace

TEST(CalculationTest, BasicAddition) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);

    ASSERT_NO_THROW(console.Execute("1 2 +"));
    EXPECT_EQ(PopInt(console), 3);
}

TEST(CalculationTest, InvalidExpression) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);

    EXPECT_ANY_THROW(console.Execute("1 +"));
    EXPECT_TRUE(console.GetExecutor()->GetDataStack()->Empty());
}
