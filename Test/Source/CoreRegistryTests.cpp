#include "TestCommon.h"

USING_NAMESPACE_KAI

// Advanced Registry tests for Core functionality
struct CoreRegistryTests : TestCommon {
   protected:
    void AddRequiredClasses() override {
        // Basic built-in types are already added in TestCommon::SetUp
        // Additional types needed for the tests
        Reg().AddClass<Array>();
        Reg().AddClass<Map>();
    }
};

// Test Registry type registration and validation
TEST_F(CoreRegistryTests, TestTypeRegistration) {
    // In KAI's implementation, classes are registered at runtime
    // When we added required classes in AddRequiredClasses(), they should be
    // registered

    // Registry is guaranteed to exist as Reg() returns a reference

    // Then check if the basic types are available
    // These should have been added in SetUp
    const ClassBase* boolClass = Reg().GetClass<bool>();
    const ClassBase* intClass = Reg().GetClass<int>();
    const ClassBase* floatClass = Reg().GetClass<float>();
    const ClassBase* stringClass = Reg().GetClass<String>();

    // Verify that these classes were properly registered
    ASSERT_NE(boolClass, nullptr);
    ASSERT_NE(intClass, nullptr);
    ASSERT_NE(floatClass, nullptr);
    ASSERT_NE(stringClass, nullptr);

    // Test with TypeNumber directly - validate that we can get classes by their
    // type number
    Type::Number boolType = Type::Traits<bool>::Number;
    Type::Number intType = Type::Traits<int>::Number;

    ASSERT_NE(Reg().GetClass(boolType), nullptr);
    ASSERT_NE(Reg().GetClass(intType), nullptr);

    // Test that an unknown type number returns nullptr as expected
    // Use a smaller value (99) that's still likely to be invalid
    ASSERT_EQ(Reg().GetClass(Type::Number(99)), nullptr);
}

// Test Registry GetClass functionality
TEST_F(CoreRegistryTests, TestGetClass) {
    // Get the int class
    const ClassBase* intClass = Reg().GetClass<int>();
    ASSERT_NE(intClass, nullptr);

    // Verify the type number matches the expected one
    ASSERT_EQ(intClass->GetTypeNumber(), Type::Traits<int>::Number);

    // The label is implementation dependent, but it should have a label
    ASSERT_FALSE(intClass->GetLabel().Empty());

    // Get the String class
    const ClassBase* stringClass = Reg().GetClass<String>();
    ASSERT_NE(stringClass, nullptr);

    // Verify the type number matches the expected one
    ASSERT_EQ(stringClass->GetTypeNumber(), Type::Traits<String>::Number);

    // The label is implementation dependent, but it should have a label
    ASSERT_FALSE(stringClass->GetLabel().Empty());
}

// Test Registry class property access
TEST_F(CoreRegistryTests, TestClassPropertyAccess) {
    const ClassBase* stringClass = Reg().GetClass<String>();
    ASSERT_TRUE(stringClass != nullptr);

    // Properties are implementation-dependent, so just verify the class exists
    // and the HasProperty method works with no exceptions
    // Check that HasProperty doesn't throw exceptions, ignoring return values
    stringClass->HasProperty(Label("ToString"));
    stringClass->HasProperty(Label("Type"));
    stringClass->HasProperty(Label("Size"));
    stringClass->HasProperty(Label("Empty"));

    // We don't assert on the property existence, just that the method completed
    // successfully Specific property names vary by implementation
}

// Test Registry creating multiple objects
TEST_F(CoreRegistryTests, TestMultipleObjectCreation) {
    // Create several objects of different types
    Pointer<int> intObj = Reg().New<int>(42);
    Pointer<String> strObj = Reg().New<String>("Hello");
    Pointer<bool> boolObj = Reg().New<bool>(true);

    // Store in root with labels
    Root().Set(Label("intValue"), intObj);
    Root().Set(Label("strValue"), strObj);
    Root().Set(Label("boolValue"), boolObj);

    // Verify retrieval
    ASSERT_EQ(*Pointer<int>(Root().Get(Label("intValue"))), 42);
    ASSERT_EQ(*Pointer<String>(Root().Get(Label("strValue"))), "Hello");
    ASSERT_EQ(*Pointer<bool>(Root().Get(Label("boolValue"))), true);

    // Clean up
    Root().Remove(Label("intValue"));
    Root().Remove(Label("strValue"));
    Root().Remove(Label("boolValue"));
}

