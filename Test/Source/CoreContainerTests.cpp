#include "TestCommon.h"

USING_NAMESPACE_KAI

// Advanced Container tests for Core functionality
struct CoreContainerTests : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
        Reg().AddClass<Map>();
        // Set and List might not be registered standard containers in this
        // codebase Will add them only if they are explicitly found in the
        // registry
    }
};

// Test nested containers (Array of Maps)
TEST_F(CoreContainerTests, TestNestedContainers) {
    // Create an array of maps
    Pointer<Array> arrayOfMaps = Reg().New<Array>();

    // Create and add maps to the array
    for (int i = 0; i < 3; ++i) {
        Pointer<Map> map = Reg().New<Map>();
        map->Insert(Reg().New<String>("id"), Reg().New<int>(i));
        map->Insert(Reg().New<String>("value"),
                    Reg().New<String>("Map " + std::to_string(i)));
        arrayOfMaps->PushBack(map);
    }

    // Verify array size
    ASSERT_EQ(arrayOfMaps->Size(), 3);

    // Verify each map's contents
    for (int i = 0; i < 3; ++i) {
        Pointer<Map> map = arrayOfMaps->At(i);
        ASSERT_TRUE(map.Exists());
        ASSERT_EQ(map->Size(), 2);

        ASSERT_EQ(ConstDeref<int>(map->GetValue(Reg().New<String>("id"))), i);
        ASSERT_EQ(ConstDeref<String>(map->GetValue(Reg().New<String>("value"))),
                  "Map " + std::to_string(i));
    }
}