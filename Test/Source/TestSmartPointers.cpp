#include <gtest/gtest.h>

#include "KAI/Core/Object.h"
#include "KAI/Core/Pointer.h"
#include "TestCommon.h"

// Test suite for smart pointer functionality
class SmartPointerTest : public TestCommon {
   protected:
    void SetUp() override { TestCommon::SetUp(); }
};

// Test 6: Shared pointer reference counting
TEST_F(SmartPointerTest, SharedPointerRefCounting) {
    auto obj = reg->New<int>(42);
    EXPECT_EQ(obj.GetRefCount(), 1);

    {
        auto copy1 = obj;
        EXPECT_EQ(obj.GetRefCount(), 2);

        auto copy2 = copy1;
        EXPECT_EQ(obj.GetRefCount(), 3);
    }

    EXPECT_EQ(obj.GetRefCount(), 1);
}

// Test 7: Weak pointer functionality
TEST_F(SmartPointerTest, WeakPointerBehavior) {
    kai::Pointer<int> strong = reg->New<int>(100);
    kai::WeakPointer<int> weak = strong;

    EXPECT_TRUE(weak.Exists());
    EXPECT_EQ(*weak.Lock(), 100);

    strong = kai::Object();  // Release strong reference

    EXPECT_FALSE(weak.Exists());
    EXPECT_TRUE(weak.Lock().Empty());
}

// Test 8: Circular reference handling
TEST_F(SmartPointerTest, CircularReferenceHandling) {
    struct Node {
        kai::Pointer<Node> next;
        kai::WeakPointer<Node> prev;
        int value;
    };

    reg->AddClass<Node>(kai::Label("Node"));

    auto node1 = reg->New<Node>();
    auto node2 = reg->New<Node>();

    kai::Deref<Node>(node1).next = node2;
    kai::Deref<Node>(node1).value = 1;

    kai::Deref<Node>(node2).prev = node1;
    kai::Deref<Node>(node2).value = 2;

    // Despite circular reference through weak pointer, objects can be deleted
    EXPECT_EQ(node1.GetRefCount(), 1);
    EXPECT_EQ(node2.GetRefCount(), 2);  // Referenced by node1.next
}

// Test 9: Pointer casting and type safety
TEST_F(SmartPointerTest, PointerCastingTypeSafety) {
    auto intObj = reg->New<int>(42);
    auto strObj = reg->New<kai::String>("Hello");

    // Valid cast to Object
    kai::Object baseObj = intObj;
    EXPECT_TRUE(baseObj.Valid());

    // Invalid cast should fail safely
    kai::Pointer<kai::String> invalidCast;
    EXPECT_NO_THROW(invalidCast = baseObj.CastTo<kai::String>());
    EXPECT_FALSE(invalidCast.Valid());

    // Valid downcast
    kai::Pointer<int> validCast = baseObj.CastTo<int>();
    EXPECT_TRUE(validCast.Valid());
    EXPECT_EQ(*validCast, 42);
}

// Test 10: Unique pointer semantics
TEST_F(SmartPointerTest, UniquePointerSemantics) {
    // Simulate unique pointer behavior
    auto obj = reg->New<int>(999);
    auto handle = obj.GetHandle();

    // Move semantics (simulated)
    auto moved = kai::Object(handle);
    obj = kai::Object();  // Clear original

    EXPECT_FALSE(obj.Valid());
    EXPECT_TRUE(moved.Valid());
    EXPECT_EQ(kai::ConstDeref<int>(moved), 999);
}