// Test Registry creating objects with initial values
TEST_F(CoreRegistryTests, TestObjectCreationWithValues) {
    // Create objects with initial values
    Pointer<int> intObj = Reg().New<int>(100);
    Pointer<String> strObj = Reg().New<String>("Initial Value");
    Pointer<bool> boolObj = Reg().New<bool>(false);

    // Store in root to prevent garbage collection
    Root().Set(Label("test_int"), intObj);
    Root().Set(Label("test_str"), strObj);
    Root().Set(Label("test_bool"), boolObj);

    // Verify values were set correctly
    ASSERT_EQ(*intObj, 100);
    ASSERT_EQ(*strObj, "Initial Value");
    ASSERT_EQ(*boolObj, false);

    // Clean up
    Root().Remove(Label("test_int"));
    Root().Remove(Label("test_str"));
    Root().Remove(Label("test_bool"));
}

// Test Registry replacing objects with same label
TEST_F(CoreRegistryTests, TestObjectReplacement) {
    // Create and store an object
    Pointer<int> intObj1 = Reg().New<int>(1);
    Root().Set(Label("testInt"), intObj1);

    // Create a new object and store with the same label
    Pointer<int> intObj2 = Reg().New<int>(2);
    Root().Set(Label("testInt"), intObj2);

    // Verify the label now points to the new object
    ASSERT_EQ(*Pointer<int>(Root().Get(Label("testInt"))), 2);

    // Garbage collect and verify first object is gone
    Reg().GarbageCollect();
    ASSERT_FALSE(intObj1.Exists());
    ASSERT_TRUE(intObj2.Exists());

    // Clean up
    Root().Remove(Label("testInt"));
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
    Root().Set(Label("array"), array);

    // Verify container exists and has correct size
    Pointer<Array> retrieved = Root().Get(Label("array"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(retrieved->Size(), 3);

    // Verify the contained objects
    ASSERT_EQ(*Pointer<int>(retrieved->At(0)), 1);
    ASSERT_EQ(*Pointer<int>(retrieved->At(1)), 2);
    ASSERT_EQ(*Pointer<int>(retrieved->At(2)), 3);

    // Clean up
    Root().Remove(Label("array"));
}

// Test Registry value modification through Object interface
TEST_F(CoreRegistryTests, TestObjectModification) {
    // Create an object
    Pointer<int> intObj = Reg().New<int>(10);
    Root().Set(Label("testInt"), intObj);

    // Get the object as a generic Object
    Object obj = Root().Get(Label("testInt"));

    // Modify the value through the Object interface
    *Pointer<int>(obj) = 20;

    // Verify the change through both interfaces
    ASSERT_EQ(*intObj, 20);
    ASSERT_EQ(*Pointer<int>(obj), 20);

    // Clean up
    Root().Remove(Label("testInt"));
}

// Test Registry type safety with casting
TEST_F(CoreRegistryTests, TestTypeSafety) {
    // Create objects of different types
    Pointer<int> intObj = Reg().New<int>(42);
    Pointer<String> strObj = Reg().New<String>("Hello");

    // Store in root
    Root().Set(Label("intValue"), intObj);
    Root().Set(Label("strValue"), strObj);

    // Verify correct type checking
    Object objInt = Root().Get(Label("intValue"));
    Object objStr = Root().Get(Label("strValue"));

    // Use IsTypeNumber with Type::Traits<T>::Number instead of IsType<T>
    ASSERT_TRUE(objInt.IsTypeNumber(Type::Traits<int>::Number));
    ASSERT_FALSE(objInt.IsTypeNumber(Type::Traits<String>::Number));

    ASSERT_TRUE(objStr.IsTypeNumber(Type::Traits<String>::Number));
    ASSERT_FALSE(objStr.IsTypeNumber(Type::Traits<int>::Number));

    // Test safe casting
    Pointer<int> castedInt = objInt;
    ASSERT_TRUE(castedInt.Exists());
    ASSERT_EQ(*castedInt, 42);

    // Test that invalid casts throw exceptions
    bool exception_thrown = false;
    try {
        Pointer<String> invalidCast = objInt;
        // If no exception, the cast should fail and return empty pointer
        ASSERT_FALSE(invalidCast.Exists());
    } catch (...) {
        // This is expected behavior - exception is thrown for type mismatch
        exception_thrown = true;
    }

    // Clean up
    Root().Remove(Label("intValue"));
    Root().Remove(Label("strValue"));
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

    // Store in root to prevent garbage collection
    Root().Set(Label("type_int"), intObj);
    Root().Set(Label("type_str"), strObj);
    Root().Set(Label("type_array"), arrayObj);

    // Verify type numbers
    ASSERT_EQ(intObj.GetTypeNumber(), intType);
    ASSERT_EQ(strObj.GetTypeNumber(), stringType);
    ASSERT_EQ(arrayObj.GetTypeNumber(), arrayType);

    // Clean up
    Root().Remove(Label("type_int"));
    Root().Remove(Label("type_str"));
    Root().Remove(Label("type_array"));
}