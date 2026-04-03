#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct FunctionScopeTest : TestLangCommon {};

TEST_F(FunctionScopeTest, VariableUpdateInLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 1
while i <= 2
    sum = sum + i
    i = i + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));  // 1+2 = 3
}

TEST_F(FunctionScopeTest, SimpleFunctionInLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
inc = fun(x)
    x + 1

i = 0
while i < 2
    i = inc(i)
i
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));
}