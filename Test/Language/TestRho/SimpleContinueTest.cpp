#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimpleContinueTest : TestLangCommon {};

TEST_F(SimpleContinueTest, ContinueInWhileLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
i = 0
while i < 5
    i = i + 1
    if i == 3
        continue
    sum = sum + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(4, ConstDeref<int>(result));  // Should skip when i==3, so 4 not 5
}

TEST_F(SimpleContinueTest, ContinueWithoutIf) {
    console_.SetLanguage(Language::Rho);

    // Test continue without being inside an if block
    const char* code = R"(
count = 0
i = 0
while i < 3
    i = i + 1
    count = count + 1
    continue
    count = count + 100  // This should never execute
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));  // Should be 3, not 303
}