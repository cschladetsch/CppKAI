#include "TestCommon.h"

USING_NAMESPACE_KAI

// Test fixture specifically for Array tests
struct TestArray : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
    }
};

// Test Array container functionality
TEST_F(TestArray, TestArrayBasicOperations) {
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
TEST_F(TestArray, TestArrayMixedTypes) {
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
TEST_F(TestArray, TestArrayInsertErase) {
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
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 2);  // The last element moved up
}

// Test Array Clear operation
TEST_F(TestArray, TestArrayClear) {
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

// Test additional array operations
TEST_F(TestArray, TestArrayAdditionalOperations) {
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
TEST_F(TestArray, TestArrayIteration) {
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

// Test container within container
TEST_F(TestArray, TestNestedArrays) {
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

// ========== 10 Additional Tests for Array::Insert ==========

// Test 1: Basic Insert at beginning
TEST_F(TestArray, TestArrayInsertAtBeginning) {
    Pointer<Array> array = Reg().New<Array>();

    // Add initial elements
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    array->PushBack(Reg().New<int>(4));

    // Insert at beginning
    array->Insert(0, Reg().New<int>(1));

    // Verify
    ASSERT_EQ(array->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 3);
    ASSERT_EQ(ConstDeref<int>(array->At(3)), 4);
}

// Test 2: Insert at middle positions
TEST_F(TestArray, TestArrayInsertAtMiddle) {
    Pointer<Array> array = Reg().New<Array>();

    // Add initial elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(3));
    array->PushBack(Reg().New<int>(5));

    // Insert at middle positions
    array->Insert(1, Reg().New<int>(2));
    array->Insert(3, Reg().New<int>(4));

    // Verify
    ASSERT_EQ(array->Size(), 5);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(ConstDeref<int>(array->At(i)), i + 1);
    }
}

// Test 3: Insert at end (equivalent to append)
TEST_F(TestArray, TestArrayInsertAtEnd) {
    Pointer<Array> array = Reg().New<Array>();

    // Add initial elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));

    // Insert at end
    array->Insert(2, Reg().New<int>(3));
    array->Insert(3, Reg().New<int>(4));

    // Verify
    ASSERT_EQ(array->Size(), 4);
    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(ConstDeref<int>(array->At(i)), i + 1);
    }
}

// Test 4: Insert into empty array
TEST_F(TestArray, TestArrayInsertIntoEmpty) {
    Pointer<Array> array = Reg().New<Array>();

    // Insert into empty array
    array->Insert(0, Reg().New<String>("first"));

    // Verify
    ASSERT_EQ(array->Size(), 1);
    ASSERT_EQ(ConstDeref<String>(array->At(0)), "first");

    // Insert another at beginning
    array->Insert(0, Reg().New<String>("new first"));

    // Verify
    ASSERT_EQ(array->Size(), 2);
    ASSERT_EQ(ConstDeref<String>(array->At(0)), "new first");
    ASSERT_EQ(ConstDeref<String>(array->At(1)), "first");
}

// Test 5: Insert mixed types
TEST_F(TestArray, TestArrayInsertMixedTypes) {
    Pointer<Array> array = Reg().New<Array>();

    // Insert different types at various positions
    array->Insert(0, Reg().New<int>(42));
    array->Insert(0, Reg().New<String>("hello"));
    array->Insert(1, Reg().New<float>(3.14f));
    array->Insert(3, Reg().New<bool>(true));

    // Verify order and types
    ASSERT_EQ(array->Size(), 4);

    ASSERT_TRUE(array->At(0).IsType<String>());
    ASSERT_EQ(ConstDeref<String>(array->At(0)), "hello");

    ASSERT_TRUE(array->At(1).IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(array->At(1)), 3.14f);

    ASSERT_TRUE(array->At(2).IsType<int>());
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 42);

    ASSERT_TRUE(array->At(3).IsType<bool>());
    ASSERT_EQ(ConstDeref<bool>(array->At(3)), true);
}

// Test 6: Insert multiple elements at same position
TEST_F(TestArray, TestArrayInsertMultipleAtSamePosition) {
    Pointer<Array> array = Reg().New<Array>();

    // Add initial elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(5));

    // Insert multiple elements at position 1
    array->Insert(1, Reg().New<int>(2));
    array->Insert(1, Reg().New<int>(3));
    array->Insert(1, Reg().New<int>(4));

    // Verify - should be [1, 4, 3, 2, 5]
    ASSERT_EQ(array->Size(), 5);
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(array->At(1)), 4);
    ASSERT_EQ(ConstDeref<int>(array->At(2)), 3);
    ASSERT_EQ(ConstDeref<int>(array->At(3)), 2);
    ASSERT_EQ(ConstDeref<int>(array->At(4)), 5);
}

// Test 7: Insert with invalid indices (boundary tests)
TEST_F(TestArray, TestArrayInsertBoundaryConditions) {
    Pointer<Array> array = Reg().New<Array>();

    // Add some elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));

    // Test inserting at size (valid - end of array)
    ASSERT_NO_THROW(array->Insert(2, Reg().New<int>(3)));
    ASSERT_EQ(array->Size(), 3);

    // Test inserting past size (should throw)
    ASSERT_THROW(array->Insert(10, Reg().New<int>(4)), Exception::BadIndex);

    // Test inserting at negative index (should throw)
    ASSERT_THROW(array->Insert(-1, Reg().New<int>(0)), Exception::BadIndex);
}

// Test 8: Insert arrays into arrays (nested arrays)
TEST_F(TestArray, TestArrayInsertNestedArrays) {
    Pointer<Array> outerArray = Reg().New<Array>();

    // Create inner arrays
    Pointer<Array> array1 = Reg().New<Array>();
    array1->PushBack(Reg().New<int>(1));
    array1->PushBack(Reg().New<int>(2));

    Pointer<Array> array2 = Reg().New<Array>();
    array2->PushBack(Reg().New<int>(3));
    array2->PushBack(Reg().New<int>(4));

    Pointer<Array> array3 = Reg().New<Array>();
    array3->PushBack(Reg().New<int>(5));
    array3->PushBack(Reg().New<int>(6));

    // Insert arrays at different positions
    outerArray->Insert(0, array1);
    outerArray->Insert(0, array2);
    outerArray->Insert(2, array3);

    // Verify
    ASSERT_EQ(outerArray->Size(), 3);

    // Check first array (was inserted at 0, so it's array2)
    Pointer<Array> first = outerArray->At(0);
    ASSERT_EQ(first->Size(), 2);
    ASSERT_EQ(ConstDeref<int>(first->At(0)), 3);
    ASSERT_EQ(ConstDeref<int>(first->At(1)), 4);

    // Check last array
    Pointer<Array> last = outerArray->At(2);
    ASSERT_EQ(last->Size(), 2);
    ASSERT_EQ(ConstDeref<int>(last->At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(last->At(1)), 6);
}

// Test 9: Insert and memory management
TEST_F(TestArray, TestArrayInsertMemoryManagement) {
    Pointer<Array> array = Reg().New<Array>();

    // Store array in root to prevent GC
    Root().Set(Label("test_insert_array"), array);

    // Create objects and store handles
    Object obj1 = Reg().New<String>("first");
    Object obj2 = Reg().New<String>("second");
    Object obj3 = Reg().New<String>("third");

    Handle h1 = obj1.GetHandle();
    Handle h2 = obj2.GetHandle();
    Handle h3 = obj3.GetHandle();

    // Insert objects
    array->Insert(0, obj1);
    array->Insert(1, obj2);
    array->Insert(1, obj3);  // Insert between first and second

    // Verify objects are still alive
    ASSERT_TRUE(Reg().GetObject(h1).Exists());
    ASSERT_TRUE(Reg().GetObject(h2).Exists());
    ASSERT_TRUE(Reg().GetObject(h3).Exists());

    // Clear array and run GC
    array->Clear();
    Reg().GarbageCollect();

    // Objects should be collected
    ASSERT_FALSE(Reg().GetObject(h1).Exists());
    ASSERT_FALSE(Reg().GetObject(h2).Exists());
    ASSERT_FALSE(Reg().GetObject(h3).Exists());

    // Clean up
    Root().Remove(Label("test_insert_array"));
}

// Test 10: Large scale insert operations
TEST_F(TestArray, TestArrayInsertLargeScale) {
    Pointer<Array> array = Reg().New<Array>();

    // Add initial elements (even numbers)
    for (int i = 0; i < 50; i++) {
        array->PushBack(Reg().New<int>(i * 2));
    }

    // Insert odd numbers at correct positions
    for (int i = 0; i < 50; i++) {
        int value = i * 2 + 1;
        int position = i + 1 + i;  // Account for previous insertions
        array->Insert(position, Reg().New<int>(value));
    }

    // Verify we have 100 sequential numbers
    ASSERT_EQ(array->Size(), 100);
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(ConstDeref<int>(array->At(i)), i);
    }

    // Test performance with many inserts at beginning
    Pointer<Array> perfArray = Reg().New<Array>();

    // Insert 100 elements always at position 0
    for (int i = 0; i < 100; i++) {
        perfArray->Insert(0, Reg().New<int>(i));
    }

    // Verify reverse order (last inserted is first)
    ASSERT_EQ(perfArray->Size(), 100);
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(ConstDeref<int>(perfArray->At(i)), 99 - i);
    }
}