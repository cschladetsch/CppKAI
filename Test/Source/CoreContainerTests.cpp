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
    
    // Store array in root to prevent garbage collection
    Root().Set(Label("test_array"), array);
    
    // Create elements
    Object elem1 = Reg().New<int>(1);
    Object elem2 = Reg().New<int>(2);
    Object elem3 = Reg().New<int>(3);
    
    // Store elements in root to prevent garbage collection
    Root().Set(Label("test_elem1"), elem1);
    Root().Set(Label("test_elem2"), elem2);
    Root().Set(Label("test_elem3"), elem3);
    
    ASSERT_TRUE(array.Exists());
    ASSERT_TRUE(array->Empty());
    ASSERT_EQ(array->Size(), 0);
    
    // Add elements
    array->PushBack(elem1);
    array->PushBack(elem2);
    array->PushBack(elem3);
    
    // Verify size and contents
    ASSERT_EQ(array->Size(), 3);
    ASSERT_FALSE(array->Empty());
    
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 3);
    
    // Clean up
    Root().Remove(Label("test_array"));
    Root().Remove(Label("test_elem1"));
    Root().Remove(Label("test_elem2"));
    Root().Remove(Label("test_elem3"));
}

// Test Array container with mixed types
TEST_F(CoreContainerTests, TestArrayMixedTypes) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Store array in root to prevent garbage collection
    Root().Set(Label("test_mixed_array"), array);
    
    // Create elements of different types
    Object intElem = Reg().New<int>(42);
    Object floatElem = Reg().New<float>(3.14f);
    Object strElem = Reg().New<String>("Hello");
    Object boolElem = Reg().New<bool>(true);
    
    // Store elements in root to prevent garbage collection
    Root().Set(Label("test_mixed_int"), intElem);
    Root().Set(Label("test_mixed_float"), floatElem);
    Root().Set(Label("test_mixed_str"), strElem);
    Root().Set(Label("test_mixed_bool"), boolElem);
    
    // Add elements of different types
    array->PushBack(intElem);
    array->PushBack(floatElem);
    array->PushBack(strElem);
    array->PushBack(boolElem);
    
    // Verify size and contents
    ASSERT_EQ(array->Size(), 4);
    
    // Use both IsType<T> and IsTypeNumber for type checking
    ASSERT_TRUE(array->At(0).IsType<int>());
    ASSERT_TRUE(array->At(0).IsTypeNumber(Type::Traits<int>::Number));
    
    ASSERT_TRUE(array->At(1).IsType<float>());
    ASSERT_TRUE(array->At(1).IsTypeNumber(Type::Traits<float>::Number));
    
    ASSERT_TRUE(array->At(2).IsType<String>());
    ASSERT_TRUE(array->At(2).IsTypeNumber(Type::Traits<String>::Number));
    
    ASSERT_TRUE(array->At(3).IsType<bool>());
    ASSERT_TRUE(array->At(3).IsTypeNumber(Type::Traits<bool>::Number));
    
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(array->At(1)), 3.14f);
    ASSERT_EQ(ConstDeref<String>(array->At(2)), "Hello");
    ASSERT_EQ(ConstDeref<bool>(array->At(3)), true);
    
    // Clean up
    Root().Remove(Label("test_mixed_array"));
    Root().Remove(Label("test_mixed_int"));
    Root().Remove(Label("test_mixed_float"));
    Root().Remove(Label("test_mixed_str"));
    Root().Remove(Label("test_mixed_bool"));
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
    
    // Store the element at index 1 for erasure
    Object elementToErase = array->At(1);
    ASSERT_TRUE(elementToErase.Exists());
    
    // Erase the element 
    array->Erase(elementToErase);
    
    // Verify result - we should have 2 elements left
    ASSERT_EQ(array->Size(), 2);
    
    // And the elements should be 1 and 2
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 2); // The last element moved up
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
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_map"), map);
    
    // Create keys and values
    Object keyOne = Reg().New<String>("one");
    Object keyTwo = Reg().New<String>("two");
    Object keyThree = Reg().New<String>("three");
    
    Object valOne = Reg().New<int>(1);
    Object valTwo = Reg().New<int>(2);
    Object valThree = Reg().New<int>(3);
    
    // Store keys and values in root to prevent garbage collection
    Root().Set(Label("test_key_one"), keyOne);
    Root().Set(Label("test_key_two"), keyTwo);
    Root().Set(Label("test_key_three"), keyThree);
    
    Root().Set(Label("test_val_one"), valOne);
    Root().Set(Label("test_val_two"), valTwo);
    Root().Set(Label("test_val_three"), valThree);
    
    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(map->Empty());
    ASSERT_EQ(map->Size(), 0);
    
    // Add key-value pairs using Insert
    map->Insert(keyOne, valOne);
    map->Insert(keyTwo, valTwo);
    map->Insert(keyThree, valThree);
    
    // Verify size
    ASSERT_EQ(map->Size(), 3);
    ASSERT_FALSE(map->Empty());
    
    // Get values by key using GetValue
    Object one = map->GetValue(keyOne);
    Object two = map->GetValue(keyTwo);
    Object three = map->GetValue(keyThree);
    
    ASSERT_TRUE(one.Exists());
    ASSERT_TRUE(two.Exists());
    ASSERT_TRUE(three.Exists());
    
    ASSERT_TRUE(one.IsType<int>());
    ASSERT_TRUE(one.IsTypeNumber(Type::Traits<int>::Number));
    
    ASSERT_EQ(ConstDeref<int>(one), 1);
    ASSERT_EQ(ConstDeref<int>(two), 2);
    ASSERT_EQ(ConstDeref<int>(three), 3);
    
    // Clean up
    Root().Remove(Label("test_map"));
    Root().Remove(Label("test_key_one"));
    Root().Remove(Label("test_key_two"));
    Root().Remove(Label("test_key_three"));
    Root().Remove(Label("test_val_one"));
    Root().Remove(Label("test_val_two"));
    Root().Remove(Label("test_val_three"));
}

