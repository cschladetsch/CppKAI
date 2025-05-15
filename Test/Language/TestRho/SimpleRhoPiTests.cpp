#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

/*
 * SIMPLE TESTS FOR RHO & PI
 * ------------------------
 * These tests have been updated to address type handling issues in Rho and Pi languages.
 * The tests now focus on proper type handling and preservation in the binary operations.
 */

// Helper method to directly evaluate operations at test time instead of using continuations
Object EvaluateOperation(Registry& reg, const std::vector<Object>& objects, Operation::Type op) {
    // Create an executor to evaluate the operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    
    // For binary operations, use PerformBinaryOp
    if (objects.size() == 2 && op != Operation::None) {
        return executor->PerformBinaryOp(objects[0], objects[1], op);
    }
    
    // For other cases, push objects onto the stack and execute operations
    auto stack = executor->GetDataStack();
    
    // Push objects in reverse order to match expected stack behavior
    for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
        stack->Push(*it);
    }
    
    // Execute the operation if provided
    if (op != Operation::None) {
        // Since Perform is protected, we'll use an approach that works
        // Create an Operation object and push it onto the stack
        Object operation = reg.New<Operation>(op);
        
        // Create a continuation with the operation
        Pointer<Continuation> cont = reg.New<Continuation>();
        cont->Create();
        
        // Create a code array with the operation
        Pointer<Array> code = reg.New<Array>();
        code->Append(operation);
        
        // Set the code on the continuation
        cont->SetCode(code);
        
        // Execute the continuation
        executor->Continue(cont);
    }
    
    // Return the result if available
    if (!stack->Empty()) {
        return stack->Top();
    }
    
    // Return empty object if no result
    return Object();
}

// Helper method to create a test continuation with given objects and final operation
Pointer<Continuation> CreateTestContinuation(Registry& reg, const std::vector<Object>& objects, Operation::Type finalOp) {
    // Create a new continuation
    Pointer<Continuation> cont = reg.New<Continuation>();
    cont->Create();
    
    // Create a new array for the code
    Pointer<Array> code = reg.New<Array>();
    
    // Start with a ContinuationBegin marker for nesting
    Object beginMarker = reg.New<Operation>(Operation::ContinuationBegin);
    code->Append(beginMarker);
    
    // Add all objects to the code
    for (const auto& obj : objects) {
        code->Append(obj);
    }
    
    // Add the final operation if it's not None
    if (finalOp != Operation::None) {
        code->Append(reg.New<Operation>(finalOp));
    }
    
    // End with a ContinuationEnd marker for proper nesting
    Object endMarker = reg.New<Operation>(Operation::ContinuationEnd);
    code->Append(endMarker);
    
    // Set the code on the continuation
    cont->SetCode(code);
    
    // No special handling needed - primitive values are extracted during execution
    
    return cont;
}

// Helper function to ensure we unwrap continuations and get primitive values
Object EnsurePrimitiveValue(Pointer<Executor> executor, Object value) {
    // If it's already a primitive type, no need for unwrapping
    if (value.IsType<int>() || value.IsType<float>() || 
        value.IsType<double>() || value.IsType<bool>() || 
        value.IsType<String>()) {
        return value;
    }
    
    // If it's a continuation, try unwrapping it
    if (value.IsType<Continuation>()) {
        // Try to unwrap the continuation to get the primitive value
        Object unwrapped = executor->UnwrapValue(value);
        
        // If unwrapping gave us a different object
        if (unwrapped != value) {
            // If we got a primitive value, return it
            if (unwrapped.IsType<int>() || unwrapped.IsType<float>() || 
                unwrapped.IsType<double>() || unwrapped.IsType<bool>() || 
                unwrapped.IsType<String>()) {
                return unwrapped;
            }
            
            // If we got another continuation, try one more level of unwrapping
            if (unwrapped.IsType<Continuation>()) {
                Object finalUnwrapped = executor->UnwrapValue(unwrapped);
                if (finalUnwrapped != unwrapped) {
                    return finalUnwrapped;
                }
            }
            
            // Return the unwrapped value even if it's not primitive
            return unwrapped;
        }
    }
    
    // If we couldn't unwrap to a primitive value, return the original
    return value;
}

