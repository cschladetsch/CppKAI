#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct DoWhileLoopTest : TestLangCommon {};

TEST_F(DoWhileLoopTest, SimpleDoWhileLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
count = 0
do
    count = count + 1
while count < 5
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}

TEST_F(DoWhileLoopTest, DoWhileExecutesAtLeastOnce) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
executed = false
do
    executed = true
while false
executed
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<bool>());
    EXPECT_TRUE(ConstDeref<bool>(result));
}

TEST_F(DoWhileLoopTest, DoWhileWithBreak) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 0
do
    sum = sum + i
    i = i + 1
    if i == 3
        break
while i < 10
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));  // 0+1+2 = 3
}

TEST_F(DoWhileLoopTest, DoWhileWithContinue) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 0
do
    i = i + 1
    if i % 2 == 0
        continue
    sum = sum + i
while i < 6
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(9, ConstDeref<int>(result));  // 1+3+5 = 9
}

TEST_F(DoWhileLoopTest, NestedDoWhileLoops) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
result = 0
i = 1
do
    j = 1
    do
        result = result + (i * j)
        j = j + 1
    while j <= 3
    i = i + 1
while i <= 2
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    // (1*1 + 1*2 + 1*3) + (2*1 + 2*2 + 2*3) = 6 + 12 = 18
    EXPECT_EQ(18, ConstDeref<int>(result));
}

TEST_F(DoWhileLoopTest, DoWhileWithFunctionCall) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
increment = fun(x)
    x + 1

value = 0
do
    value = increment(value)
while value < 5
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}