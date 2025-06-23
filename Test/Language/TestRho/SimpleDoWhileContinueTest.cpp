#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimpleDoWhileContinueTest : TestLangCommon {};

TEST_F(SimpleDoWhileContinueTest, DoWhileContinueWithoutModulo) {
    console_.SetLanguage(Language::Rho);

    // Similar to the failing test but without modulo
    const char* code = R"(
sum = 0
i = 0
do
    i = i + 1
    if i == 2
        continue
    sum = sum + i
while i < 4
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(8, ConstDeref<int>(result));  // 1 + 3 + 4 = 8 (skips 2)
}

TEST_F(SimpleDoWhileContinueTest, DoWhileContinueNoIf) {
    console_.SetLanguage(Language::Rho);

    // Test continue in do-while without if block
    const char* code = R"(
count = 0
i = 0
do
    i = i + 1
    count = count + 1
    continue
    count = count + 100  // Should never execute
while i < 3
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));  // Should be 3, not 303
}