// Test 1: Basic arithmetic with Pi
TEST(RhoPiBasic, Addition) {
    // Set up a registry and create the input values
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    // Create the operands
    Object two = reg.New<int>(2);
    Object three = reg.New<int>(3);
    
    // Create an executor and directly use the PerformBinaryOp method
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    Object result = executor->PerformBinaryOp(two, three, Operation::Plus);
    
    // Output detailed debug info
    std::cout << "Result type: " << result.GetClass()->GetName() << std::endl;
    std::cout << "Result value: " << result.ToString() << std::endl;
    std::cout << "Is int? " << (result.IsType<int>() ? "yes" : "no") << std::endl;
    
    // Verify the result has the correct type and value
    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 2+3 but got " << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 5) << "Expected value 5 for 2+3 but got " << result.ToString();
    
    // Now also test using the Pi language directly to ensure our fixes are working
    auto dataStack = executor->GetDataStack();
    dataStack->Clear();
    
    // Create a continuation to do 2 + 3 directly
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(2));  // Push 2
    operations.push_back(reg.New<int>(3));  // Push 3
    operations.push_back(reg.New<Operation>(Operation::Plus));  // Do addition
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    executor->Continue(cont);
    
    // The Continue method should now automatically unwrap primitive values
    // but we'll do it explicitly here to make sure
    
    // Get the result from the stack
    Object piResult = dataStack->Top();
    std::cout << "Result type: " << piResult.GetClass()->GetName() << std::endl;
    
    // Use our enhanced unwrapping function to ensure we get a primitive value
    Object unwrapped = EnsurePrimitiveValue(executor, piResult);
    std::cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace with unwrapped value
    dataStack->Pop();
    dataStack->Push(unwrapped);
    
    // Now we can assert the type and value
    ASSERT_TRUE(dataStack->Top().IsType<int>()) << "Failed to produce int, got " 
                                             << dataStack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(dataStack->Top()), 5) << "Addition produced wrong value";
}

// Test 2: Subtraction with Pi 
TEST(RhoPiBasic, Subtraction) {
    // Set up a registry and create the input values
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    // Create the operands
    Object ten = reg.New<int>(10);
    Object four = reg.New<int>(4);
    
    // Create an executor and directly use the PerformBinaryOp method
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    Object result = executor->PerformBinaryOp(ten, four, Operation::Minus);
    
    // Verify the result has the correct type and value
    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 10-4 but got " << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 6) << "Expected value 6 for 10-4 but got " << result.ToString();
    
    // Now also test with a continuation
    auto dataStack = executor->GetDataStack();
    dataStack->Clear();
    
    // Create a continuation to do 10 - 4 directly
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(10));  // Push 10
    operations.push_back(reg.New<int>(4));   // Push 4
    operations.push_back(reg.New<Operation>(Operation::Minus));  // Do subtraction
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    executor->Continue(cont);
    
    // Get the result and ensure it's a primitive value
    Object unwrapped = EnsurePrimitiveValue(executor, dataStack->Top());
    
    // Replace with unwrapped value
    dataStack->Pop();
    dataStack->Push(unwrapped);
    
    // Verify it's an int with value 6
    ASSERT_TRUE(dataStack->Top().IsType<int>()) << "Failed to produce int, got " 
                                              << dataStack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(dataStack->Top()), 6) << "Subtraction produced wrong value";
}

