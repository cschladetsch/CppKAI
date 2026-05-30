#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct PiVeryComplexTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
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

TEST_F(PiVeryComplexTests, NestedForWhileAccumulator) {
    ExpectTop<int>(
        "0 'sum # "
        "{ 0 'i # } { i 6 < } { i 1 + 'i # } "
        "{ 0 'j # { j i < } "
        "{ sum i j * + 'sum # j 1 + 'j # } while } for "
        "sum",
        85);
}

TEST_F(PiVeryComplexTests, ContinuationPipelineWithIndependentFrames) {
    ExpectTop<int>(
        "{ 2 3 + } & "
        "{ 4 5 + } & * "
        "{ 10 2 div } & + "
        "{ 7 3 * } & -",
        29);
}

TEST_F(PiVeryComplexTests, ReplaceExitsBeforeDeadCodeAfterLoop) {
    ExpectTop<int>(
        "{ 5 'n # 1 'acc # "
        "{ n 1 > } { acc n * 'acc # n 1 - 'n # } while "
        "{ acc } ! 999 } &",
        120);
}

TEST_F(PiVeryComplexTests, MixedBooleanArithmeticAndBranching) {
    ExpectTop<int>(
        "0 'score # "
        "{ 1 'i # } { i 8 <= } { i 1 + 'i # } "
        "{ i 2 mod 0 == "
        "{ score i i * + 'score # } "
        "{ score i + 'score # } ife } for "
        "score",
        136);
}
