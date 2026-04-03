#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimpleFunctionTest : TestLangCommon {};

TEST_F(SimpleFunctionTest, BasicFunctionCall) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
double = fun(x)
    x * 2

result = double(5)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));
}

TEST_F(SimpleFunctionTest, FunctionInExpression) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
add = fun(a, b)
    a + b

result = 1 + add(2, 3)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));
}