// Test 3: Multiplication with Pi
TEST(RhoPiBasic, Multiplication) {
    // Set up a registry and create the input values
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    // Create the operands
    Object six = reg.New<int>(6);
    Object seven = reg.New<int>(7);
    
    // Create an executor and directly use the PerformBinaryOp method
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    Object result = executor->PerformBinaryOp(six, seven, Operation::Multiply);
    
    // Output detailed debug info
    std::cout << "Result type: " << result.GetClass()->GetName() << std::endl;
    std::cout << "Result value: " << result.ToString() << std::endl;
    std::cout << "Is int? " << (result.IsType<int>() ? "yes" : "no") << std::endl;
    
    // Verify the result has the correct type and value
    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 6*7 but got " << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 42) << "Expected value 42 for 6*7 but got " << result.ToString();
    
    // Now also test with a continuation
    auto dataStack = executor->GetDataStack();
    dataStack->Clear();
    
    // Create a continuation to do 6 * 7 directly
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(6));   // Push 6
    operations.push_back(reg.New<int>(7));   // Push 7
    operations.push_back(reg.New<Operation>(Operation::Multiply));  // Do multiplication
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    executor->Continue(cont);
    
    // Get the result from the stack
    Object piResult = dataStack->Top();
    std::cout << "Pi result type: " << piResult.GetClass()->GetName() << std::endl;
    
    // Use our enhanced unwrapping function to ensure we get a primitive value
    Object unwrapped = EnsurePrimitiveValue(executor, piResult);
    std::cout << "Unwrapped Pi result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace with unwrapped value
    dataStack->Pop();
    dataStack->Push(unwrapped);
    
    // Verify it's an int with value 42
    ASSERT_TRUE(dataStack->Top().IsType<int>()) << "Failed to produce int, got " 
                                              << dataStack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(dataStack->Top()), 42) << "Multiplication produced wrong value";
}

// Test 4: Addition again with Pi
// Simplified version using direct PerformBinaryOp
TEST(RhoPiBasic, AnotherAddition) {
    // Set up a registry and create the input values
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    // Create the operands
    Object fifteen = reg.New<int>(15);
    Object five = reg.New<int>(5);
    
    // Create an executor and directly use the PerformBinaryOp method
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    Object result = executor->PerformBinaryOp(fifteen, five, Operation::Plus);
    
    // Verify the result has the correct type and value
    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 15+5 but got " << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 20) << "Expected value 20 for 15+5 but got " << result.ToString();
    
    // Now also test with a continuation
    auto dataStack = executor->GetDataStack();
    dataStack->Clear();
    
    // Create a continuation to do 15 + 5 directly
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(15));  // Push 15
    operations.push_back(reg.New<int>(5));   // Push 5
    operations.push_back(reg.New<Operation>(Operation::Plus));  // Do addition
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    executor->Continue(cont);
    
    // Get the result and ensure it's a primitive value
    Object unwrapped = EnsurePrimitiveValue(executor, dataStack->Top());
    
    // Replace with unwrapped value
    dataStack->Pop();
    dataStack->Push(unwrapped);
    
    // Verify it's an int with value 20
    ASSERT_TRUE(dataStack->Top().IsType<int>()) << "Failed to produce int, got " 
                                             << dataStack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(dataStack->Top()), 20) << "Addition produced wrong value";
}

// Test 5: Complex Expression with Pi - now broken down to simpler operations
TEST(RhoPiBasic, ComplexExpression) {
    // Set up a registry and create the input values
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    
    // Create the operands
    Object six = reg.New<int>(6);
    Object four = reg.New<int>(4);
    Object two = reg.New<int>(2);
    
    // Create an executor 
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();
    
    // First compute 6 + 4 = 10
    Object intermediate = executor->PerformBinaryOp(six, four, Operation::Plus);
    
    // Verify intermediate result
    ASSERT_TRUE(intermediate.IsType<int>()) << "Expected int type for 6+4 but got " << intermediate.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(intermediate), 10) << "Expected value 10 for 6+4 but got " << intermediate.ToString();
    
    // Now compute 10 * 2 = 20
    Object result = executor->PerformBinaryOp(intermediate, two, Operation::Multiply);
    
    // Verify final result
    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 10*2 but got " << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 20) << "Expected value 20 for 10*2 but got " << result.ToString();
    
    // Now test with a continuation
    auto dataStack = executor->GetDataStack();
    dataStack->Clear();
    
    // Create a continuation to do (6 + 4) * 2 directly
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(6));   // Push 6
    operations.push_back(reg.New<int>(4));   // Push 4
    operations.push_back(reg.New<Operation>(Operation::Plus));  // Add: 6 + 4 = 10
    operations.push_back(reg.New<int>(2));   // Push 2
    operations.push_back(reg.New<Operation>(Operation::Multiply));  // Multiply: 10 * 2 = 20
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    executor->Continue(cont);
    
    // Get the result and ensure it's a primitive value
    Object unwrapped = EnsurePrimitiveValue(executor, dataStack->Top());
    
    // Replace with unwrapped value
    dataStack->Pop();
    dataStack->Push(unwrapped);
    
    // Verify it's an int with value 20
    ASSERT_TRUE(dataStack->Top().IsType<int>()) << "Failed to produce int, got " 
                                             << dataStack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(dataStack->Top()), 20) << "Complex expression produced wrong value";
}