// Test Map with different value types
TEST_F(CoreContainerTests, TestMapMixedValues) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_mixed_map"), map);
    
    // Create keys and values
    Object keyInt = Reg().New<String>("int");
    Object keyFloat = Reg().New<String>("float");
    Object keyString = Reg().New<String>("string");
    Object keyBool = Reg().New<String>("bool");
    
    Object valInt = Reg().New<int>(42);
    Object valFloat = Reg().New<float>(3.14f);
    Object valString = Reg().New<String>("Hello");
    Object valBool = Reg().New<bool>(true);
    
    // Store keys and values in root to prevent garbage collection
    Root().Set(Label("test_key_int"), keyInt);
    Root().Set(Label("test_key_float"), keyFloat);
    Root().Set(Label("test_key_string"), keyString);
    Root().Set(Label("test_key_bool"), keyBool);
    
    Root().Set(Label("test_val_int"), valInt);
    Root().Set(Label("test_val_float"), valFloat);
    Root().Set(Label("test_val_string"), valString);
    Root().Set(Label("test_val_bool"), valBool);
    
    // Add key-value pairs with different value types
    map->Insert(keyInt, valInt);
    map->Insert(keyFloat, valFloat);
    map->Insert(keyString, valString);
    map->Insert(keyBool, valBool);
    
    // Verify retrieval and types using both methods
    ASSERT_TRUE(map->GetValue(keyInt).IsType<int>());
    ASSERT_TRUE(map->GetValue(keyInt).IsTypeNumber(Type::Traits<int>::Number));
    
    ASSERT_TRUE(map->GetValue(keyFloat).IsType<float>());
    ASSERT_TRUE(map->GetValue(keyFloat).IsTypeNumber(Type::Traits<float>::Number));
    
    ASSERT_TRUE(map->GetValue(keyString).IsType<String>());
    ASSERT_TRUE(map->GetValue(keyString).IsTypeNumber(Type::Traits<String>::Number));
    
    ASSERT_TRUE(map->GetValue(keyBool).IsType<bool>());
    ASSERT_TRUE(map->GetValue(keyBool).IsTypeNumber(Type::Traits<bool>::Number));
    
    ASSERT_EQ(ConstDeref<int>(map->GetValue(keyInt)), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(map->GetValue(keyFloat)), 3.14f);
    ASSERT_EQ(ConstDeref<String>(map->GetValue(keyString)), "Hello");
    ASSERT_EQ(ConstDeref<bool>(map->GetValue(keyBool)), true);
    
    // Clean up
    Root().Remove(Label("test_mixed_map"));
    Root().Remove(Label("test_key_int"));
    Root().Remove(Label("test_key_float"));
    Root().Remove(Label("test_key_string"));
    Root().Remove(Label("test_key_bool"));
    Root().Remove(Label("test_val_int"));
    Root().Remove(Label("test_val_float"));
    Root().Remove(Label("test_val_string"));
    Root().Remove(Label("test_val_bool"));
}

