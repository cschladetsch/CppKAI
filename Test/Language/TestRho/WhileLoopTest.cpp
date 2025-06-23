#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct WhileLoopTest : TestLangCommon {};

TEST_F(WhileLoopTest, SimpleWhileLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 0
while i < 5
    sum = sum + i
    i = i + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));  // 0+1+2+3+4 = 10
}

TEST_F(WhileLoopTest, WhileWithBreak) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
count = 0
while true
    count = count + 1
    if count == 5
        break
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}

TEST_F(WhileLoopTest, WhileWithContinue) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 0
while i < 10
    i = i + 1
    if i % 2 == 0
        continue
    sum = sum + i
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(25, ConstDeref<int>(result));  // 1+3+5+7+9 = 25
}

TEST_F(WhileLoopTest, NestedWhileLoops) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
total = 0
i = 0
while i < 3
    j = 0
    while j < 3
        total = total + (i * 10 + j)
        j = j + 1
    i = i + 1
total
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    // 0+1+2 + 10+11+12 + 20+21+22 = 99
    EXPECT_EQ(99, ConstDeref<int>(result));
}

TEST_F(WhileLoopTest, WhileWithFunctionCall) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
double = fun(x)
    x * 2

sum = 0
i = 1
while i <= 4
    sum = sum + double(i)
    i = i + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(20, ConstDeref<int>(result));  // 2+4+6+8 = 20
}

TEST_F(WhileLoopTest, WhileConditionWithFunctionCall) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
isLessThan10 = fun(x)
    x < 10

count = 0
while isLessThan10(count)
    count = count + 2
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));
}