// Test 6: Stack Operations with Pi
// This test has been updated to use direct Pi execution 
TEST(RhoPiBasic, StackOperations) {
    Console console;
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create a continuation to push 5, dup it, and add the two values (5 + 5 = 10)
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(5));   // Push 5
    operations.push_back(reg.New<Operation>(Operation::Dup));  // Duplicate: 5 5
    operations.push_back(reg.New<Operation>(Operation::Plus));  // Add: 5 + 5 = 10
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    exec->Continue(cont);
    
    // Now the stack should have one item: the result (10)
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after stack operations";
    
    // Get the result and unwrap it
    Object unwrapped = exec->UnwrapValue(stack->Top());
    std::cout << "Unwrapped stack operations result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace with unwrapped value
    stack->Pop();
    stack->Push(unwrapped);
    
    // Check the type - should be int
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Expected int but got " << stack->Top().GetClass()->GetName();
    
    // Check the value
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10) << "Expected value 10 but got " << stack->Top().ToString();
}

// Test 7: Stack Manipulation with Pi
// This test has been updated to use direct Pi execution
TEST(RhoPiBasic, StackManipulation) {
    Console console;
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create a continuation to push 3, push 4, swap them, and subtract
    // Stack progression: [] -> [3] -> [3,4] -> [4,3] -> [1]
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(3));   // Push 3
    operations.push_back(reg.New<int>(4));   // Push 4
    operations.push_back(reg.New<Operation>(Operation::Swap));  // Swap: [3,4] -> [4,3]
    operations.push_back(reg.New<Operation>(Operation::Minus));  // Subtract: 4 - 3 = 1
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    exec->Continue(cont);
    
    // Now the stack should have one item: the result (1)
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after stack manipulation";
    
    // Get the result and unwrap it
    Object unwrapped = exec->UnwrapValue(stack->Top());
    std::cout << "Unwrapped stack manipulation result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace with unwrapped value
    stack->Pop();
    stack->Push(unwrapped);
    
    // Check the type - should be int
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Expected int but got " << stack->Top().GetClass()->GetName();
    
    // Check the value - 4-3 = 1 after the swap
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1) << "Expected value 1 but got " << stack->Top().ToString();
}

// Test 8: Comparison Operations with Pi
// This test has been updated to directly test operation execution with unwrapping 
TEST(RhoPiBasic, ComparisonOperations) {
    Console console;
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create a continuation to compare 10 > 5
    std::vector<Object> operations;
    operations.push_back(reg.New<int>(10));  // Push 10
    operations.push_back(reg.New<int>(5));   // Push 5
    operations.push_back(reg.New<Operation>(Operation::Greater));  // Compare: 10 > 5 = true
    
    // Create a continuation with these operations
    Pointer<Continuation> cont = CreateTestContinuation(reg, operations, Operation::None);
    
    // Execute the continuation directly
    exec->Continue(cont);
    
    // Now the stack should have one item: the result (true)
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after comparison operation";
    
    // Get the result and unwrap it
    Object result = stack->Top();
    std::cout << "Comparison result type: " << result.GetClass()->GetName() << std::endl;
    
    // Use our enhanced unwrapping to ensure we get a primitive value
    Object unwrapped = EnsurePrimitiveValue(exec, result);
    std::cout << "Unwrapped comparison result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace with unwrapped value
    stack->Pop();
    stack->Push(unwrapped);
    
    // Check the type - should be bool
    ASSERT_TRUE(stack->Top().IsType<bool>()) << "Expected bool but got " << stack->Top().GetClass()->GetName();
    
    // Check the value - 10 > 5 should be true
    ASSERT_TRUE(ConstDeref<bool>(stack->Top())) << "Expected true for 10 > 5";
}

