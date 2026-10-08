#include "TestCommon.h"

USING_NAMESPACE_KAI

// Tri-colour GC behaviour for object graphs that contain cycles.
//
// Every test asserts on the object handles themselves (Exists()), never on a
// label lookup through Root(): once a label is removed, Root().Get() returns
// nothing whether or not the object was actually destroyed.
struct TestGarbageCycles : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<Array>();
        Reg().AddClass<Map>();
    }

    // TriColor() is incremental (bounded work per call), so a single
    // GarbageCollect() is not guaranteed to finish a cycle.
    void CollectFully() {
        for (int n = 0; n < 1000; ++n) Reg().GarbageCollect();
    }
};

TEST_F(TestGarbageCycles, TwoNodeDictionaryCycleIsCollected) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    b.Set(Label("a"), a);
    Root().Set(Label("cycle"), a);

    CollectFully();
    ASSERT_TRUE(a.Exists());
    ASSERT_TRUE(b.Exists());

    Root().Remove(Label("cycle"));
    CollectFully();

    EXPECT_FALSE(a.Exists());
    EXPECT_FALSE(b.Exists());
}

TEST_F(TestGarbageCycles, ThreeNodeDictionaryCycleIsCollected) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    Object c = Reg().New<int>(3);
    a.Set(Label("next"), b);
    b.Set(Label("next"), c);
    c.Set(Label("next"), a);
    Root().Set(Label("ring"), a);

    CollectFully();
    ASSERT_TRUE(a.Exists());
    ASSERT_TRUE(b.Exists());
    ASSERT_TRUE(c.Exists());

    Root().Remove(Label("ring"));
    CollectFully();

    EXPECT_FALSE(a.Exists());
    EXPECT_FALSE(b.Exists());
    EXPECT_FALSE(c.Exists());
}

TEST_F(TestGarbageCycles, CycleThroughArrayIsCollected) {
    Pointer<Array> array = Reg().New<Array>();
    Object node = Reg().New<int>(42);
    array->PushBack(node);
    node.Set(Label("owner"), array);
    Root().Set(Label("arr"), array);

    CollectFully();
    ASSERT_TRUE(array.Exists());
    ASSERT_TRUE(node.Exists());

    Root().Remove(Label("arr"));
    CollectFully();

    EXPECT_FALSE(array.Exists());
    EXPECT_FALSE(node.Exists());
}

TEST_F(TestGarbageCycles, ReachableCycleSurvives) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    b.Set(Label("a"), a);
    Root().Set(Label("live"), a);

    CollectFully();

    EXPECT_TRUE(a.Exists());
    EXPECT_TRUE(b.Exists());
    Root().Remove(Label("live"));
}

TEST_F(TestGarbageCycles, CycleReachableFromSecondRootSurvives) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    b.Set(Label("a"), a);
    Root().Set(Label("first"), a);
    Root().Set(Label("second"), b);

    CollectFully();
    Root().Remove(Label("first"));
    CollectFully();

    EXPECT_TRUE(a.Exists());
    EXPECT_TRUE(b.Exists());

    Root().Remove(Label("second"));
    CollectFully();

    EXPECT_FALSE(a.Exists());
    EXPECT_FALSE(b.Exists());
}

TEST_F(TestGarbageCycles, DetachedAcyclicChainIsCollected) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    Root().Set(Label("chain"), a);

    CollectFully();
    Root().Remove(Label("chain"));
    CollectFully();

    EXPECT_FALSE(a.Exists());
    EXPECT_FALSE(b.Exists());
}

TEST_F(TestGarbageCycles, ObjectAttachedMidCollectionSurvives) {
    // Build a large live graph so TriColor() cannot finish in one call, then
    // attach a fresh object to an already-traversed part of it.
    Pointer<Array> big = Reg().New<Array>();
    for (int n = 0; n < 200; ++n) big->PushBack(Reg().New<int>(n));
    Root().Set(Label("big"), big);
    CollectFully();

    Object holder = Reg().New<int>(0);
    Root().Set(Label("holder"), holder);
    Reg().GarbageCollect();  // partial pass

    Object late = Reg().New<int>(99);
    holder.Set(Label("late"), late);
    CollectFully();

    EXPECT_TRUE(late.Exists());
    EXPECT_TRUE(holder.Exists());
    Root().Remove(Label("holder"));
    Root().Remove(Label("big"));
}

