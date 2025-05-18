#include "TestCommon.h"

USING_NAMESPACE_KAI

// Advanced Object tests for Core functionality
struct CoreObjectTests : TestCommon {
protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        // Reg().AddClass<double>(); // Removing due to linking issues
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
        Reg().AddClass<Map>();
    }
};

// Test basic Object creation and validation
TEST_F(CoreObjectTests, TestObjectCreation) {
    // Create objects of different types
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object strObj = Reg().New<String>("Hello World");
    
    // Verify they exist and have the correct types
    ASSERT_TRUE(intObj.Exists());
    ASSERT_TRUE(floatObj.Exists());
    ASSERT_TRUE(strObj.Exists());
    
    ASSERT_TRUE(intObj.IsType<int>());
    ASSERT_TRUE(floatObj.IsType<float>());
    ASSERT_TRUE(strObj.IsType<String>());
}

// Test Object handle management
TEST_F(CoreObjectTests, TestObjectHandleManagement) {
    // Create an object and get its handle
    Object intObj = Reg().New<int>(42);
    Handle handle = intObj.GetHandle();
    
    // Create another Object referencing the same handle
    Object sameObj = Reg().GetObject(handle);
    
    // Verify both objects refer to the same data
    ASSERT_TRUE(intObj.Exists());
    ASSERT_TRUE(sameObj.Exists());
    ASSERT_EQ(intObj.GetHandle(), sameObj.GetHandle());
    ASSERT_EQ(*Pointer<int>(intObj), *Pointer<int>(sameObj));
    
    // Modify through one Object
    *Pointer<int>(intObj) = 100;
    
    // Verify change is visible through the other
    ASSERT_EQ(*Pointer<int>(sameObj), 100);
}

// Test Object dereferencing
TEST_F(CoreObjectTests, TestObjectDereferencing) {
    // Create objects
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object strObj = Reg().New<String>("Hello");
    
    // Test correct dereferencing
    ASSERT_EQ(ConstDeref<int>(intObj), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(floatObj), 3.14f);
    ASSERT_EQ(ConstDeref<String>(strObj), "Hello");
    
    // Modify through dereferencing
    Deref<int>(intObj) = 100;
    Deref<float>(floatObj) = 2.71f;
    Deref<String>(strObj) = "World";
    
    // Verify changes
    ASSERT_EQ(ConstDeref<int>(intObj), 100);
    ASSERT_FLOAT_EQ(ConstDeref<float>(floatObj), 2.71f);
    ASSERT_EQ(ConstDeref<String>(strObj), "World");
}

// Test Object assignment operators
TEST_F(CoreObjectTests, TestObjectAssignment) {
    // Create source objects
    Object intObj1 = Reg().New<int>(42);
    Object intObj2 = Reg().New<int>(100);
    
    // Test assignment operator
    intObj2 = intObj1;
    
    // Verify both now refer to the same object
    ASSERT_EQ(intObj1.GetHandle(), intObj2.GetHandle());
    ASSERT_EQ(*Pointer<int>(intObj1), *Pointer<int>(intObj2));
    
    // Modify and verify both see the change
    *Pointer<int>(intObj1) = 200;
    ASSERT_EQ(*Pointer<int>(intObj2), 200);
}

// Test Object comparison operators
TEST_F(CoreObjectTests, TestObjectComparison) {
    // Create test objects
    Object intObj1 = Reg().New<int>(42);
    Object intObj2 = Reg().New<int>(42);
    Object intObj3 = Reg().New<int>(100);
    
    // Create a reference to the first object
    Object sameAsIntObj1 = Reg().GetObject(intObj1.GetHandle());
    
    // Test equality operators
    ASSERT_TRUE(intObj1 == sameAsIntObj1);  // Same handle
    ASSERT_FALSE(intObj1 == intObj2);       // Different handles but same value
    ASSERT_FALSE(intObj1 == intObj3);       // Different handles and values
    
    ASSERT_FALSE(intObj1 != sameAsIntObj1);
    ASSERT_TRUE(intObj1 != intObj2);
    ASSERT_TRUE(intObj1 != intObj3);
}

// Test Object wrapper methods
TEST_F(CoreObjectTests, TestObjectWrapperMethods) {
    // Create an object
    Object strObj = Reg().New<String>("Hello");
    
    // Test type checking
    ASSERT_TRUE(strObj.IsType<String>());
    ASSERT_FALSE(strObj.IsType<int>());
    
    // Test class retrieval
    auto cls = Reg().GetClass<String>();
    ASSERT_EQ(cls->GetLabel(), Label("String"));
    
    // Test existence check
    ASSERT_TRUE(strObj.Exists());
    
    // Test null object
    Object nullObj;
    ASSERT_FALSE(nullObj.Exists());
}

