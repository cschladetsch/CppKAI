#include <gtest/gtest.h>
#include "KAI/Core/Object.h"
#include "KAI/Core/Type.h"
#include "KAI/Core/ClassBuilder.h"
#include "TestCommon.h"

// Test suite for reflection and type system
class ReflectionTest : public TestCommon {
protected:
    struct TestClass {
        int value;
        String name;
        
        void SetValue(int v) { value = v; }
        int GetValue() const { return value; }
        String GetDescription() const { return name + " = " + String(value); }
    };
    
    void SetUp() override {
        TestCommon::SetUp();
        
        // Register test class
        ClassBuilder<TestClass>(*reg, "TestClass")
            .Methods
                ("SetValue", &TestClass::SetValue)
                ("GetValue", &TestClass::GetValue)
                ("GetDescription", &TestClass::GetDescription)
            .Properties
                ("value", &TestClass::value)
                ("name", &TestClass::name)
            ;
    }
};

// Test 11: Type information and reflection
TEST_F(ReflectionTest, TypeInformationRetrieval) {
    auto obj = reg->New<TestClass>();
    
    EXPECT_EQ(obj.GetClass()->GetName(), "TestClass");
    EXPECT_TRUE(obj.GetClass()->HasMethod(Label("SetValue")));
    EXPECT_TRUE(obj.GetClass()->HasProperty(Label("value")));
    
    auto properties = obj.GetClass()->GetProperties();
    EXPECT_GE(properties.Size(), 2);
}

// Test 12: Dynamic method invocation
TEST_F(ReflectionTest, DynamicMethodInvocation) {
    auto obj = reg->New<TestClass>();
    Deref<TestClass>(obj).name = "TestObject";
    
    // Get method by name
    auto setValueMethod = obj.GetClass()->GetMethod(Label("SetValue"));
    ASSERT_TRUE(setValueMethod.Exists());
    
    // Invoke method dynamically
    Array args;
    args.PushBack(reg->New<int>(123));
    setValueMethod.Invoke(obj, args);
    
    EXPECT_EQ(Deref<TestClass>(obj).value, 123);
}

// Test 13: Property access through reflection
TEST_F(ReflectionTest, PropertyAccessReflection) {
    auto obj = reg->New<TestClass>();
    
    // Get property by name
    auto valueProp = obj.GetClass()->GetProperty(Label("value"));
    ASSERT_TRUE(valueProp.Exists());
    
    // Set property value
    valueProp.SetValue(obj, reg->New<int>(456));
    EXPECT_EQ(Deref<TestClass>(obj).value, 456);
    
    // Get property value
    auto propValue = valueProp.GetValue(obj);
    EXPECT_EQ(ConstDeref<int>(propValue), 456);
}

// Test 14: Type traits and meta-programming
TEST_F(ReflectionTest, TypeTraitsAndMeta) {
    // Test type traits
    EXPECT_TRUE(Type::Traits<int>::IsNumeric());
    EXPECT_FALSE(Type::Traits<String>::IsNumeric());
    EXPECT_TRUE(Type::Traits<Pointer<int>>::IsPointer());
    
    // Test type conversion
    auto intObj = reg->New<int>(42);
    auto floatObj = intObj.ConvertTo<float>();
    EXPECT_TRUE(floatObj.Valid());
    EXPECT_FLOAT_EQ(ConstDeref<float>(floatObj), 42.0f);
}

// Test 15: Custom type registration
TEST_F(ReflectionTest, CustomTypeRegistration) {
    struct CustomType {
        std::vector<int> data;
        
        void Add(int value) { data.push_back(value); }
        size_t Size() const { return data.size(); }
    };
    
    // Register custom type
    ClassBuilder<CustomType>(*reg, "CustomType")
        .Methods
            ("Add", &CustomType::Add)
            ("Size", &CustomType::Size)
        ;
    
    auto obj = reg->New<CustomType>();
    auto cls = obj.GetClass();
    
    EXPECT_EQ(cls->GetName(), "CustomType");
    EXPECT_TRUE(cls->HasMethod(Label("Add")));
    EXPECT_TRUE(cls->HasMethod(Label("Size")));
}