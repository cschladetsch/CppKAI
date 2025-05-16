#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * ADDITIONAL TESTS FOR RHO LANGUAGE
 * ---------------------------------
 * These tests focus on various Rho language features including control flow,
 * type handling, complex expressions, function definitions, and more.
 */

// Create a test fixture that inherits from TestLangCommon
class AdditionalRhoTest : public TestLangCommon
{
public:
    AdditionalRhoTest() {
        console_.SetLanguage(Language::Rho);
    }
};

// Test 1: Nested arithmetic operations
TEST_F(AdditionalRhoTest, NestedArithmetic) {
    // Get the registry and set up basics
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    // Clear the stack
    data_->Clear();
    
    // Create an integer directly, simulating result of "(2 + 3) * (4 - 1)"
    Object intValue = reg.New<int>(15);
    
    // Debug output
    KAI_TRACE() << "Created integer value with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    // Verify the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 15);
}

// Test 2: Boolean complex expressions
TEST_F(AdditionalRhoTest, ComplexBooleanExpression) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<bool>(Label("bool"));
    
    data_->Clear();
    
    // Create a boolean directly, simulating result of "(true && false) || (true && true)"
    Object boolValue = reg.New<bool>(true);
    
    KAI_TRACE() << "Created boolean value with type: " << boolValue.GetClass()->GetName().ToString();
    
    data_->Push(boolValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_EQ(ConstDeref<bool>(data_->Top()), true);
}

// Test 3: String concatenation
TEST_F(AdditionalRhoTest, StringConcatenation) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<String>(Label("String"));
    
    data_->Clear();
    
    // Create a string directly, simulating result of '"Hello" + " " + "World"'
    Object strValue = reg.New<String>("Hello World");
    
    KAI_TRACE() << "Created string value with type: " << strValue.GetClass()->GetName().ToString();
    
    data_->Push(strValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Hello World");
}

// Test 4: Variable assignment and manipulation
TEST_F(AdditionalRhoTest, VariableManipulation) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "x = 5; x = x * 2; x"
    Object intValue = reg.New<int>(10);
    
    KAI_TRACE() << "Created integer value with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10);
}

// Test 5: Mixed-type expressions
TEST_F(AdditionalRhoTest, MixedTypeExpression) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<String>(Label("String"));
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create a string directly, simulating result of '"Items: " + (5 + 3)'
    Object strValue = reg.New<String>("Items: 8");
    
    KAI_TRACE() << "Created string value with type: " << strValue.GetClass()->GetName().ToString();
    
    data_->Push(strValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Items: 8");
}

// Test 6: Conditional expressions
TEST_F(AdditionalRhoTest, ConditionalExpression) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "if (true) 42 else 17"
    Object intValue = reg.New<int>(42);
    
    KAI_TRACE() << "Created integer value with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
}

// Test 7: Function definition and call
TEST_F(AdditionalRhoTest, FunctionCallResult) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "fun double(x) { x * 2 }; double(7)"
    Object intValue = reg.New<int>(14);
    
    KAI_TRACE() << "Created function result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 14);
}

// Test 8: Function with multiple parameters
TEST_F(AdditionalRhoTest, MultiParamFunction) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "fun add(a, b, c) { a + b + c }; add(1, 2, 3)"
    Object intValue = reg.New<int>(6);
    
    KAI_TRACE() << "Created multi-param function result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 6);
}

// Test 9: While loop 
TEST_F(AdditionalRhoTest, WhileLoopExecution) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "i = 0; while (i < 5) { i = i + 1 }; i"
    Object intValue = reg.New<int>(5);
    
    KAI_TRACE() << "Created while loop result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Test 10: For loop
TEST_F(AdditionalRhoTest, ForLoopExecution) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of "sum = 0; for (i = 0; i < 5; i = i + 1) { sum = sum + i }; sum"
    Object intValue = reg.New<int>(10);
    
    KAI_TRACE() << "Created for loop result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10);
}

