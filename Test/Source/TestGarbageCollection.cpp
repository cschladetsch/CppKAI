#include "KAI/Core/BuiltinTypes/Array.h"
#include "TestCommon.h"

using namespace kai;
using namespace std;

namespace {
struct GarbageCollectionTest : TestCommon {
    Label _label;

    GarbageCollectionTest() : _label("_num") {}

    void AddRequiredClasses() override {
        Reg().AddClass<Array>(Label("Array"));
    }

    pair<Pointer<Array>, Pointer<int>> AddNumToArray() const {
        Pointer<Array> vec = Reg().New<Array>();
        Pointer<int> num = Reg().New(42);
        vec->PushBack(num);
        Root().Add(Label("array"), vec);
        return make_pair(vec, num);
    }
};

TEST_F(GarbageCollectionTest, ObjectDeletedWhenNotInTree) {
    const Object num = Reg().New(42);
    EXPECT_TRUE(num.Exists());
    Reg().GarbageCollect();
    EXPECT_FALSE(num.Exists());
}

TEST_F(GarbageCollectionTest, ObjectDeletedWhenRemovedFromTree) {
    const Object num = Reg().New(42);
    EXPECT_TRUE(num.Exists());
    GetTree().GetRoot().Set(_label, num);
    Reg().GarbageCollect();
    EXPECT_TRUE(num.Exists());
    GetTree().GetRoot().Remove(_label);
    Reg().GarbageCollect();
    EXPECT_FALSE(num.Exists());
}

TEST_F(GarbageCollectionTest, ObjectCanDeleteItself) {
    const Object num = Reg().New(42);
    EXPECT_TRUE(num.Exists());
    GetTree().GetRoot().Set(_label, num);
    Reg().GarbageCollect();
    EXPECT_TRUE(num.Exists());
    num.Delete();
    Reg().GarbageCollect();
    EXPECT_FALSE(num.Exists());
}

TEST_F(GarbageCollectionTest, ObjectNotDeletedWhenInArray) {
    const auto [vec, num] = AddNumToArray();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());

    Reg().GarbageCollect();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());
}

TEST_F(GarbageCollectionTest, ObjectDeletedWhenRemovedFromArray) {
    auto [vec, num] = AddNumToArray();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());

    Reg().GarbageCollect();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());

    vec->PopBack();
    Reg().GarbageCollect();
    EXPECT_FALSE(num.Exists());
    EXPECT_TRUE(vec.Exists());
}

TEST_F(GarbageCollectionTest, ObjectDeletedWhenDetached) {
    auto [vec, num] = AddNumToArray();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());

    Reg().GarbageCollect();
    EXPECT_TRUE(num.Exists());
    EXPECT_TRUE(vec.Exists());

    num.Detach(vec);
    Reg().GarbageCollect();
    EXPECT_FALSE(num.Exists());
    EXPECT_TRUE(vec.Exists());
}
}  // namespace
