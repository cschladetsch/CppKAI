#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimpleForLoopTest : TestLangCommon {};

TEST_F(SimpleForLoopTest, BasicForLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
for i = 0; i < 3; i = i + 1
    sum = sum + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));
}

TEST_F(SimpleForLoopTest, ForLoopWithSimpleBody) {
    console_.SetLanguage(Language::Rho);

    // Even simpler - just increment a counter
    const char* code = R"(
count = 0
for i = 0; i < 5; i = i + 1
    count = count + 1
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}

TEST_F(SimpleForLoopTest, EmptyForLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
x = 10
for i = 0; i < 0; i = i + 1
    x = 99
x
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));  // Loop should not execute
}