#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct DebugFunctionInLoop : TestLangCommon {};

TEST_F(DebugFunctionInLoop, SimpleAccumulation) {
    console_.SetLanguage(Language::Rho);

    // First test without function calls
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
    EXPECT_EQ(3, ConstDeref<int>(result));  // 1 + 2 = 3
}

TEST_F(DebugFunctionInLoop, FunctionCallOutsideLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
double = fun(x)
    x * 2

sum = 0
sum = sum + double(1)
sum = sum + double(2)
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));  // 2 + 4 = 6
}

TEST_F(DebugFunctionInLoop, SingleIterationWithFunction) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
double = fun(x)
    x * 2

sum = 0
i = 1
if i <= 1
    sum = sum + double(i)
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));  // double(1) = 2
}

TEST_F(DebugFunctionInLoop, FunctionReturnValue) {
    console_.SetLanguage(Language::Rho);

    // Test that function returns value properly
    const char* code = R"(
double = fun(x)
    x * 2

result = double(3)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));
}

TEST_F(DebugFunctionInLoop, AssignmentInCondition) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
if true
    sum = 5
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}