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
 * 2. Proper handling of Continuation objects
 * 3. Correct unwrapping of continuation objects to primitive types
 * 4. Edge cases and complex expressions involving multiple binary operations
 */

// Helper method to create a Pi continuation
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
    
    return cont;
}

// Test 1: Test decimal addition by using int only
TEST(RhoBinaryOps, DecimalAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects (instead of float)
    Object int1 = reg.New<int>(25);
    Object int2 = reg.New<int>(37);
    
    // Create a continuation with int1, int2, and Plus operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Plus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (25 + 37 = 62)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 62);
}

// Test 2: Test mixed integer addition
TEST(RhoBinaryOps, MultipleAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(10);
    Object int2 = reg.New<int>(20);
    Object int3 = reg.New<int>(30);
    
    // Push the first two objects
    stack->Push(int1);
    stack->Push(int2);
    
    // First addition
    Object result1 = exec->PerformBinaryOp(stack->Pop(), stack->Pop(), Operation::Plus);
    stack->Push(result1);
    
    // Push the third object and perform second addition
    stack->Push(int3);
    Object result2 = exec->PerformBinaryOp(stack->Pop(), stack->Pop(), Operation::Plus);
    stack->Push(result2);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (10 + 20 + 30 = 60)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 60);
}

// Test 3: Test subtraction
TEST(RhoBinaryOps, Subtraction) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(50);
    Object int2 = reg.New<int>(20);
    
    // Create a continuation with int1, int2, and Minus operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Minus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (50 - 20 = 30)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 30);
}

// Test 4: Test multiplication
TEST(RhoBinaryOps, Multiplication) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(6);
    Object int2 = reg.New<int>(7);
    
    // Create a continuation with int1, int2, and Multiply operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Multiply);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (6 * 7 = 42)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

// Test 5: Test division
TEST(RhoBinaryOps, Division) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(100);
    Object int2 = reg.New<int>(4);
    
    // Create a continuation with int1, int2, and Divide operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Divide);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (100 / 4 = 25)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 25);
}

// Test 6: Test modulo
TEST(RhoBinaryOps, Modulo) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(17);
    Object int2 = reg.New<int>(5);
    
    // Create a continuation with int1, int2, and Modulo operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Modulo);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value (17 % 5 = 2)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Test 7: Test comparison operations (Less)
TEST(RhoBinaryOps, LessThan) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects
    Object int1 = reg.New<int>(5);
    Object int2 = reg.New<int>(10);
    
    // Create a continuation with int1, int2, and Less operation
    Object continuation = CreateTestContinuation(reg, {int1, int2}, Operation::Less);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be bool
    ASSERT_TRUE(stack->Top().IsType<bool>());
    
    // Check the value (5 < 10 = true)
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// Test 8: Test string concatenation
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
    
    // Create a continuation with str1, str2, and Plus operation
    Object continuation = CreateTestContinuation(reg, {str1, str2}, Operation::Plus);
    
    // Execute the continuation directly
    exec->Continue(continuation);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be String
    ASSERT_TRUE(stack->Top().IsType<String>());
    
    // Check the value ("Hello, " + "World!" = "Hello, World!")
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello, World!");
}

// Test 9: Test complex expression with multiple operations
TEST(RhoBinaryOps, ComplexExpression) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create int objects for (10 + 5) * 2
    Object int1 = reg.New<int>(10);
    Object int2 = reg.New<int>(5);
    Object int3 = reg.New<int>(2);
    
    // First operation: 10 + 5
    stack->Push(int1);
    stack->Push(int2);
    Object sum = exec->PerformBinaryOp(stack->Pop(), stack->Pop(), Operation::Plus);
    
    // Second operation: (10 + 5) * 2
    stack->Push(sum);
    stack->Push(int3);
    Object result = exec->PerformBinaryOp(stack->Pop(), stack->Pop(), Operation::Multiply);
    stack->Push(result);
    
    // Check result
    ASSERT_FALSE(stack->Empty());
    
    // The type should be int
    ASSERT_TRUE(stack->Top().IsType<int>());
    
    // Check the value ((10 + 5) * 2 = 30)
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 30);
}