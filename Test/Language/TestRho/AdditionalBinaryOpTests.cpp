#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

/*
 * ADDITIONAL TESTS FOR BINARY OPERATIONS
 * -------------------------------------
 * These tests specifically focus on binary operations to ensure
 * they return the correct primitive types after our fixes.
 * 
 * The tests are designed to extensively verify:
 * 1. Type preservation in all binary operations
 * 2. Proper handling of Continuation objects with specialHandling flag
 * 3. Correct unwrapping of continuation objects to primitive types
 * 4. Edge cases and complex expressions involving multiple binary operations
 */

// Helper method to create a Pi continuation with special handling enabled (same as in SimpleRhoPiTests)
Pointer<Continuation> CreateTestContinuation(Registry& reg, const std::vector<Object>& objects, Operation::Type op) {
    // Create a continuation
    Pointer<Continuation> cont = reg.New<Continuation>();
    cont->Create();
    
    // Create a code array
    Pointer<Array> code = reg.New<Array>();
    
    // Add a ContinuationBegin marker for proper nesting
    Object beginMarker = reg.New<Operation>(Operation::ContinuationBegin);
    code->Append(beginMarker);
    
    // Add all objects to the code array
    for (const auto& obj : objects) {
        code->Append(obj);
    }
    
    // Add the operation
    if (op != Operation::None) {
        code->Append(reg.New<Operation>(op));
    }
    
    // Add a ContinuationEnd marker for proper nesting
    Object endMarker = reg.New<Operation>(Operation::ContinuationEnd);
    code->Append(endMarker);
    
    // Set the code array on the continuation
    cont->SetCode(code);
    
    // Mark it for special handling
    cont->SetSpecialHandling(true);
    
    return cont;
}

// Test 1: Test that floating point addition works and preserves type
TEST(RhoBinaryOps, FloatAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<float>(Label("float"));
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create float objects
    Object float1 = reg.New<float>(2.5f);
    Object float2 = reg.New<float>(3.7f);
    
    // Create a continuation with float1, float2, and Plus operation
    Object continuation = CreateTestContinuation(reg, {float1, float2}, Operation::Plus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be float
    ASSERT_TRUE(stack->Top().IsType<float>());
    
    // Check the value (approximately 6.2)
    ASSERT_NEAR(ConstDeref<float>(stack->Top()), 6.2f, 0.0001f);
}

// Test 2: Test mixed integer and float addition
TEST(RhoBinaryOps, MixedTypeAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<float>(Label("float"));
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create objects - int and float
    Object intVal = reg.New<int>(5);
    Object floatVal = reg.New<float>(2.5f);
    
    // Create a continuation that adds an int and a float
    Object continuation = CreateTestContinuation(reg, {intVal, floatVal}, Operation::Plus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // Result should be a float (type promotion)
    ASSERT_TRUE(stack->Top().IsType<float>());
    
    // Check the value (7.5)
    ASSERT_NEAR(ConstDeref<float>(stack->Top()), 7.5f, 0.0001f);
}

// Test 3: Test string concatenation with binary plus
TEST(RhoBinaryOps, StringConcatenation) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create string objects
    Object str1 = reg.New<String>("Hello, ");
    Object str2 = reg.New<String>("World!");
    
    // Create a continuation with string1, string2, and Plus operation
    Object continuation = CreateTestContinuation(reg, {str1, str2}, Operation::Plus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be String
    ASSERT_TRUE(stack->Top().IsType<String>());
    
    // Check the value
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello, World!");
}

// Test 4: Test boolean logical operations (AND)
TEST(RhoBinaryOps, LogicalAND) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create boolean objects for true AND false
    Object boolTrue = reg.New<bool>(true);
    Object boolFalse = reg.New<bool>(false);
    
    // Create a continuation for true AND false
    Object continuation = CreateTestContinuation(reg, {boolTrue, boolFalse}, Operation::LogicalAnd);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be bool
    ASSERT_TRUE(stack->Top().IsType<bool>());
    
    // true AND false should be false
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
    
    // Test true AND true as well
    stack->Clear();
    Object continuation2 = CreateTestContinuation(reg, {boolTrue, boolTrue}, Operation::LogicalAnd);
    exec->Continue(continuation2);
    
    // Check result - should be true
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// Test 5: Test boolean logical operations (OR)
TEST(RhoBinaryOps, LogicalOR) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create boolean objects for true OR false
    Object boolTrue = reg.New<bool>(true);
    Object boolFalse = reg.New<bool>(false);
    
    // Create a continuation for true OR false
    Object continuation = CreateTestContinuation(reg, {boolTrue, boolFalse}, Operation::LogicalOr);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be bool
    ASSERT_TRUE(stack->Top().IsType<bool>());
    
    // true OR false should be true
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
    
    // Test false OR false as well
    stack->Clear();
    Object continuation2 = CreateTestContinuation(reg, {boolFalse, boolFalse}, Operation::LogicalOr);
    exec->Continue(continuation2);
    
    // Check result - should be false
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Test 6: Test integer division with exact result
TEST(RhoBinaryOps, IntegerDivision) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create integer objects
    Object int1 = reg.New<int>(10);
    Object int2 = reg.New<int>(2);
    
    // Create a continuation with int1, int2, and Divide operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Divide);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Test 7: Test integer modulo operation
TEST(RhoBinaryOps, ModuloOperation) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create integer objects
    Object int1 = reg.New<int>(7);
    Object int2 = reg.New<int>(3);
    
    // Create a continuation with int1, int2, and Modulo operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Modulo);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (7 % 3 = 1)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1);
}

// Test 8: Test complex expression with multiple operations
TEST(RhoBinaryOps, ComplexExpression) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // To compute (2 + 3) * 4 - 5:
    // First calculate (2 + 3)
    Object two = reg.New<int>(2);
    Object three = reg.New<int>(3);
    Object addition = CreateTestContinuation(reg, {two, three}, Operation::Plus);
    
    // Execute the first step
    exec->Continue(addition);
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Get the result of 2+3 for the next step
    Object five = stack->Top();
    stack->Clear();
    
    // Now calculate (2+3) * 4 => 5 * 4
    Object four = reg.New<int>(4);
    Object multiplication = CreateTestContinuation(reg, {five, four}, Operation::Multiply);
    
    // Execute the second step
    exec->Continue(multiplication);
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Get the result of 5*4 for the final step
    Object twenty = stack->Top();
    stack->Clear();
    
    // Finally calculate (2+3) * 4 - 5 => 20 - 5
    Object five2 = reg.New<int>(5);
    Object subtraction = CreateTestContinuation(reg, {twenty, five2}, Operation::Minus);
    
    // Execute the final step
    exec->Continue(subtraction);
    
    // Check the final result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value ((2+3) * 4 - 5 = 15)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 15);
}

