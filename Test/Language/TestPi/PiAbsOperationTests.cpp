// PiAbsOperationTests.cpp
//
// Tests for the Pi `abs` operation (absolute value of the top of the stack).
// `abs` pops one numeric value and pushes its magnitude. Integers and floats
// are supported; other types are left unchanged.

#include "TestLangCommon.h"

using namespace kai;

struct PiAbsTest : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }

    void Exec(const char *code) {
        exec_->ClearStacks();
        console_.Execute(code);
    }

    void ExpectInt(int expected) {
        ASSERT_EQ(data_->Size(), 1);
        ASSERT_TRUE(data_->Top().IsType<int>());
        EXPECT_EQ(ConstDeref<int>(data_->Top()), expected);
    }

    void ExpectFloat(float expected) {
        ASSERT_EQ(data_->Size(), 1);
        ASSERT_TRUE(data_->Top().IsType<float>());
        EXPECT_FLOAT_EQ(ConstDeref<float>(data_->Top()), expected);
    }
};

TEST_F(PiAbsTest, AbsOfPositiveInt) {
    Exec("5 abs");
    ExpectInt(5);
}

TEST_F(PiAbsTest, AbsOfNegativeInt) {
    // 0 5 - produces -5, abs makes it 5.
    Exec("0 5 - abs");
    ExpectInt(5);
}

TEST_F(PiAbsTest, AbsOfZero) {
    Exec("0 abs");
    ExpectInt(0);
}

TEST_F(PiAbsTest, AbsOfLargeNegativeInt) {
    Exec("0 100 - abs");
    ExpectInt(100);
}

TEST_F(PiAbsTest, AbsOfPositiveFloat) {
    Exec("3.5 abs");
    ExpectFloat(3.5f);
}

TEST_F(PiAbsTest, AbsOfNegativeFloat) {
    // 0.0 2.5 - produces -2.5, abs makes it 2.5.
    Exec("0.0 2.5 - abs");
    ExpectFloat(2.5f);
}

TEST_F(PiAbsTest, AbsCombinedWithArithmetic) {
    // abs of -5 plus abs of -3 = 8.
    Exec("0 5 - abs 0 3 - abs +");
    ExpectInt(8);
}

TEST_F(PiAbsTest, AbsIsIdempotent) {
    Exec("0 7 - abs abs");
    ExpectInt(7);
}
