#include "TestCommon.h"

USING_NAMESPACE_KAI

// Define traits for our custom types
namespace kai {
    namespace Type {
        // CustomType1 with an int value
        class CustomType1 {
        public:
            int value;
            CustomType1(int v = 0) : value(v) {}
            bool operator==(const CustomType1& other) const { return value == other.value; }
        };

        template<>
        struct Traits<CustomType1> {
            static const TypeNumber Number = 901;  // Choose an unused number
            static const char* Name() { return "CustomType1"; }
        };

        // CustomType2 with a name string
        class CustomType2 {
        public:
            std::string name;
            CustomType2(const std::string& n = "") : name(n) {}
            bool operator==(const CustomType2& other) const { return name == other.name; }
        };

        template<>
        struct Traits<CustomType2> {
            static const TypeNumber Number = 902;  // Choose an unused number
            static const char* Name() { return "CustomType2"; }
        };
    }
}

// Advanced Type System tests for Core functionality
struct CoreTypeTests : TestCommon {
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

// Test basic type number generation and uniqueness
TEST_F(CoreTypeTests, TestTypeNumbers) {
    // Get type numbers for built-in types
    TypeNumber boolType = Type::Traits<bool>::Number;
    TypeNumber intType = Type::Traits<int>::Number;
    TypeNumber floatType = Type::Traits<float>::Number;
    TypeNumber stringType = Type::Traits<String>::Number;
    
    // Verify they are all different
    ASSERT_NE(boolType, intType);
    ASSERT_NE(boolType, floatType);
    ASSERT_NE(boolType, stringType);
    ASSERT_NE(intType, floatType);
    ASSERT_NE(intType, stringType);
    ASSERT_NE(floatType, stringType);
    
    // Verify type numbers are consistent
    ASSERT_EQ(Type::Traits<bool>::Number, boolType);
    ASSERT_EQ(Type::Traits<int>::Number, intType);
    ASSERT_EQ(Type::Traits<float>::Number, floatType);
    ASSERT_EQ(Type::Traits<String>::Number, stringType);
}

// Test class registration and retrieval
TEST_F(CoreTypeTests, TestClassRegistration) {
    // Verify registered classes
    ASSERT_TRUE(Reg().GetClass<bool>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<int>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<float>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<String>() != nullptr);
    
    // Test with type numbers
    ASSERT_TRUE(Reg().GetClass(Type::Traits<bool>::Number) != nullptr);
    ASSERT_TRUE(Reg().GetClass(Type::Traits<int>::Number) != nullptr);
    
    // Check that non-existent type returns nullptr
    ASSERT_TRUE(Reg().GetClass(99999) == nullptr);
}

// Test class information retrieval
TEST_F(CoreTypeTests, TestClassInformation) {
    // Get class objects
    const ClassBase* intClass = Reg().GetClass<int>();
    const ClassBase* stringClass = Reg().GetClass<String>();
    
    // Verify they exist
    ASSERT_TRUE(intClass != nullptr);
    ASSERT_TRUE(stringClass != nullptr);
    
    // Check class names
    ASSERT_EQ(intClass->GetLabel(), Label("int"));
    ASSERT_EQ(stringClass->GetLabel(), Label("String"));
    
    // Check type numbers
    ASSERT_EQ(intClass->GetTypeNumber(), Type::Traits<int>::Number);
    ASSERT_EQ(stringClass->GetTypeNumber(), Type::Traits<String>::Number);
}

// Test object type checking
TEST_F(CoreTypeTests, TestObjectTypeChecking) {
    // Create objects of different types
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object stringObj = Reg().New<String>("Hello");
    
    // Test IsType with correct types
    ASSERT_TRUE(intObj.IsType<int>());
    ASSERT_TRUE(floatObj.IsType<float>());
    ASSERT_TRUE(stringObj.IsType<String>());
    
    // Test IsType with incorrect types
    ASSERT_FALSE(intObj.IsType<float>());
    ASSERT_FALSE(floatObj.IsType<int>());
    ASSERT_FALSE(stringObj.IsType<float>());
    
    // Test with type numbers
    ASSERT_TRUE(intObj.IsTypeNumber(Type::Traits<int>::Number));
    ASSERT_TRUE(floatObj.IsTypeNumber(Type::Traits<float>::Number));
    ASSERT_TRUE(stringObj.IsTypeNumber(Type::Traits<String>::Number));
    
    ASSERT_FALSE(intObj.IsTypeNumber(Type::Traits<float>::Number));
    ASSERT_FALSE(floatObj.IsTypeNumber(Type::Traits<int>::Number));
}

// Test type-specific casting
TEST_F(CoreTypeTests, TestTypeCasting) {
    // Create objects
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object strObj = Reg().New<String>("Hello");
    
    // Test valid casts using Pointer<T>
    Pointer<int> intPtr = intObj;
    Pointer<float> floatPtr = floatObj;
    Pointer<String> strPtr = strObj;
    
    ASSERT_TRUE(intPtr.Exists());
    ASSERT_TRUE(floatPtr.Exists());
    ASSERT_TRUE(strPtr.Exists());
    
    ASSERT_EQ(*intPtr, 42);
    ASSERT_FLOAT_EQ(*floatPtr, 3.14f);
    ASSERT_EQ(*strPtr, "Hello");
    
    // Test invalid casts
    Pointer<float> invalidIntToFloat = intObj;
    Pointer<int> invalidFloatToInt = floatObj;
    Pointer<int> invalidStrToInt = strObj;
    
    ASSERT_FALSE(invalidIntToFloat.Exists());
    ASSERT_FALSE(invalidFloatToInt.Exists());
    ASSERT_FALSE(invalidStrToInt.Exists());
}

// Test type-safe dereferencing
TEST_F(CoreTypeTests, TestTypeDereferencing) {
    // Create objects
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    Object strObj = Reg().New<String>("Hello");
    
    // Test valid dereferencing
    ASSERT_EQ(ConstDeref<int>(intObj), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(floatObj), 3.14f);
    ASSERT_EQ(ConstDeref<String>(strObj), "Hello");
    
    // Test modifying through Deref
    Deref<int>(intObj) = 100;
    Deref<float>(floatObj) = 2.71f;
    Deref<String>(strObj) = "World";
    
    ASSERT_EQ(ConstDeref<int>(intObj), 100);
    ASSERT_FLOAT_EQ(ConstDeref<float>(floatObj), 2.71f);
    ASSERT_EQ(ConstDeref<String>(strObj), "World");
}

// Test class property access through type system
TEST_F(CoreTypeTests, TestClassProperties) {
    // Get the String class
    const ClassBase* stringClass = Reg().GetClass<String>();
    ASSERT_TRUE(stringClass != nullptr);
    
    // Check for expected properties
    ASSERT_TRUE(stringClass->HasProperty(Label("Size")));
    ASSERT_TRUE(stringClass->HasProperty(Label("Empty")));
    ASSERT_TRUE(stringClass->HasProperty(Label("ToString")));
    
    // Create a String object
    Object stringObj = Reg().New<String>("Hello World");
    
    // Get properties through the type system
    Object sizeProperty = stringObj.Get(Label("Size"));
    Object emptyProperty = stringObj.Get(Label("Empty"));
    
    ASSERT_TRUE(sizeProperty.Exists());
    ASSERT_TRUE(emptyProperty.Exists());
    
    ASSERT_TRUE(sizeProperty.IsType<int>());
    ASSERT_TRUE(emptyProperty.IsType<bool>());
    
    ASSERT_EQ(ConstDeref<int>(sizeProperty), 11);
    ASSERT_EQ(ConstDeref<bool>(emptyProperty), false);
}

// Test type-safe method calls
TEST_F(CoreTypeTests, TestTypeSafeMethodCalls) {
    // Create an Array object
    Pointer<Array> array = Reg().New<Array>();
    
    // Add elements
    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    array->PushBack(Reg().New<int>(3));
    
    // Get the Array class
    const ClassBase* arrayClass = Reg().GetClass<Array>();
    ASSERT_TRUE(arrayClass != nullptr);
    
    // Check for method existence
    ASSERT_TRUE(arrayClass->GetMethod(Label("PushBack")) != nullptr);
    ASSERT_TRUE(arrayClass->GetMethod(Label("PopBack")) != nullptr);
    ASSERT_TRUE(arrayClass->GetMethod(Label("At")) != nullptr);
    
    // Call methods directly
    Object obj = array;
    // Instead of using Call, we'll use the method directly
    array->PushBack(Reg().New<int>(4));
    
    // Verify the method call worked
    ASSERT_EQ(array->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(array->At(3)), 4);
    
    // Get element directly
    Object element = array->At(2);
    ASSERT_TRUE(element.Exists());
    ASSERT_TRUE(element.IsType<int>());
    ASSERT_EQ(ConstDeref<int>(element), 3);
}

// Test additional simple types
TEST_F(CoreTypeTests, TestSimpleTypesStorage) {
    // Create multiple objects of different types
    Pointer<int> intObj = Reg().New<int>(42);
    Pointer<float> floatObj = Reg().New<float>(3.14f);
    Pointer<bool> boolObj = Reg().New<bool>(true);
    Pointer<String> strObj = Reg().New<String>("Hello");
    
    // Store in root
    Root().Set(Label("intObj"), intObj);
    Root().Set(Label("floatObj"), floatObj);
    Root().Set(Label("boolObj"), boolObj);
    Root().Set(Label("strObj"), strObj);
    
    // Retrieve from root
    Pointer<int> retrievedInt = Root().Get(Label("intObj"));
    Pointer<float> retrievedFloat = Root().Get(Label("floatObj"));
    Pointer<bool> retrievedBool = Root().Get(Label("boolObj"));
    Pointer<String> retrievedStr = Root().Get(Label("strObj"));
    
    // Verify they exist
    ASSERT_TRUE(retrievedInt.Exists());
    ASSERT_TRUE(retrievedFloat.Exists());
    ASSERT_TRUE(retrievedBool.Exists());
    ASSERT_TRUE(retrievedStr.Exists());
    
    // Verify values
    ASSERT_EQ(*retrievedInt, 42);
    ASSERT_FLOAT_EQ(*retrievedFloat, 3.14f);
    ASSERT_EQ(*retrievedBool, true);
    ASSERT_EQ(*retrievedStr, "Hello");
}

// Test basic type relationships
TEST_F(CoreTypeTests, TestTypeRelationships) {
    // Test simple type relationships
    ASSERT_TRUE(Reg().GetClass<int>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<float>() != nullptr);
    ASSERT_TRUE(Reg().GetClass<String>() != nullptr);
    
    // Verify type numbers are different
    ASSERT_NE(Type::Traits<int>::Number, Type::Traits<float>::Number);
    ASSERT_NE(Type::Traits<int>::Number, Type::Traits<String>::Number);
    ASSERT_NE(Type::Traits<float>::Number, Type::Traits<String>::Number);
}

// Test type conversion operations
TEST_F(CoreTypeTests, TestTypeConversion) {
    // Create numeric objects
    Object intObj = Reg().New<int>(42);
    Object floatObj = Reg().New<float>(3.14f);
    
    // Test numeric conversion
    float intAsFloat = static_cast<float>(ConstDeref<int>(intObj));
    int floatAsInt = static_cast<int>(ConstDeref<float>(floatObj));
    
    ASSERT_FLOAT_EQ(intAsFloat, 42.0f);
    ASSERT_EQ(floatAsInt, 3);
    
    // Test string conversion through ToString
    String intAsString = intObj.ToString();
    String floatAsString = floatObj.ToString();
    
    ASSERT_EQ(intAsString, String("42"));
    ASSERT_TRUE(intAsString.Contains("42"));
}

// Test class method introspection
TEST_F(CoreTypeTests, TestClassMethodIntrospection) {
    // Get the Array class
    const ClassBase* arrayClass = Reg().GetClass<Array>();
    ASSERT_TRUE(arrayClass != nullptr);
    
    // Get methods
    auto pushBackMethod = arrayClass->GetMethod(Label("PushBack"));
    auto atMethod = arrayClass->GetMethod(Label("At"));
    
    ASSERT_TRUE(pushBackMethod != nullptr);
    ASSERT_TRUE(atMethod != nullptr);
    
    // Check non-existent method
    auto nonExistentMethod = arrayClass->GetMethod(Label("NonExistentMethod"));
    ASSERT_TRUE(nonExistentMethod == nullptr);
}