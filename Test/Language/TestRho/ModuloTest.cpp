#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct ModuloTest : TestLangCommon {};

TEST_F(ModuloTest, BasicModulo) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("result = 5 % 2", Structure::Statement);
    console_.Execute("result", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(1, ConstDeref<int>(result));
}

TEST_F(ModuloTest, ModuloInCondition) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
result = 0
if 4 % 2 == 0
    result = 1
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(1, ConstDeref<int>(result));
}