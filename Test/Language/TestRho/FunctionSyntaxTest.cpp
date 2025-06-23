#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct FunctionSyntaxTest : TestLangCommon {};

TEST_F(FunctionSyntaxTest, OnlyAssignmentFormAllowed) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
add = fun(a, b)
    a + b
result = add(3, 4)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(7, ConstDeref<int>(result));
}

TEST_F(FunctionSyntaxTest, AnonymousFunctionInExpression) {
    console_.SetLanguage(Language::Rho);

    // Anonymous function call
    const char* code = R"(
square = fun(x)
    x * x
result = square(5)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(25, ConstDeref<int>(result));
}

TEST_F(FunctionSyntaxTest, FunctionReturningFunction) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
makeAdder = fun(x)
    fun(y)
        x + y

add5 = makeAdder(5)
result = add5(3)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(8, ConstDeref<int>(result));
}

TEST_F(FunctionSyntaxTest, FunctionAsParameter) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
apply = fun(f, x)
    f(x)
double = fun(n)
    n * 2

result = apply(double, 21)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(FunctionSyntaxTest, RecursiveFunction) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
factorial = fun(n)
    if n <= 1
        1
    else
        n * factorial(n - 1)

result = factorial(5)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(120, ConstDeref<int>(result));  // 5! = 120
}