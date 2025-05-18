#include "TestCommon.h"

USING_NAMESPACE_KAI

// Advanced Pointer tests for Core functionality
struct CorePointerTests : TestCommon {
protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
        Reg().AddClass<Map>();
    }
};

// Test basic Pointer<T> creation and access
TEST_F(CorePointerTests, TestPointerCreationAndAccess) {
    // Create pointers to various types
    Pointer<int> intPtr = Reg().New<int>(42);
    Pointer<String> strPtr = Reg().New<String>("Hello");
    Pointer<Array> arrayPtr = Reg().New<Array>();
    
    // Verify pointers exist
    ASSERT_TRUE(intPtr.Exists());
    ASSERT_TRUE(strPtr.Exists());
    ASSERT_TRUE(arrayPtr.Exists());
    
    // Test dereferencing operators
    ASSERT_EQ(*intPtr, 42);
    ASSERT_EQ(*strPtr, "Hello");
    ASSERT_EQ(arrayPtr->Size(), 0);
    
    // Modify through pointer
    *intPtr = 100;
    *strPtr = "World";
    arrayPtr->PushBack(Reg().New<int>(1));
    
    // Verify changes
    ASSERT_EQ(*intPtr, 100);
    ASSERT_EQ(*strPtr, "World");
    ASSERT_EQ(arrayPtr->Size(), 1);
}

// Test Pointer<T> copying
TEST_F(CorePointerTests, TestPointerCopying) {
    // Create original pointer
    Pointer<int> original = Reg().New<int>(42);
    
    // Create a copy
    Pointer<int> copy = original;
    
    // Verify both point to same object
    ASSERT_EQ(original.GetHandle(), copy.GetHandle());
    ASSERT_EQ(*original, *copy);
    
    // Modify through copy
    *copy = 100;
    
    // Verify original also sees the change
    ASSERT_EQ(*original, 100);
}

// Test Pointer<T> assignment
TEST_F(CorePointerTests, TestPointerAssignment) {
    // Create two pointers
    Pointer<int> ptr1 = Reg().New<int>(1);
    Pointer<int> ptr2 = Reg().New<int>(2);
    
    // Verify they are different
    ASSERT_NE(ptr1.GetHandle(), ptr2.GetHandle());
    ASSERT_NE(*ptr1, *ptr2);
    
    // Store handles for verification later
    Handle handle1 = ptr1.GetHandle();
    Handle handle2 = ptr2.GetHandle();
    
    // Assign ptr2 to ptr1
    ptr1 = ptr2;
    
    // Verify ptr1 now points to ptr2's object
    ASSERT_EQ(ptr1.GetHandle(), ptr2.GetHandle());
    ASSERT_EQ(*ptr1, *ptr2);
    
    // Modify through ptr1
    *ptr1 = 100;
    
    // Verify ptr2 sees the change
    ASSERT_EQ(*ptr2, 100);
    
    // After garbage collection, the original ptr1 object should be gone
    Reg().GarbageCollect();
    
    // Try to access the original objects
    Object obj1 = Reg().GetObject(handle1);
    Object obj2 = Reg().GetObject(handle2);
    
    ASSERT_FALSE(obj1.Exists());  // Original ptr1 object should be gone
    ASSERT_TRUE(obj2.Exists());   // ptr2 object should still exist
}

// Test Pointer<T> null handling
TEST_F(CorePointerTests, TestPointerNullHandling) {
    // Create null pointer
    Pointer<int> nullPtr;
    
    // Verify it doesn't exist
    ASSERT_FALSE(nullPtr.Exists());
    
    // Create a valid pointer
    Pointer<int> validPtr = Reg().New<int>(42);
    ASSERT_TRUE(validPtr.Exists());
    
    // Assign null to valid
    validPtr = nullPtr;
    
    // Verify valid is now null
    ASSERT_FALSE(validPtr.Exists());
}