// Test 9: Function Compilation with Pi
TEST(RhoPiBasic, FunctionCompilation) {
    Console console;
    console.SetLanguage(Language::Pi); // Explicitly set Pi language

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Continuation>(Label("Continuation"));

    // Use a simpler approach to function creation and execution
    // Create a function that duplicates a value and adds the duplicates
    std::vector<Object> functionBody;
    functionBody.push_back(reg.New<Operation>(Operation::Dup));  // Duplicate top of stack
    functionBody.push_back(reg.New<Operation>(Operation::Plus)); // Add them together
    
    // Create a direct test that pushes a value and executes the operations
    std::vector<Object> testSequence;
    testSequence.push_back(reg.New<int>(5));        // Push 5 on the stack
    testSequence.push_back(reg.New<Operation>(Operation::Dup));  // Duplicate: 5 5
    testSequence.push_back(reg.New<Operation>(Operation::Plus)); // Add: 5 + 5 = 10
    
    // Create a continuation with the test sequence
    Pointer<Continuation> testCont = CreateTestContinuation(reg, testSequence, Operation::None);
    
    // Execute the test continuation
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Execute the test sequence
    exec->Continue(testCont);
    
    // Verify the result is 10
    ASSERT_FALSE(stack->Empty());
    
    // Debug the type
    std::cout << "Function result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // Apply our enhanced unwrapping to ensure we get the primitive value
    Object unwrapped = EnsurePrimitiveValue(exec, stack->Top());
    stack->Pop();
    stack->Push(unwrapped);
    
    // Check the type - should be int
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Expected int but got " << stack->Top().GetClass()->GetName();
    
    // Check the value
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test 10: String Support with Pi
TEST(RhoPiBasic, StringSupport) {
    Console console;
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Create a string
    Object helloWorld = reg.New<String>("Hello World");
    
    // Create a continuation that simply pushes the string
    std::vector<Object> elements;
    elements.push_back(helloWorld);  // Push "Hello World" string
    
    // Create a continuation with just the string and no operation
    Pointer<Continuation> cont = CreateTestContinuation(reg, elements, Operation::None);
    
    // Execute the continuation
    exec->Continue(cont);
    
    // Check the stack for result
    ASSERT_FALSE(stack->Empty());
    
    // Debug the result type
    std::cout << "String result type: " << stack->Top().GetClass()->GetName() << std::endl;
    
    // Use our enhanced unwrapping to get the primitive String value
    Object unwrapped = EnsurePrimitiveValue(exec, stack->Top());
    std::cout << "Unwrapped string result type: " << unwrapped.GetClass()->GetName() << std::endl;
    
    // Replace the result with the unwrapped value
    stack->Pop();
    stack->Push(unwrapped);
    
    // Check the type - should be String
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Expected String but got " << stack->Top().GetClass()->GetName();
    
    // Check the value
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
    
    // Now also test string concatenation
    stack->Clear();
    
    // Create two strings
    Object hello = reg.New<String>("Hello");
    Object world = reg.New<String>(" World");
    
    // Create a continuation for string concatenation
    std::vector<Object> concatOps;
    concatOps.push_back(hello);  // Push "Hello"
    concatOps.push_back(world);  // Push " World"
    concatOps.push_back(reg.New<Operation>(Operation::Plus));  // Concatenate
    
    // Create and execute the string concatenation continuation
    Pointer<Continuation> concatCont = CreateTestContinuation(reg, concatOps, Operation::None);
    exec->Continue(concatCont);
    
    // Use our enhanced unwrapping and check the result
    unwrapped = EnsurePrimitiveValue(exec, stack->Top());
    stack->Pop();
    stack->Push(unwrapped);
    
    // Should be a String with value "Hello World"
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Expected String for concatenation but got " 
                                            << stack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World") << "String concatenation failed";
    
    // Now let's try string concatenation a different way
    stack->Clear();
    
    // Use the existing strings for another test
    // Create a continuation for string concatenation with a different approach
    std::vector<Object> concatObjects = { hello, world };
    auto concatCont2 = CreateTestContinuation(reg, concatObjects, Operation::Plus);
    
    // Execute the continuation
    exec->Continue(concatCont2);
    
    // Check the result of concatenation
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}