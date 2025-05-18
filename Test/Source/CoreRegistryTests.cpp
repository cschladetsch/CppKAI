#include "TestCommon.h"

USING_NAMESPACE_KAI

// Advanced Registry tests for Core functionality
struct CoreRegistryTests : TestCommon {
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

// Test Registry type registration and validation
TEST_F(CoreRegistryTests, TestTypeRegistration) {
    // Verify that our required types are registered
    ASSERT_TRUE(Reg().GetClass<bool>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<int>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<float>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<String>() != nullptr);
    
    // Test with TypeNumber directly
    TypeNumber boolType = Type::Traits<bool>::Number;
    TypeNumber intType = Type::Traits<int>::Number;
    ASSERT_TRUE(Reg().GetClass(boolType) != nullptr);
    ASSERT_TRUE(Reg().GetClass(intType) != nullptr);
    
    // Test unknown type number
    ASSERT_TRUE(Reg().GetClass(99999) == nullptr);
}

// Test Registry GetClass functionality
TEST_F(CoreRegistryTests, TestGetClass) {
    // Get the int class
    auto intClass = Reg().GetClass<int>();
    ASSERT_TRUE(intClass != nullptr);
    ASSERT_EQ(intClass->GetTypeNumber(), Type::Traits<int>::Number);
    ASSERT_EQ(intClass->GetLabel(), Label("int"));
    
    // Get the String class
    auto stringClass = Reg().GetClass<String>();
    ASSERT_TRUE(stringClass != nullptr);
    ASSERT_EQ(stringClass->GetTypeNumber(), Type::Traits<String>::Number);
    ASSERT_EQ(stringClass->GetLabel(), Label("String"));
}

// Test Registry class property access
TEST_F(CoreRegistryTests, TestClassPropertyAccess) {
    auto stringClass = Reg().GetClass<String>();
    ASSERT_TRUE(stringClass != nullptr);
    
    // Test properties that should exist on all classes
    ASSERT_TRUE(stringClass->HasProperty(Label("ToString")));
    ASSERT_TRUE(stringClass->HasProperty(Label("Type")));
    
    // Test String-specific properties
    ASSERT_TRUE(stringClass->HasProperty(Label("Size")));
    ASSERT_TRUE(stringClass->HasProperty(Label("Empty")));
}

// Test Registry creating multiple objects
TEST_F(CoreRegistryTests, TestMultipleObjectCreation) {
    // Create several objects of different types
    Pointer<int> intObj = Reg().New<int>(42);
    Pointer<String> strObj = Reg().New<String>("Hello");
    Pointer<bool> boolObj = Reg().New<bool>(true);
    
    // Store in root with labels
    Root().Set("intValue", intObj);
    Root().Set("strValue", strObj);
    Root().Set("boolValue", boolObj);
    
    // Verify retrieval
    ASSERT_EQ(*Pointer<int>(Root().Get("intValue")), 42);
    ASSERT_EQ(*Pointer<String>(Root().Get("strValue")), "Hello");
    ASSERT_EQ(*Pointer<bool>(Root().Get("boolValue")), true);
}

// Test Registry creating objects with initial values
TEST_F(CoreRegistryTests, TestObjectCreationWithValues) {
    // Create objects with initial values
    Pointer<int> intObj = Reg().New<int>(100);
    Pointer<String> strObj = Reg().New<String>("Initial Value");
    Pointer<bool> boolObj = Reg().New<bool>(false);
    
    // Verify values were set correctly
    ASSERT_EQ(*intObj, 100);
    ASSERT_EQ(*strObj, "Initial Value");
    ASSERT_EQ(*boolObj, false);
}

// Test Registry replacing objects with same label
TEST_F(CoreRegistryTests, TestObjectReplacement) {
    // Create and store an object
    Pointer<int> intObj1 = Reg().New<int>(1);
    Root().Set("testInt", intObj1);
    
    // Create a new object and store with the same label
    Pointer<int> intObj2 = Reg().New<int>(2);
    Root().Set("testInt", intObj2);
    
    // Verify the label now points to the new object
    ASSERT_EQ(*Pointer<int>(Root().Get("testInt")), 2);
    
    // Garbage collect and verify first object is gone
    Reg().GarbageCollect();
    ASSERT_FALSE(intObj1.Exists());
    ASSERT_TRUE(intObj2.Exists());
}

// Test Registry with nested object structures
TEST_F(CoreRegistryTests, TestNestedObjects) {
    // Create a container
    Pointer<Array> array = Reg().New<Array>();
    
    // Add objects to the container
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    
    // Store the container in the root
    Root().Set("array", array);
    
    // Verify container exists and has correct size
    Pointer<Array> retrieved = Root().Get("array");
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(retrieved->Size(), 3);
    
    // Verify the contained objects
    ASSERT_EQ(*Pointer<int>(retrieved->At(0)), 1);
    ASSERT_EQ(*Pointer<int>(retrieved->At(1)), 2);
    ASSERT_EQ(*Pointer<int>(retrieved->At(2)), 3);
}

// Test Registry value modification through Object interface
TEST_F(CoreRegistryTests, TestObjectModification) {
    // Create an object
    Pointer<int> intObj = Reg().New<int>(10);
    Root().Set("testInt", intObj);
    
    // Get the object as a generic Object
    Object obj = Root().Get("testInt");
    
    // Modify the value through the Object interface
    *Pointer<int>(obj) = 20;
    
    // Verify the change through both interfaces
    ASSERT_EQ(*intObj, 20);
    ASSERT_EQ(*Pointer<int>(obj), 20);
}

// Test Registry type safety with casting
TEST_F(CoreRegistryTests, TestTypeSafety) {
    // Create objects of different types
    Pointer<int> intObj = Reg().New<int>(42);
    Pointer<String> strObj = Reg().New<String>("Hello");
    
    // Store in root
    Root().Set("intValue", intObj);
    Root().Set("strValue", strObj);
    
    // Verify correct type checking
    Object objInt = Root().Get("intValue");
    Object objStr = Root().Get("strValue");
    
    ASSERT_TRUE(objInt.IsType<int>());
    ASSERT_FALSE(objInt.IsType<String>());
    
    ASSERT_TRUE(objStr.IsType<String>());
    ASSERT_FALSE(objStr.IsType<int>());
    
    // Test safe casting
    Pointer<int> castedInt = objInt;
    ASSERT_TRUE(castedInt.Exists());
    ASSERT_EQ(*castedInt, 42);
    
    // Test that invalid casts fail
    Pointer<String> invalidCast = objInt;
    ASSERT_FALSE(invalidCast.Exists());
}

// Test TypeNumber retrieval and comparison
TEST_F(CoreRegistryTests, TestTypeNumbers) {
    // Get type numbers for different types
    TypeNumber intType = Type::Traits<int>::Number;
    TypeNumber stringType = Type::Traits<String>::Number;
    TypeNumber arrayType = Type::Traits<Array>::Number;
    
    // Verify they are different
    ASSERT_NE(intType, stringType);
    ASSERT_NE(intType, arrayType);
    ASSERT_NE(stringType, arrayType);
    
    // Create objects and verify their type numbers
    Pointer<int> intObj = Reg().New<int>();
    Pointer<String> strObj = Reg().New<String>();
    Pointer<Array> arrayObj = Reg().New<Array>();
    
    ASSERT_EQ(intObj.GetTypeNumber(), intType);
    ASSERT_EQ(strObj.GetTypeNumber(), stringType);
    ASSERT_EQ(arrayObj.GetTypeNumber(), arrayType);
}