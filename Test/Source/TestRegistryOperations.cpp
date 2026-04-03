#include "TestCommon.h"

USING_NAMESPACE_KAI

// Registry tests for operations on existing classes
struct TestRegistryOperations : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
    }
};

// Test Registry create and retrieve operations with simple types
TEST_F(TestRegistryOperations, TestCreateAndRetrieve) {
    // Create a new int object
    Pointer<int> obj = Reg().New<int>();
    *obj = 42;

    // Verify object was created correctly
    ASSERT_TRUE(obj.Exists());
    ASSERT_EQ(*obj, 42);

    // Store the object with a specific label in the root
    Label label("testObject");
    Root().Set(label, obj);

    // Retrieve object from the root
    Object retrieved = Root().Get(label);
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<int>());

    // Verify retrieved object has correct value
    Pointer<int> retrievedObj = retrieved;
    ASSERT_EQ(*retrievedObj, 42);
}

// Simplified registry object resolution test
TEST_F(TestRegistryOperations, TestObjectResolution) {
    // Create object and add to root
    Pointer<int> obj = Reg().New<int>();
    *obj = 123;

    // Name and add to root - ensure we use the same type of label for Set and
    // Get
    Label label("testObject");
    Root().Set(label, obj);

    // Test direct access
    Object retrieved = Root().Get(label);
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<int>());
    ASSERT_EQ(*Pointer<int>(retrieved), 123);
}

// Simplified scope test
TEST_F(TestRegistryOperations, TestScopeOperations) {
    // Create object
    Pointer<int> obj = Reg().New<int>();
    *obj = 42;

    // Add to root using consistent Label object
    Label label("testValue");
    Root().Set(label, obj);

    // Test direct access using the same Label object
    Object retrieved = Root().Get(label);
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<int>());
    ASSERT_EQ(*Pointer<int>(retrieved), 42);
}