// Test Pointer<T> from Object
TEST_F(CorePointerTests, TestPointerFromObject) {
    // Create an Object
    Object obj = Reg().New<int>(42);
    
    // Create a Pointer<T> from Object
    Pointer<int> ptr = obj;
    
    // Verify it exists and points to the same data
    ASSERT_TRUE(ptr.Exists());
    ASSERT_EQ(ptr.GetHandle(), obj.GetHandle());
    ASSERT_EQ(*ptr, 42);
    
    // Modify through pointer
    *ptr = 100;
    
    // Verify Object sees the change
    ASSERT_EQ(ConstDeref<int>(obj), 100);
}

// Test Pointer<T> to Object conversion
TEST_F(CorePointerTests, TestPointerToObjectConversion) {
    // Create a Pointer<T>
    Pointer<int> ptr = Reg().New<int>(42);
    
    // Get Object from Pointer<T>
    Object obj = ptr;
    
    // Verify it exists and points to the same data
    ASSERT_TRUE(obj.Exists());
    ASSERT_EQ(obj.GetHandle(), ptr.GetHandle());
    ASSERT_EQ(ConstDeref<int>(obj), 42);
    
    // Modify through Object
    Deref<int>(obj) = 100;
    
    // Verify Pointer<T> sees the change
    ASSERT_EQ(*ptr, 100);
}

// Test Pointer<T> with invalid type casting
TEST_F(CorePointerTests, TestPointerInvalidTypeCasting) {
    // Create an Object
    Object obj = Reg().New<int>(42);
    
    // Try to cast to incorrect type
    Pointer<float> incorrectPtr = obj;
    
    // Verify casting failed
    ASSERT_FALSE(incorrectPtr.Exists());
    
    // The original object should still exist
    ASSERT_TRUE(obj.Exists());
    ASSERT_EQ(ConstDeref<int>(obj), 42);
}

// Test Pointer<T> lifetime during garbage collection
TEST_F(CorePointerTests, TestPointerLifetimeDuringGC) {
    // Create pointer not stored in tree
    Pointer<int> tempPtr = Reg().New<int>(42);
    
    // Hold handle for later verification
    Handle handle = tempPtr.GetHandle();
    
    // Verify it exists
    ASSERT_TRUE(tempPtr.Exists());
    
    // Run garbage collection - the object should survive because tempPtr holds a reference
    Reg().GarbageCollect();
    ASSERT_TRUE(tempPtr.Exists());
    
    // Clear the pointer
    tempPtr = Pointer<int>();
    
    // Run garbage collection - now the object should be gone
    Reg().GarbageCollect();
    
    // Try to recreate a pointer to the old handle
    Object obj = Reg().GetObject(handle);
    ASSERT_FALSE(obj.Exists());
}

// Test Pointer<T> comparison operators
TEST_F(CorePointerTests, TestPointerComparisonOperators) {
    // Create pointers
    Pointer<int> ptr1 = Reg().New<int>(1);
    Pointer<int> ptr2 = Reg().New<int>(2);
    Pointer<int> sameAsPtr1 = ptr1;
    
    // Equality operators
    ASSERT_TRUE(ptr1 == sameAsPtr1);
    ASSERT_FALSE(ptr1 == ptr2);
    
    ASSERT_FALSE(ptr1 != sameAsPtr1);
    ASSERT_TRUE(ptr1 != ptr2);
    
    // Null comparison
    Pointer<int> nullPtr;
    ASSERT_TRUE(nullPtr != ptr1);
    ASSERT_FALSE(nullPtr == ptr1);
    
    // Self-comparison
    ASSERT_TRUE(ptr1 == ptr1);
    ASSERT_FALSE(ptr1 != ptr1);
}

