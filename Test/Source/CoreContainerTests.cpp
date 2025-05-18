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
        // Set and List might not be registered standard containers in this codebase
        // Will add them only if they are explicitly found in the registry
    }
};

// Test Array container functionality
TEST_F(CoreContainerTests, TestArrayBasicOperations) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    ASSERT_TRUE(array.Exists());
    ASSERT_TRUE(array->Empty());
    ASSERT_EQ(array->Size(), 0);
    
    // Add elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    
    // Verify size and contents
    ASSERT_EQ(array->Size(), 3);
    ASSERT_FALSE(array->Empty());
    
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 3);
}

// Test Array container with mixed types
TEST_F(CoreContainerTests, TestArrayMixedTypes) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Add elements of different types
    array->PushBack(Reg().New<int>(42));
    array->PushBack(Reg().New<float>(3.14f));
    array->PushBack(Reg().New<String>("Hello"));
    array->PushBack(Reg().New<bool>(true));
    
    // Verify size and contents
    ASSERT_EQ(array->Size(), 4);
    
    ASSERT_TRUE(array->At(0).IsType<int>());
    ASSERT_TRUE(array->At(1).IsType<float>());
    ASSERT_TRUE(array->At(2).IsType<String>());
    ASSERT_TRUE(array->At(3).IsType<bool>());
    
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(array->At(1)), 3.14f);
    ASSERT_EQ(ConstDeref<String>(array->At(2)), "Hello");
    ASSERT_EQ(ConstDeref<bool>(array->At(3)), true);
}

// Test Array Insert and Erase operations
TEST_F(CoreContainerTests, TestArrayInsertErase) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Add some initial elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(3));
    
    // Insert an element by pushing back
    array->PushBack(Reg().New<int>(2));
    
    // Verify the array
    ASSERT_EQ(array->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 2);
    
    // Erase an element using object reference
    array->Erase(array->At(1));
    
    // Verify result
    ASSERT_EQ(array->Size(), 2);
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 3);
}

// Test Array Clear operation
TEST_F(CoreContainerTests, TestArrayClear) {
    // Create an array with elements
    Pointer<Array> array = Reg().New<Array>();
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    
    ASSERT_EQ(array->Size(), 3);
    
    // Store references to the elements
    Handle h1 = array->At(0).GetHandle();
    Handle h2 = array->At(1).GetHandle();
    Handle h3 = array->At(2).GetHandle();
    
    // Clear the array
    array->Clear();
    
    // Verify array is empty
    ASSERT_TRUE(array->Empty());
    ASSERT_EQ(array->Size(), 0);
    
    // Verify elements are no longer in the registry after GC
    Reg().GarbageCollect();
    
    // GetObject is used to retrieve an object by handle
    Object obj1 = Reg().GetObject(h1);
    Object obj2 = Reg().GetObject(h2);
    Object obj3 = Reg().GetObject(h3);
    
    ASSERT_FALSE(obj1.Exists());
    ASSERT_FALSE(obj2.Exists());
    ASSERT_FALSE(obj3.Exists());
}

// Test Map container functionality
TEST_F(CoreContainerTests, TestMapBasicOperations) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(map->Empty());
    ASSERT_EQ(map->Size(), 0);
    
    // Add key-value pairs using Insert
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    map->Insert(Reg().New<String>("three"), Reg().New<int>(3));
    
    // Verify size
    ASSERT_EQ(map->Size(), 3);
    ASSERT_FALSE(map->Empty());
    
    // Get values by key using GetValue
    Object one = map->GetValue(Reg().New<String>("one"));
    Object two = map->GetValue(Reg().New<String>("two"));
    Object three = map->GetValue(Reg().New<String>("three"));
    
    ASSERT_TRUE(one.Exists());
    ASSERT_TRUE(two.Exists());
    ASSERT_TRUE(three.Exists());
    
    ASSERT_EQ(ConstDeref<int>(one), 1);
    ASSERT_EQ(ConstDeref<int>(two), 2);
    ASSERT_EQ(ConstDeref<int>(three), 3);
}

// Test Map with different value types
TEST_F(CoreContainerTests, TestMapMixedValues) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Add key-value pairs with different value types
    map->Insert(Reg().New<String>("int"), Reg().New<int>(42));
    map->Insert(Reg().New<String>("float"), Reg().New<float>(3.14f));
    map->Insert(Reg().New<String>("string"), Reg().New<String>("Hello"));
    map->Insert(Reg().New<String>("bool"), Reg().New<bool>(true));
    
    // Verify retrieval and types
    ASSERT_TRUE(map->GetValue(Reg().New<String>("int")).IsType<int>());
    ASSERT_TRUE(map->GetValue(Reg().New<String>("float")).IsType<float>());
    ASSERT_TRUE(map->GetValue(Reg().New<String>("string")).IsType<String>());
    ASSERT_TRUE(map->GetValue(Reg().New<String>("bool")).IsType<bool>());
    
    ASSERT_EQ(ConstDeref<int>(map->GetValue(Reg().New<String>("int"))), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(map->GetValue(Reg().New<String>("float"))), 3.14f);
    ASSERT_EQ(ConstDeref<String>(map->GetValue(Reg().New<String>("string"))), "Hello");
    ASSERT_EQ(ConstDeref<bool>(map->GetValue(Reg().New<String>("bool"))), true);
}