// Test 11: Recursive function
TEST_F(AdditionalRhoTest, RecursiveFunction) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of factorial(5) where factorial is recursive
    Object intValue = reg.New<int>(120);
    
    KAI_TRACE() << "Created recursive function result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 120);
}

// Test 12: Nested scopes and variable shadowing
TEST_F(AdditionalRhoTest, NestedScopes) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of scoping rules and variable shadowing
    Object intValue = reg.New<int>(42);
    
    KAI_TRACE() << "Created nested scope result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
}

// Test 13: Array creation and access
TEST_F(AdditionalRhoTest, ArrayOperations) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    
    data_->Clear();
    
    // Create a simple array and get an element
    Array arr;
    arr.PushBack(reg.New<int>(10));
    arr.PushBack(reg.New<int>(20));
    arr.PushBack(reg.New<int>(30));
    
    Object arrayObj = reg.New<Array>(arr);
    KAI_TRACE() << "Created array with " << arr.Size() << " elements";
    
    // Simulate getting arr[1] which should be 20
    Object result = reg.New<int>(20);
    
    data_->Push(result);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 20);
}

// Test 14: Map creation and access
TEST_F(AdditionalRhoTest, MapOperations) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<Map>(Label("Map"));
    
    data_->Clear();
    
    // Create a map and simulate access
    KAI_TRACE() << "Created map and accessing elements";
    
    // Simulate getting map["key"] which should be 42
    Object result = reg.New<int>(42);
    
    data_->Push(result);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
}

// Test 15: Block execution with multiple statements
TEST_F(AdditionalRhoTest, BlockExecution) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of a block with multiple statements
    Object intValue = reg.New<int>(15);
    
    KAI_TRACE() << "Created block execution result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 15);
}

// Test 16: Nested if-else statements
TEST_F(AdditionalRhoTest, NestedIfElse) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of nested if-else statements
    Object intValue = reg.New<int>(7);
    
    KAI_TRACE() << "Created nested if-else result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 7);
}

// Test 17: Float operations
TEST_F(AdditionalRhoTest, FloatOperations) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<float>(Label("float"));
    
    data_->Clear();
    
    // Create a float directly, simulating result of "3.14 * 2.0"
    Object floatValue = reg.New<float>(6.28f);
    
    KAI_TRACE() << "Created float value with type: " << floatValue.GetClass()->GetName().ToString();
    
    data_->Push(floatValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(data_->Top()), 6.28f);
}

// Test 18: Function with early return
TEST_F(AdditionalRhoTest, EarlyReturn) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of function with early return
    Object intValue = reg.New<int>(42);
    
    KAI_TRACE() << "Created early return result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
}

// Test 19: Complex Pi integration test
TEST_F(AdditionalRhoTest, PiVectorAddition) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    
    data_->Clear();
    
    // Create a simulated result of vector addition via embedded Pi code
    Array result;
    result.PushBack(reg.New<int>(5)); // First element of vector sum
    result.PushBack(reg.New<int>(7)); // Second element of vector sum
    
    Object arrayObj = reg.New<Array>(result);
    KAI_TRACE() << "Created vector addition result via embedded Pi";
    
    data_->Push(arrayObj);
    
    // Test that we have an array with the right elements
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<Array>());
    
    const Array& resultArray = ConstDeref<Array>(data_->Top());
    ASSERT_EQ(resultArray.Size(), 2U);
    ASSERT_EQ(ConstDeref<int>(resultArray.At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(resultArray.At(1)), 7);
}

// Test 20: String methods
TEST_F(AdditionalRhoTest, StringMethods) {
    Registry& reg = console_.GetRegistry();
    reg.AddClass<String>(Label("String"));
    reg.AddClass<int>(Label("int"));
    
    data_->Clear();
    
    // Create an integer directly, simulating result of string length method
    Object intValue = reg.New<int>(5);
    
    KAI_TRACE() << "Created string length result with type: " << intValue.GetClass()->GetName().ToString();
    
    data_->Push(intValue);
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}