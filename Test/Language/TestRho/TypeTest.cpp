#include <gtest/gtest.h>
#include <iostream>
#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// Simple test to verify that Pi language operations preserve type correctly
TEST(TestRho, TestTypePreservation) {
    Console console;
    console.SetLanguage(Language::Pi);
    
    // Clear the stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Test addition
    console.Execute("2 3 +");
    
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after '2 3 +'";
    
    if (!stack->Empty()) {
        Object result = stack->Top();
        
        // Debug info
        std::cout << "Addition result type: " << result.GetClass()->GetName() << std::endl;
        std::cout << "Addition result value: " << result.ToString() << std::endl;
        
        // If it's a continuation, unwrap it
        if (result.IsType<Continuation>()) {
            Object unwrapped = exec->UnwrapValue(result);
            std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
            std::cout << "Unwrapped result value: " << unwrapped.ToString() << std::endl;
            
            // Replace with unwrapped value
            stack->Pop();
            stack->Push(unwrapped);
            result = unwrapped;
        }
        
        // Check type and value
        ASSERT_TRUE(result.IsType<int>()) << "Expected int but got " << result.GetClass()->GetName();
        ASSERT_EQ(ConstDeref<int>(result), 5) << "Expected 5 but got " << result.ToString();
    }
    
    // Clear the stack for the next test
    stack->Clear();
    
    // Test subtraction
    console.Execute("10 4 -");
    
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after '10 4 -'";
    
    if (!stack->Empty()) {
        Object result = stack->Top();
        
        // Debug info
        std::cout << "Subtraction result type: " << result.GetClass()->GetName() << std::endl;
        std::cout << "Subtraction result value: " << result.ToString() << std::endl;
        
        // If it's a continuation, unwrap it
        if (result.IsType<Continuation>()) {
            Object unwrapped = exec->UnwrapValue(result);
            std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
            std::cout << "Unwrapped result value: " << unwrapped.ToString() << std::endl;
            
            // Replace with unwrapped value
            stack->Pop();
            stack->Push(unwrapped);
            result = unwrapped;
        }
        
        // Check type and value
        ASSERT_TRUE(result.IsType<int>()) << "Expected int but got " << result.GetClass()->GetName();
        ASSERT_EQ(ConstDeref<int>(result), 6) << "Expected 6 but got " << result.ToString();
    }
    
    // Clear the stack for the next test
    stack->Clear();
    
    // Test multiplication
    console.Execute("6 7 *");
    
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after '6 7 *'";
    
    if (!stack->Empty()) {
        Object result = stack->Top();
        
        // Debug info
        std::cout << "Multiplication result type: " << result.GetClass()->GetName() << std::endl;
        std::cout << "Multiplication result value: " << result.ToString() << std::endl;
        
        // If it's a continuation, unwrap it
        if (result.IsType<Continuation>()) {
            Object unwrapped = exec->UnwrapValue(result);
            std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
            std::cout << "Unwrapped result value: " << unwrapped.ToString() << std::endl;
            
            // Replace with unwrapped value
            stack->Pop();
            stack->Push(unwrapped);
            result = unwrapped;
        }
        
        // Check type and value
        ASSERT_TRUE(result.IsType<int>()) << "Expected int but got " << result.GetClass()->GetName();
        ASSERT_EQ(ConstDeref<int>(result), 42) << "Expected 42 but got " << result.ToString();
    }
    
    // Clear the stack for the next test
    stack->Clear();
    
    // Test stack operations: push, dup, add
    console.Execute("5 dup +");
    
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after '5 dup +'";
    
    if (!stack->Empty()) {
        Object result = stack->Top();
        
        // Debug info
        std::cout << "Stack operations result type: " << result.GetClass()->GetName() << std::endl;
        std::cout << "Stack operations result value: " << result.ToString() << std::endl;
        
        // If it's a continuation, unwrap it
        if (result.IsType<Continuation>()) {
            Object unwrapped = exec->UnwrapValue(result);
            std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
            std::cout << "Unwrapped result value: " << unwrapped.ToString() << std::endl;
            
            // Replace with unwrapped value
            stack->Pop();
            stack->Push(unwrapped);
            result = unwrapped;
        }
        
        // Check type and value
        ASSERT_TRUE(result.IsType<int>()) << "Expected int but got " << result.GetClass()->GetName();
        ASSERT_EQ(ConstDeref<int>(result), 10) << "Expected 10 but got " << result.ToString();
    }
    
    // Clear the stack for the next test
    stack->Clear();
    
    // Test comparison operations 
    console.Execute("10 5 >");
    
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after '10 5 >'";
    
    if (!stack->Empty()) {
        Object result = stack->Top();
        
        // Debug info
        std::cout << "Comparison result type: " << result.GetClass()->GetName() << std::endl;
        std::cout << "Comparison result value: " << result.ToString() << std::endl;
        
        // If it's a continuation, unwrap it
        if (result.IsType<Continuation>()) {
            Object unwrapped = exec->UnwrapValue(result);
            std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
            std::cout << "Unwrapped result value: " << unwrapped.ToString() << std::endl;
            
            // Replace with unwrapped value
            stack->Pop();
            stack->Push(unwrapped);
            result = unwrapped;
        }
        
        // Check type and value
        ASSERT_TRUE(result.IsType<bool>()) << "Expected bool but got " << result.GetClass()->GetName();
        ASSERT_TRUE(ConstDeref<bool>(result)) << "Expected true but got " << result.ToString();
    }
}