// Test Object ToString functionality
TEST_F(CoreObjectTests, TestObjectToString) {
    // Create objects of different types
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object strObj = Reg().New<String>("Hello");
    Object boolObj = Reg().New<bool>(true);
    
    // Test ToString results
    ASSERT_EQ(intObj.ToString(), "42");
    ASSERT_EQ(strObj.ToString(), "Hello");
    ASSERT_EQ(boolObj.ToString(), "true");
    
    // Float ToString may vary slightly by platform, so check contains main digits
    String floatStr = floatObj.ToString();
    ASSERT_TRUE(floatStr.Contains("3.14"));
}

// Test Object property access
TEST_F(CoreObjectTests, TestObjectPropertyAccess) {
    // Create a String object
    Object strObj = Reg().New<String>("Hello World");
    
    // Test retrieving properties through the Object interface
    Object sizeProperty = strObj.Get("Size");
    ASSERT_TRUE(sizeProperty.Exists());
    ASSERT_TRUE(sizeProperty.IsType<int>());
    ASSERT_EQ(ConstDeref<int>(sizeProperty), 11);  // Length of "Hello World"
    
    // Test empty property
    Object emptyProperty = strObj.Get("Empty");
    ASSERT_TRUE(emptyProperty.Exists());
    ASSERT_TRUE(emptyProperty.IsType<bool>());
    ASSERT_EQ(ConstDeref<bool>(emptyProperty), false);  // String is not empty
}

// Test Object methods through Pointer interface
TEST_F(CoreObjectTests, TestObjectMethodUsage) {
    // Create an Array object
    Pointer<Array> array = Reg().New<Array>();
    
    // Add elements directly through Pointer interface
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    
    // Check array size
    ASSERT_EQ(array->Size(), 3);
    
    // Access elements
    Object firstElement = array->At(0);
    Object secondElement = array->At(1);
    Object thirdElement = array->At(2);
    
    ASSERT_TRUE(firstElement.Exists());
    ASSERT_TRUE(secondElement.Exists());
    ASSERT_TRUE(thirdElement.Exists());
    
    ASSERT_EQ(ConstDeref<int>(firstElement), 1);
    ASSERT_EQ(ConstDeref<int>(secondElement), 2);
    ASSERT_EQ(ConstDeref<int>(thirdElement), 3);
}

// Test Object lifetime management
TEST_F(CoreObjectTests, TestObjectLifetimeManagement) {
    // Create objects
    Object obj1 = Reg().New<int>(1);
    Handle handle = obj1.GetHandle();
    
    // Create a weak reference
    Object weakRef = Reg().GetObject(handle);
    ASSERT_TRUE(weakRef.Exists());
    
    // Clear the original reference
    obj1 = Object();
    
    // Garbage collect - weakRef should still be valid as it holds a reference
    Reg().GarbageCollect();
    ASSERT_TRUE(weakRef.Exists());
    
    // Clear the weak reference
    weakRef = Object();
    
    // Garbage collect - now the object should be gone
    Reg().GarbageCollect();
    
    // Try to recreate an object with the old handle
    Object testObj = Reg().GetObject(handle);
    ASSERT_FALSE(testObj.Exists());
}

// Test Object marking for garbage collection
TEST_F(CoreObjectTests, TestObjectGCMarking) {
    // Create an object that's not stored in the tree
    Object tempObj = Reg().New<int>(42);
    
    // Verify it exists
    ASSERT_TRUE(tempObj.Exists());
    
    // Garbage collect
    Reg().GarbageCollect();
    
    // Object should be gone as it's not referenced from the tree
    ASSERT_FALSE(tempObj.Exists());
    
    // Create another object and store it in the tree
    Object storedObj = Reg().New<int>(100);
    Root().Set("storedObj", storedObj);
    
    // Clear the direct reference
    Handle handle = storedObj.GetHandle();
    storedObj = Object();
    
    // Garbage collect
    Reg().GarbageCollect();
    
    // Object should still exist as it's referenced from the tree
    Object retrievedObj = Reg().GetObject(handle);
    ASSERT_TRUE(retrievedObj.Exists());
    ASSERT_EQ(ConstDeref<int>(retrievedObj), 100);
}