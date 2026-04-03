#include "TestCommon.h"

USING_NAMESPACE_KAI

struct TestString : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<String>();
    }
};

TEST_F(TestString, TestCompare) {
    Pointer<String> s0 = Reg().New<String>("a");
    Pointer<String> s1 = Reg().New<String>("a");

    ASSERT_TRUE(s0.GetClass()->Equiv(s0.GetStorageBase(), s1.GetStorageBase()));

    *s1 = "b";
    ASSERT_FALSE(
        s0.GetClass()->Equiv(s0.GetStorageBase(), s1.GetStorageBase()));
    ASSERT_TRUE(s0.GetClass()->Less(s0.GetStorageBase(), s1.GetStorageBase()));
    ASSERT_FALSE(
        s0.GetClass()->Greater(s0.GetStorageBase(), s1.GetStorageBase()));
}

TEST_F(TestString, TestConcat) {}

TEST_F(TestString, TestLength) {}

TEST_F(TestString, TestStringStreamInsert) {}

TEST_F(TestString, TestStringStreamExtract) {}
