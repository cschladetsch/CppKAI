#include "TestCommon.h"

USING_NAMESPACE_KAI

// Explicitly Delete()ing an object must remove it from every container that
// holds it. StorageBase::Delete() reaches containers through
// ClassBase::DetachFromContainer -> Traits<T>::ContainerOps::Erase, which was
// an empty stub, so containers kept a dead handle and size never dropped.
//
// Stack is deliberately not covered: Stack::Push never Attach()es, so stack
// elements are not registered with their stack and Delete() cannot reach
// them. Changing that alters GC behaviour on the executor's data stack and
// is a separate change.
struct TestContainerDetach : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<Array>();
        Reg().AddClass<List>();
        Reg().AddClass<ObjectSet>();
    }

    void CollectFully() {
        for (int n = 0; n < 100; ++n) Reg().GarbageCollect();
    }
};

TEST_F(TestContainerDetach, DeletedElementLeavesArray) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("c"), array);
    Object n = Reg().New<int>(42);
    array->PushBack(n);
    CollectFully();

    n.Delete();
    CollectFully();

    EXPECT_FALSE(n.Exists());
    EXPECT_EQ(array->Size(), 0);
}

TEST_F(TestContainerDetach, DeletedElementLeavesList) {
    Pointer<List> list = Reg().New<List>();
    Root().Set(Label("c"), list);
    Object n = Reg().New<int>(42);
    list->PushBack(n);
    CollectFully();

    n.Delete();
    CollectFully();

    EXPECT_FALSE(n.Exists());
    EXPECT_EQ(list->Size(), 0);
}

TEST_F(TestContainerDetach, DeletedElementLeavesSet) {
    Pointer<ObjectSet> set = Reg().New<ObjectSet>();
    Root().Set(Label("c"), set);
    Object n = Reg().New<int>(42);
    set->Insert(n);
    CollectFully();

    n.Delete();
    CollectFully();

    EXPECT_FALSE(n.Exists());
    EXPECT_EQ(set->Size(), 0);
}

TEST_F(TestContainerDetach, OnlyTheDeletedObjectLeavesWhenValuesAreEqual) {
    // The detach must match by handle, not value, or deleting one 42 could
    // remove a different 42.
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("c"), array);
    Object first = Reg().New<int>(42);
    Object second = Reg().New<int>(42);
    array->PushBack(first);
    array->PushBack(second);
    CollectFully();

    // Delete the second: a value search would find (and remove) the first.
    second.Delete();
    CollectFully();

    ASSERT_EQ(array->Size(), 1);
    EXPECT_EQ(array->At(0).GetHandle(), first.GetHandle());
    EXPECT_TRUE(first.Exists());
}

TEST_F(TestContainerDetach, SurvivingElementsKeepTheirOrder) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("c"), array);
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    Object c = Reg().New<int>(3);
    array->PushBack(a);
    array->PushBack(b);
    array->PushBack(c);
    CollectFully();

    b.Delete();
    CollectFully();

    ASSERT_EQ(array->Size(), 2);
    EXPECT_EQ(array->At(0).GetHandle(), a.GetHandle());
    EXPECT_EQ(array->At(1).GetHandle(), c.GetHandle());
}

TEST_F(TestContainerDetach, ObjectHeldTwiceLeavesBothSlots) {
    // Each PushBack registers a separate containers_ entry, so Erase runs
    // once per slot.
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("c"), array);
    Object n = Reg().New<int>(7);
    array->PushBack(n);
    array->PushBack(n);
    CollectFully();

    n.Delete();
    CollectFully();

    EXPECT_EQ(array->Size(), 0);
}

TEST_F(TestContainerDetach, DeletedObjectLeavesEveryContainer) {
    Pointer<Array> array = Reg().New<Array>();
    Pointer<List> list = Reg().New<List>();
    Root().Set(Label("a"), array);
    Root().Set(Label("l"), list);
    Object n = Reg().New<int>(9);
    array->PushBack(n);
    list->PushBack(n);
    CollectFully();

    n.Delete();
    CollectFully();

    EXPECT_EQ(array->Size(), 0);
    EXPECT_EQ(list->Size(), 0);
}