// Test Map key existence check
TEST_F(CoreContainerTests, TestMapContains) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_contains_map"), map);
    
    // Create keys and values
    Object keyOne = Reg().New<String>("one");
    Object keyTwo = Reg().New<String>("two");
    Object keyThree = Reg().New<String>("three"); // Key that won't be added
    
    Object valOne = Reg().New<int>(1);
    Object valTwo = Reg().New<int>(2);
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_contains_key_one"), keyOne);
    Root().Set(Label("test_contains_key_two"), keyTwo);
    Root().Set(Label("test_contains_key_three"), keyThree);
    Root().Set(Label("test_contains_val_one"), valOne);
    Root().Set(Label("test_contains_val_two"), valTwo);
    
    // Add some entries
    map->Insert(keyOne, valOne);
    map->Insert(keyTwo, valTwo);
    
    // Check existing keys
    ASSERT_TRUE(map->ContainsKey(keyOne));
    ASSERT_TRUE(map->ContainsKey(keyTwo));
    
    // Check non-existing key
    ASSERT_FALSE(map->ContainsKey(keyThree));
    
    // Clean up
    Root().Remove(Label("test_contains_map"));
    Root().Remove(Label("test_contains_key_one"));
    Root().Remove(Label("test_contains_key_two"));
    Root().Remove(Label("test_contains_key_three"));
    Root().Remove(Label("test_contains_val_one"));
    Root().Remove(Label("test_contains_val_two"));
}

// Test Map value replacement
TEST_F(CoreContainerTests, TestMapValueReplacement) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_replace_map"), map);
    
    // Create key and values
    Object key = Reg().New<String>("key");
    Object val1 = Reg().New<int>(1);
    Object val2 = Reg().New<int>(2);
    Object val3 = Reg().New<String>("new value");
    
    // Store in root to prevent garbage collection
    Root().Set(Label("test_replace_key"), key);
    Root().Set(Label("test_replace_val1"), val1);
    Root().Set(Label("test_replace_val2"), val2);
    Root().Set(Label("test_replace_val3"), val3);
    
    // Add an initial key-value pair
    map->Insert(key, val1);
    ASSERT_EQ(ConstDeref<int>(map->GetValue(key)), 1);
    
    // Replace the value
    map->Insert(key, val2);
    ASSERT_EQ(ConstDeref<int>(map->GetValue(key)), 2);
    
    // Replace with a different type
    map->Insert(key, val3);
    ASSERT_TRUE(map->GetValue(key).IsType<String>());
    ASSERT_TRUE(map->GetValue(key).IsTypeNumber(Type::Traits<String>::Number));
    ASSERT_EQ(ConstDeref<String>(map->GetValue(key)), "new value");
    
    // Clean up
    Root().Remove(Label("test_replace_map"));
    Root().Remove(Label("test_replace_key"));
    Root().Remove(Label("test_replace_val1"));
    Root().Remove(Label("test_replace_val2"));
    Root().Remove(Label("test_replace_val3"));
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