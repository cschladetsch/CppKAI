#include "TestCommon.h"

USING_NAMESPACE_KAI

class TestList : public TestCommon {
   protected:
    void AddRequiredClasses() override { Reg().AddClass<List>(); }
};

TEST_F(TestList, TestCreation) {
    Pointer<List> list = Reg().New<List>();
    ASSERT_TRUE(list.Exists());
    ASSERT_TRUE(list->Size() == 0);
    ASSERT_TRUE(list->Empty());

    Reg().GarbageCollect();

    ASSERT_FALSE(list.Exists());
}

TEST_F(TestList, TestInsertDelete) {
    Pointer<List> list = Reg().New<List>();
    root_.Set("list", list);

    Object n = Reg().New(42);
    list->PushBack(n);
    Reg().GarbageCollect();

    ASSERT_TRUE(list.Exists());
    ASSERT_TRUE(n.Exists());

    ASSERT_EQ(list->Size(), 1);
    ASSERT_FALSE(list->Empty());
    list->Erase(n);
    ASSERT_EQ(list->Size(), 0);
    ASSERT_TRUE(list->Empty());
    Reg().GarbageCollect();

    ASSERT_TRUE(list.Exists());
    ASSERT_FALSE(n.Exists());
}

TEST_F(TestList, TestComparison) {
    Pointer<List> l0 = Reg().New<List>();
    Pointer<List> l1 = Reg().New<List>();
    root_.Set("list0", l0);
    root_.Set("list1", l1);

    l0->PushBack(Reg().New(42));
    l0->PushBack(Reg().New<String>("Hello"));

    l1->PushBack(Reg().New(42));
    l1->PushBack(Reg().New<String>("Hello"));

    // assert same value
    ASSERT_EQ(*l0, *l1);

    // change the first value to be 2, which is less than 42, so it should be
    // less than
    Deref<int>(l0->Front()) = 2;

    // assert less than
    ASSERT_LT(*l0, *l1);
}

TEST_F(TestList, TestStringStream) {}

TEST_F(TestList, TestBinaryStream) {
    Pointer<List> l = Reg().New<List>();
    Pointer<BinaryStream> s = Reg().New<BinaryStream>();

    l->PushBack(Reg().New(42));
    l->PushBack(Reg().New<String>("Hello"));

    ASSERT_EQ(s->Size(), 0);

    *s << l;

    ASSERT_GT(s->Size(), 0);

    // this fails for same reason that all the other extraction
    // from binary streams fail: the underlying BinaryPacket
    // that BinaryStream derives from isn't given a registry...
    // Object l2;
    // *s >> l2;

    // ASSERT_EQ(l2, l);
}