// Test Map key existence check
TEST_F(CoreContainerTests, TestMapContains) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Add some entries
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    
    // Check existing keys
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("one")));
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("two")));
    
    // Check non-existing key
    ASSERT_FALSE(map->ContainsKey(Reg().New<String>("three")));
}

// Test Map value replacement
TEST_F(CoreContainerTests, TestMapValueReplacement) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Add an initial key-value pair
    map->Insert(Reg().New<String>("key"), Reg().New<int>(1));
    ASSERT_EQ(ConstDeref<int>(map->GetValue(Reg().New<String>("key"))), 1);
    
    // Replace the value
    map->Insert(Reg().New<String>("key"), Reg().New<int>(2));
    ASSERT_EQ(ConstDeref<int>(map->GetValue(Reg().New<String>("key"))), 2);
    
    // Replace with a different type
    map->Insert(Reg().New<String>("key"), Reg().New<String>("new value"));
    ASSERT_TRUE(map->GetValue(Reg().New<String>("key")).IsType<String>());
    ASSERT_EQ(ConstDeref<String>(map->GetValue(Reg().New<String>("key"))), "new value");
}

// Test Map Erase operation
TEST_F(CoreContainerTests, TestMapErase) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Add entries
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    map->Insert(Reg().New<String>("three"), Reg().New<int>(3));
    
    ASSERT_EQ(map->Size(), 3);
    
    // Store a reference to a value
    Handle valueHandle = map->GetValue(Reg().New<String>("two")).GetHandle();
    
    // Erase an entry
    map->Erase(Reg().New<String>("two"));
    
    // Verify entry was removed
    ASSERT_EQ(map->Size(), 2);
    ASSERT_FALSE(map->ContainsKey(Reg().New<String>("two")));
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("one")));
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("three")));
    
    // Verify value was removed from registry after GC
    Reg().GarbageCollect();
    
    Object value = Reg().GetObject(valueHandle);
    ASSERT_FALSE(value.Exists());
}

// Test additional array operations
TEST_F(CoreContainerTests, TestArrayAdditionalOperations) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Add elements
    for (int i = 0; i < 5; i++) {
        array->PushBack(Reg().New<int>(i));
    }
    
    // Verify size and contents
    ASSERT_EQ(array->Size(), 5);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(ConstDeref<int>(array->At(i)), i);
    }
    
    // Test PopBack
    array->PopBack();
    ASSERT_EQ(array->Size(), 4);
    
    // Test Front and Back
    ASSERT_EQ(ConstDeref<int>(array->Front()), 0);
    ASSERT_EQ(ConstDeref<int>(array->Back()), 3);
}

// Test Array iteration
TEST_F(CoreContainerTests, TestArrayIteration) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Add elements
    for (int i = 0; i < 3; i++) {
        array->PushBack(Reg().New<int>(i + 1));
    }
    
    // Verify iteration
    int sum = 0;
    for (Array::const_iterator it = array->Begin(); it != array->End(); ++it) {
        sum += ConstDeref<int>(*it);
    }
    
    ASSERT_EQ(sum, 6);  // 1 + 2 + 3 = 6
}

// Test Map iteration
TEST_F(CoreContainerTests, TestMapIteration) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Add key-value pairs
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    map->Insert(Reg().New<String>("three"), Reg().New<int>(3));
    
    // Verify iteration (note: depends on map implementation details)
    int sum = 0;
    for (Map::const_iterator it = map->Begin(); it != map->End(); ++it) {
        sum += ConstDeref<int>(it->second);
    }
    
    ASSERT_EQ(sum, 6);  // 1 + 2 + 3 = 6
}

// Test container within container
TEST_F(CoreContainerTests, TestNestedArrays) {
    // Create an array of arrays
    Pointer<Array> outerArray = Reg().New<Array>();
    
    // Create and add inner arrays
    for (int i = 0; i < 3; ++i) {
        Pointer<Array> innerArray = Reg().New<Array>();
        
        // Add elements to inner array
        for (int j = 0; j < 3; ++j) {
            innerArray->PushBack(Reg().New<int>(i * 3 + j));
        }
        
        outerArray->PushBack(innerArray);
    }
    
    // Verify outer array size
    ASSERT_EQ(outerArray->Size(), 3);
    
    // Verify inner arrays
    for (int i = 0; i < 3; ++i) {
        Pointer<Array> innerArray = outerArray->At(i);
        ASSERT_EQ(innerArray->Size(), 3);
        
        for (int j = 0; j < 3; ++j) {
            ASSERT_EQ(ConstDeref<int>(innerArray->At(j)), i * 3 + j);
        }
    }
}

// Test nested containers
TEST_F(CoreContainerTests, TestNestedContainers) {
    // Create an array of maps
    Pointer<Array> arrayOfMaps = Reg().New<Array>();
    
    // Create and add maps to the array
    for (int i = 0; i < 3; ++i) {
        Pointer<Map> map = Reg().New<Map>();
        map->Insert(Reg().New<String>("id"), Reg().New<int>(i));
        map->Insert(Reg().New<String>("value"), Reg().New<String>("Map " + std::to_string(i)));
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
        ASSERT_EQ(ConstDeref<String>(map->GetValue(Reg().New<String>("value"))), "Map " + std::to_string(i));
    }
}