// Test Pointer<T> with container membership
TEST_F(CorePointerTests, TestPointerContainerMembership) {
    // Create an array
    Pointer<Array> array = Reg().New<Array>();
    
    // Create objects to store in the array
    Pointer<int> int1 = Reg().New<int>(1);
    Pointer<int> int2 = Reg().New<int>(2);
    Pointer<int> int3 = Reg().New<int>(3);
    
    // Add to array
    array->PushBack(int1);
    array->PushBack(int2);
    array->PushBack(int3);
    
    // Store array in root
    Root().Set("array", array);
    
    // Clear direct pointers
    int1 = Pointer<int>();
    int2 = Pointer<int>();
    int3 = Pointer<int>();
    
    // Run garbage collection
    Reg().GarbageCollect();
    
    // Retrieve array from root
    array = Root().Get("array");
    ASSERT_TRUE(array.Exists());
    ASSERT_EQ(array->Size(), 3);
    
    // Verify elements still exist
    Pointer<int> retrieved1 = array->At(0);
    Pointer<int> retrieved2 = array->At(1);
    Pointer<int> retrieved3 = array->At(2);
    
    ASSERT_TRUE(retrieved1.Exists());
    ASSERT_TRUE(retrieved2.Exists());
    ASSERT_TRUE(retrieved3.Exists());
    
    ASSERT_EQ(*retrieved1, 1);
    ASSERT_EQ(*retrieved2, 2);
    ASSERT_EQ(*retrieved3, 3);
}

// Test Pointer<T> with complex operations
TEST_F(CorePointerTests, TestPointerComplexOperations) {
    // Create map of pointers
    Pointer<Map> map = Reg().New<Map>();
    
    // Create pointers
    Pointer<int> int1 = Reg().New<int>(1);
    Pointer<String> str1 = Reg().New<String>("Hello");
    Pointer<float> float1 = Reg().New<float>(3.14f);
    
    // Add to map with string keys
    map->Insert(Reg().New<String>("int"), int1);
    map->Insert(Reg().New<String>("string"), str1);
    map->Insert(Reg().New<String>("float"), float1);
    
    // Store map in root
    Root().Set("map", map);
    
    // Clear direct pointers
    int1 = Pointer<int>();
    str1 = Pointer<String>();
    float1 = Pointer<float>();
    map = Pointer<Map>();
    
    // Run garbage collection
    Reg().GarbageCollect();
    
    // Retrieve map from root
    map = Root().Get("map");
    ASSERT_TRUE(map.Exists());
    ASSERT_EQ(map->Size(), 3);
    
    // Retrieve and verify pointers
    int1 = map->GetValue(Reg().New<String>("int"));
    str1 = map->GetValue(Reg().New<String>("string"));
    float1 = map->GetValue(Reg().New<String>("float"));
    
    ASSERT_TRUE(int1.Exists());
    ASSERT_TRUE(str1.Exists());
    ASSERT_TRUE(float1.Exists());
    
    ASSERT_EQ(*int1, 1);
    ASSERT_EQ(*str1, "Hello");
    ASSERT_FLOAT_EQ(*float1, 3.14f);
}

// Skip BasePointer test as it requires a different API
TEST_F(CorePointerTests, TestPointerToBaseTypes) {
    // Create pointers to basic types
    Pointer<int> intPtr = Reg().New<int>(42);
    Pointer<String> strPtr = Reg().New<String>("Hello");
    
    // Store values
    int intValue = *intPtr;
    String strValue = *strPtr;
    
    // Verify values
    ASSERT_EQ(intValue, 42);
    ASSERT_EQ(strValue, "Hello");
    
    // Modify values
    *intPtr = 100;
    *strPtr = "World";
    
    // Verify changes
    ASSERT_EQ(*intPtr, 100);
    ASSERT_EQ(*strPtr, "World");
}

// Test const reference to objects
TEST_F(CorePointerTests, TestConstReferences) {
    // Create objects
    Pointer<int> intPtr = Reg().New<int>(42);
    Pointer<String> strPtr = Reg().New<String>("Hello");
    
    // Create const references
    const int& constIntRef = *intPtr;
    const String& constStrRef = *strPtr;
    
    // Verify correct values
    ASSERT_EQ(constIntRef, 42);
    ASSERT_EQ(constStrRef, "Hello");
    
    // Modify through original pointers
    *intPtr = 100;
    *strPtr = "World";
    
    // Verify changes through const references
    ASSERT_EQ(constIntRef, 100);
    ASSERT_EQ(constStrRef, "World");
    
    // Const references prevent modification
    // This would not compile:
    // constIntRef = 200;
    // constStrRef = "Test";
}