// Test 9: Test comparison operations
TEST(RhoBinaryOps, ComparisonOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    // Test equality comparison
    stack->Clear();
    Object five1 = reg.New<int>(5);
    Object five2 = reg.New<int>(5);
    Object eqComparison = CreateTestContinuation(reg, {five1, five2}, Operation::Equiv);
    exec->Continue(eqComparison);
    
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
    
    // Test inequality comparison
    stack->Clear();
    Object five = reg.New<int>(5);
    Object six = reg.New<int>(6);
    Object neqComparison = CreateTestContinuation(reg, {five, six}, Operation::NotEquiv);
    exec->Continue(neqComparison);
    
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
    
    // Test greater than
    stack->Clear();
    Object seven = reg.New<int>(7);
    Object gtComparison = CreateTestContinuation(reg, {seven, five}, Operation::Greater);
    exec->Continue(gtComparison);
    
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top())); // 7 > 5 is true
    
    // Test less than
    stack->Clear();
    Object ltComparison = CreateTestContinuation(reg, {five, seven}, Operation::Less);
    exec->Continue(ltComparison);
    
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top())); // 5 < 7 is true
}

// Test 10: Test nested binary operations with special handling
TEST(RhoBinaryOps, NestedOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    // Create objects
    Object two = reg.New<int>(2);
    Object three = reg.New<int>(3);
    Object four = reg.New<int>(4);
    
    // First create a continuation for 3+4
    Pointer<Continuation> innerCont = CreateTestContinuation(reg, {three, four}, Operation::Plus);
    
    // Now create an outer continuation that multiplies 2 with the result of (3+4)
    // We need to push the inner continuation and the integer 2 to the stack
    std::vector<Object> outerObjects = {innerCont, two};
    Pointer<Continuation> outerCont = CreateTestContinuation(reg, outerObjects, Operation::Multiply);
    
    // Execute the outer continuation
    stack->Clear();
    exec->Continue(outerCont);
    
    // Check the result
    ASSERT_FALSE(stack->Empty());
    std::cout << "Result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (2 * (3 + 4) = 2 * 7 = 14)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 14);
}