TEST_F(TestGarbageCycles, ObjectLinkedBehindTraceFrontierSurvives) {
    // The trace snapshots edges when it begins, so removing an edge mid-trace
    // can only make it conservative. What it cannot see is an object that had
    // no referrer when it began and is then linked from an already-traced
    // object. Without the write barrier the sweep reclaims that live object.
    // Every budget is tried so the link lands at every point in the walk.
    for (int budget = 1; budget <= 8; ++budget) {
        Object a = Reg().New<int>(1);
        Root().Set(Label("a"), a);
        CollectFully();

        Reg().CollectCycles();  // no trace in progress
        Object c = Reg().New<int>(3);  // exists before the trace begins
        Reg().StepCycleTrace(budget);

        a.Set(Label("c"), c);

        while (!Reg().StepCycleTrace(1)) {
        }

        EXPECT_TRUE(c.Exists()) << "budget " << budget;

        Root().Remove(Label("a"));
        CollectFully();
    }
}

TEST_F(TestGarbageCycles, ObjectMovedMidTraceSurvives) {
    for (int budget = 1; budget <= 8; ++budget) {
        Object a = Reg().New<int>(1);
        Object d = Reg().New<int>(2);
        Object c = Reg().New<int>(3);
        Root().Set(Label("a"), a);
        Root().Set(Label("d"), d);
        d.Set(Label("c"), c);
        CollectFully();

        Reg().CollectCycles();
        Reg().StepCycleTrace(budget);

        a.Set(Label("c"), c);
        d.Remove(Label("c"));

        while (!Reg().StepCycleTrace(1)) {
        }

        EXPECT_TRUE(c.Exists()) << "budget " << budget;

        Root().Remove(Label("a"));
        Root().Remove(Label("d"));
        CollectFully();
    }
}

TEST_F(TestGarbageCycles, CollectCyclesIsImmediate) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    b.Set(Label("a"), a);
    Root().Set(Label("cycle"), a);
    CollectFully();

    Root().Remove(Label("cycle"));
    Reg().CollectCycles();

    EXPECT_FALSE(a.Exists());
    EXPECT_FALSE(b.Exists());
}

TEST_F(TestGarbageCycles, UnmanagedObjectKeepsCycleAlive) {
    Object a = Reg().New<int>(1);
    Object b = Reg().New<int>(2);
    a.Set(Label("b"), b);
    b.Set(Label("a"), a);
    Root().Set(Label("cycle"), a);
    CollectFully();

    a.SetManaged(false);
    Root().Remove(Label("cycle"));
    CollectFully();

    EXPECT_TRUE(a.Exists());
    EXPECT_TRUE(b.Exists());
}

TEST_F(TestGarbageCycles, ArrayElementLinkedMidTraceSurvives) {
    // Same as above, but through an Array element rather than a dictionary
    // child, and with the array possibly not yet visited when the link is
    // made. Array contents are only discoverable through the edges the
    // barrier records, so this covers the "unvisited container" branch.
    for (int budget = 1; budget <= 8; ++budget) {
        Pointer<Array> array = Reg().New<Array>();
        Object a = Reg().New<int>(1);
        Root().Set(Label("a"), a);
        Root().Set(Label("arr"), array);
        CollectFully();

        Reg().CollectCycles();
        Object c = Reg().New<int>(3);
        Reg().StepCycleTrace(budget);

        array->PushBack(c);

        while (!Reg().StepCycleTrace(1)) {
        }

        EXPECT_TRUE(c.Exists()) << "budget " << budget;

        Root().Remove(Label("a"));
        Root().Remove(Label("arr"));
        CollectFully();
    }
}
