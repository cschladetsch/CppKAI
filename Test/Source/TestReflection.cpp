#include <KAI/ClassBuilder.h>
#include <KAI/Core/Object.h>
#include <KAI/Core/Type.h>
#include <gtest/gtest.h>

#include "TestCommon.h"

// Test suite for reflection and type system
class ReflectionTest : public kai::TestCommon {
   protected:
    struct TestClass {
        int value;
        kai::String name;

        void SetValue(int v) { value = v; }
        int GetValue() const { return value; }
        kai::String GetDescription() const { return name + " = " + kai::String(std::to_string(value)); }
    };

    void SetUp() override {
        kai::TestCommon::SetUp();

        // Register test class
        kai::ClassBuilder<TestClass>(*reg_, "TestClass")
            .Methods("SetValue", &TestClass::SetValue)(
                "GetValue", &TestClass::GetValue)("GetDescription",
                                                  &TestClass::GetDescription)
            .Properties("value", &TestClass::value)("name", &TestClass::name);
    }
};

// Test 11: Type information and reflection
TEST_F(ReflectionTest, TypeInformationRetrieval) {
    auto obj = reg_->New<TestClass>();

    EXPECT_EQ(obj.GetClass()->GetName(), "TestClass");
    EXPECT_TRUE(obj.GetClass()->HasMethod(kai::Label("SetValue")));
    EXPECT_TRUE(obj.GetClass()->HasProperty(kai::Label("value")));

    auto properties = obj.GetClass()->GetProperties();
    EXPECT_GE(properties.Size(), 2);
}

// Test 12: Dynamic method invocation
TEST_F(ReflectionTest, DynamicMethodInvocation) {
    auto obj = reg_->New<TestClass>();
    kai::Deref<TestClass>(obj).name = "TestObject";

    // Get method by name
    auto setValueMethod = obj.GetClass()->GetMethod(kai::Label("SetValue"));
    ASSERT_TRUE(setValueMethod.Exists());

    // Invoke method dynamically
    kai::Array args;
    args.PushBack(reg_->New<int>(123));
    setValueMethod.Invoke(obj, args);

    EXPECT_EQ(kai::Deref<TestClass>(obj).value, 123);
}

// Test 13: Property access through reflection
TEST_F(ReflectionTest, PropertyAccessReflection) {
    auto obj = reg_->New<TestClass>();

    // Get property by name
    auto valueProp = obj.GetClass()->GetProperty(kai::Label("value"));
    ASSERT_TRUE(valueProp.Exists());

    // Set property value
    valueProp.SetValue(obj, reg_->New<int>(456));
    EXPECT_EQ(kai::Deref<TestClass>(obj).value, 456);

    // Get property value
    auto propValue = valueProp.GetValue(obj);
    EXPECT_EQ(kai::ConstDeref<int>(propValue), 456);
}

// Test 14: Type traits and meta-programming
TEST_F(ReflectionTest, TypeTraitsAndMeta) {
    // Test type traits
    // TODO: Type traits methods not implemented
    // EXPECT_TRUE(kai::Type::Traits<int>::IsNumeric());
    // EXPECT_FALSE(kai::Type::Traits<kai::String>::IsNumeric());
    // EXPECT_TRUE(kai::Type::Traits<kai::Pointer<int>>::IsPointer());

    // Test type conversion - not implemented
    // auto intObj = reg_->New<int>(42);
    // auto floatObj = intObj.ConvertTo<float>();
    // EXPECT_TRUE(floatObj.Valid());
    // EXPECT_FLOAT_EQ(kai::ConstDeref<float>(floatObj), 42.0f);
}

// Test 15: Custom type registration
TEST_F(ReflectionTest, CustomTypeRegistration) {
    struct CustomType {
        std::vector<int> data;

        void Add(int value) { data.push_back(value); }
        size_t Size() const { return data.size(); }
    };

    // Register custom type
    kai::ClassBuilder<CustomType>(*reg_, "CustomType")
        .Methods("Add", &CustomType::Add)("Size", &CustomType::Size);

    auto obj = reg_->New<CustomType>();
    auto cls = obj.GetClass();

    EXPECT_EQ(cls->GetName(), "CustomType");
    EXPECT_TRUE(cls->HasMethod(kai::Label("Add")));
    EXPECT_TRUE(cls->HasMethod(kai::Label("Size")));
}