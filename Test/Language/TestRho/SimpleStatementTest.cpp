#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimpleStatementTest : TestLangCommon {};

TEST_F(SimpleStatementTest, SingleAssignment) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("x = 42", Structure::Statement);
    console_.Execute("x", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(SimpleStatementTest, SimpleWhile) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
i = 0
while i < 3
    i = i + 1
i
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));
}