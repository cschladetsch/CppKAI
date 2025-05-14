#include "TestCommon.h"

// WHY THE FUCK did I call this an Array
#include <KAI/Core/BuiltinTypes/Array.h>

USING_NAMESPACE_KAI

class TestArray : public TestCommon {
   protected:
    void AddRequiredClasses() override { Reg().AddClass<Array>(); }
};

TEST_F(TestArray, TestCreation) {
    Pointer<Array> cont = Reg().New<Array>();
    ASSERT_TRUE(cont.Exists());
    ASSERT_TRUE(cont->Size() == 0);
    ASSERT_TRUE(cont->Empty());

    Reg().GarbageCollect();

    ASSERT_FALSE(cont.Exists());
}

TEST_F(TestArray, TestInsertDelete) {
    Pointer<Array> cont = Reg().New<Array>();
    root_.Set("cont", cont);

    Object n = Reg().New(42);
    cont->PushBack(n);
    Reg().GarbageCollect();

    ASSERT_TRUE(cont.Exists());
    ASSERT_TRUE(n.Exists());

    cont->Erase(n);
    Reg().GarbageCollect();

    ASSERT_TRUE(cont.Exists());
    ASSERT_FALSE(n.Exists());
}

TEST_F(TestArray, TestComparison) {}

TEST_F(TestArray, TestStringStream) {}

TEST_F(TestArray, TestBinaryStream) {}
