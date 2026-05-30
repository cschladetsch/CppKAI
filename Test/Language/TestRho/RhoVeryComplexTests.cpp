#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct RhoVeryComplexTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }

    template <class T>
    void ExpectTop(const char* script, const T& expected) {
        data_->Clear();
        console_.Execute(script, Structure::Program);
        ASSERT_FALSE(data_->Empty()) << script;
        ASSERT_TRUE(data_->Top().IsType<T>())
            << "Unexpected result type for script: " << script;
        EXPECT_EQ(ConstDeref<T>(data_->Top()), expected) << script;
    }
};

TEST_F(RhoVeryComplexTests, NestedLoopsWithPiBlockTerms) {
    ExpectTop<int>(
        R"(
sum = 0
for i = 1; i <= 5; i = i + 1
    for j = 1; j <= i; j = j + 1
        term = pi { i j * }
        if term % 2 == 0
            sum = sum + term
        else
            sum = sum + 1
sum
)",
        88);
}

TEST_F(RhoVeryComplexTests, WhileLoopWithPiBlockPredicateAndMutation) {
    ExpectTop<int>(
        R"(
i = 0
acc = 1
while pi { i 6 < }
    i = i + 1
    acc = pi { acc i + }
    if acc % 3 == 0
        acc = acc + pi { i 2 * }
acc
)",
        22);
}

TEST_F(RhoVeryComplexTests, FunctionLoopAndPiBlockComposition) {
    ExpectTop<int>(
        R"(
weighted = fun(n)
    total = 0
    for k = 1; k <= n; k = k + 1
        total = total + pi { k k * }
    total

result = weighted(4) + weighted(3)
result
)",
        44);
}
