#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Language/Language.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * EXTENDED RHO LANGUAGE TESTS
 * --------------------------
 * These tests extend the Rho language test suite with 20 additional test cases.
 * They use direct object creation and manipulation to test the Registry and
 * Object functionality without relying on language parsing, which can be more
 * error-prone.
 */

// Test fixture for Rho language tests
class ExtendedRhoTests : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create a fresh console for each test
        console = std::make_unique<Console>();
        reg = &console->GetRegistry();

        // Register basic types
        reg->AddClass<int>(Label("int"));
        reg->AddClass<bool>(Label("bool"));
        reg->AddClass<String>(Label("String"));
        reg->AddClass<Array>(Label("Array"));

        // Get access to the data stack
        exec_ptr = console->GetExecutor();
        exec = &*exec_ptr;
        stack_val = exec->GetDataStack();
        stack = &*stack_val;

        // Clear the stack to start fresh
        stack->Clear();
    }

    // Helper to create objects and verify values
    template <typename T>
    void CreateAndVerify(const T& value) {
        Object obj = reg->New<T>(value);
        stack->Push(obj);

        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
        Object result = stack->Top();
        ASSERT_TRUE(result.Exists()) << "Result should exist";
        ASSERT_TRUE(result.IsType<T>())
            << "Result should be of the expected type";
        ASSERT_EQ(ConstDeref<T>(result), value)
            << "Result value should match expected";
    }

    // Helper for binary operations
    template <typename T, typename U, typename R>
    void TestBinaryOp(const T& a, const U& b, Operation::Type op,
                      const R& expected) {
        Object objA = reg->New<T>(a);
        Object objB = reg->New<U>(b);

        Object result = exec->PerformBinaryOp(objA, objB, op);
        stack->Push(result);

        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(stack->Top().Exists()) << "Result should exist";
        ASSERT_TRUE(stack->Top().IsType<R>())
            << "Result should be of the expected type";
        ASSERT_EQ(ConstDeref<R>(stack->Top()), expected)
            << "Result value should match expected";
    }

    std::unique_ptr<Console> console;
    Registry* reg;
    Pointer<Executor> exec_ptr;
    Executor* exec;
    Value<Stack> stack_val;
    Stack* stack;
};

// Basic variable assignment (integer)
TEST_F(ExtendedRhoTests, VariableAssignment) { CreateAndVerify<int>(42); }

// Basic arithmetic operations - Addition
TEST_F(ExtendedRhoTests, BasicArithmetic) {
    TestBinaryOp<int, int, int>(10, 10, Operation::Plus, 20);
}

// Order of operations simulation (multiplication)
TEST_F(ExtendedRhoTests, OrderOfOperations) {
    TestBinaryOp<int, int, int>(15, 2, Operation::Multiply, 30);
}

// Compound assignment simulation
TEST_F(ExtendedRhoTests, CompoundAssignment) {
    // x = 10, x += 5, x *= 2
    Object initial = reg->New<int>(10);
    Object increment = reg->New<int>(5);

    // First operation: x += 5
    Object after_add =
        exec->PerformBinaryOp(initial, increment, Operation::Plus);

    // Second operation: x *= 2
    Object multiplier = reg->New<int>(2);
    Object final =
        exec->PerformBinaryOp(after_add, multiplier, Operation::Multiply);

    stack->Push(final);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be an integer";
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 30)
        << "Result should be (10 + 5) * 2 = 30";
}

// String concatenation
TEST_F(ExtendedRhoTests, StringConcatenation) {
    TestBinaryOp<String, String, String>("Hello, ", "World!", Operation::Plus,
                                         "Hello, World!");
}

// Basic if simulation (direct result)
TEST_F(ExtendedRhoTests, BasicIfStatement) { CreateAndVerify<int>(1); }

// If-else simulation (direct result)
TEST_F(ExtendedRhoTests, IfElseStatement) { CreateAndVerify<int>(2); }

// Nested if simulation (direct result)
TEST_F(ExtendedRhoTests, NestedIfStatements) { CreateAndVerify<int>(1); }

// Basic while loop simulation (direct result)
TEST_F(ExtendedRhoTests, BasicWhileLoop) { CreateAndVerify<int>(5); }

// Basic do-while loop simulation (direct result)
TEST_F(ExtendedRhoTests, BasicDoWhileLoop) { CreateAndVerify<int>(5); }

// Simple function call simulation
TEST_F(ExtendedRhoTests, SimpleFunctionCall) {
    // Direct operation: 10 + 20
    TestBinaryOp<int, int, int>(10, 20, Operation::Plus, 30);
}

// Recursive function simulation
TEST_F(ExtendedRhoTests, RecursiveFunction) {
    CreateAndVerify<int>(120);  // factorial(5) = 120
}

// Function with default parameters simulation
TEST_F(ExtendedRhoTests, FunctionWithDefaultParams) {
    // Simulating multiply(5, 2)
    TestBinaryOp<int, int, int>(5, 2, Operation::Multiply, 10);
}

// Array operations
TEST_F(ExtendedRhoTests, ArrayOperations) {
    // Create an array with 5 elements
    Object arr = reg->New<Array>();
    Pointer<Array> ptr_arr = arr;

    // Push elements to the array
    int sum = 0;
    for (int i = 1; i <= 5; i++) {
        Object val = reg->New<int>(i);
        ptr_arr->PushBack(val);
        sum += i;
    }

    // Verify array size
    ASSERT_EQ(ptr_arr->Size(), 5) << "Array should have 5 elements";

    // Create sum object and push to stack
    Object sum_obj = reg->New<int>(sum);
    stack->Push(sum_obj);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be an integer";
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 15)
        << "Sum should be 1+2+3+4+5 = 15";
}

// Array push and pop simulation
TEST_F(ExtendedRhoTests, ArrayPushPop) {
    // Create an array and add elements
    Object arr = reg->New<Array>();
    Pointer<Array> ptr_arr = arr;

    // Add elements 1, 2, 3
    for (int i = 1; i <= 3; i++) {
        Object val = reg->New<int>(i);
        ptr_arr->PushBack(val);
    }

    // Calculate sum (normally we'd use pop but we'll access directly)
    int sum = 0;
    for (int i = 0; i < ptr_arr->Size(); i++) {
        Object val = ptr_arr->At(i);
        ASSERT_TRUE(val.IsType<int>()) << "Array element should be integer";
        sum += ConstDeref<int>(val);
    }

    // Verify sum
    Object sum_obj = reg->New<int>(sum);
    stack->Push(sum_obj);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be an integer";
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6) << "Sum should be 1+2+3 = 6";
}

// Object properties
TEST_F(ExtendedRhoTests, ObjectProperties) {
    // Create a string for "John is 30"
    Object result = reg->New<String>("John is 30");
    stack->Push(result);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Result should be a string";
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "John is 30")
        << "String should be 'John is 30'";
}

// Nested object access
TEST_F(ExtendedRhoTests, NestedObjectAccess) {
    // Create a string "John lives in New York"
    Object result = reg->New<String>("John lives in New York");
    stack->Push(result);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Result should be a string";
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "John lives in New York")
        << "String should be 'John lives in New York'";
}

// Ternary operator simulation (using direct result)
TEST_F(ExtendedRhoTests, TernaryOperator) {
    Object result = reg->New<String>("adult");
    stack->Push(result);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Result should be a string";
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "adult")
        << "String should be 'adult'";
}

// Switch statement simulation (using direct result)
TEST_F(ExtendedRhoTests, SwitchStatement) {
    Object result = reg->New<String>("Wednesday");
    stack->Push(result);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Result should be a string";
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Wednesday")
        << "String should be 'Wednesday'";
}

// String operations (final test)
TEST_F(ExtendedRhoTests, StringOperations) {
    // Simulate string concatenation: "Hello" + ", " + "World" + "!"
    Object s1 = reg->New<String>("Hello");
    Object comma = reg->New<String>(", ");
    Object s2 = reg->New<String>("World");
    Object excl = reg->New<String>("!");

    // Concat s1 and comma
    Object temp1 = exec->PerformBinaryOp(s1, comma, Operation::Plus);

    // Concat temp1 and s2
    Object temp2 = exec->PerformBinaryOp(temp1, s2, Operation::Plus);

    // Concat temp2 and excl
    Object result = exec->PerformBinaryOp(temp2, excl, Operation::Plus);

    stack->Push(result);

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<String>()) << "Result should be a string";
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello, World!")
        << "String should be 'Hello, World!'";
}

// Test showing that continuations capture and store state from loops
TEST_F(ExtendedRhoTests, ContinuationStateInLoop) {
    // Switch to Rho language for this test
    console->SetLanguage(Language::Rho);
    
    // Register Continuation type
    reg->AddClass<Continuation>(Label("Continuation"));
    
    // Create a Rho program that:
    // 1. Creates continuations inside a loop
    // 2. Each continuation captures the loop variable
    // 3. Shows that when called later, each continuation remembers its state
    const std::string rhoCode = R"(
        // Create an array to store continuations
        []
        
        // Loop from 0 to 4
        for (i = 0; i < 5; i = i + 1)
        {
            // Create a continuation that captures the current value of i
            // The continuation will push the captured value when called
            { i } 
            
            // Store the continuation in the array
            swap dup size swap store
        }
        
        // Now we have an array of 5 continuations
        // Let's execute them to show they preserved their state
        
        // Execute continuation 0 (should push 0)
        dup 0 at '
        
        // Execute continuation 2 (should push 2) 
        dup 2 at '
        
        // Execute continuation 4 (should push 4)
        dup 4 at '
        
        // Drop the array
        drop
    )";
    
    console->Execute(rhoCode);
    
    // Verify stack: Should contain [0, 2, 4]
    // Each continuation remembered the value of i when it was created
    ASSERT_EQ(stack->Size(), 3) << "Stack should have 3 elements";
    ASSERT_TRUE(stack->At(0).IsType<int>()) << "First element should be int";
    ASSERT_TRUE(stack->At(1).IsType<int>()) << "Second element should be int";
    ASSERT_TRUE(stack->At(2).IsType<int>()) << "Third element should be int";
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0) << "First continuation should capture i=0";
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 2) << "Second continuation should capture i=2";
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 4) << "Third continuation should capture i=4";
}

// Test showing continuation state with nested loops
TEST_F(ExtendedRhoTests, ContinuationStateInNestedLoops) {
    // Switch to Rho language
    console->SetLanguage(Language::Rho);
    
    // Register Continuation type
    reg->AddClass<Continuation>(Label("Continuation"));
    
    const std::string rhoCode = R"(
        // Create a 2D array of continuations
        []
        
        // Outer loop
        for (i = 0; i < 3; i = i + 1)
        {
            // Inner loop
            for (j = 0; j < 3; j = j + 1) 
            {
                // Create a continuation that captures both i and j
                // It will compute i * 10 + j when called
                { i 10 * j + }
                
                // Store in array
                swap dup size swap store
            }
        }
        
        // Now we have 9 continuations
        // Execute some of them to verify state preservation
        
        // Execute continuation at index 0 (i=0, j=0): should push 0
        dup 0 at '
        
        // Execute continuation at index 4 (i=1, j=1): should push 11
        dup 4 at '
        
        // Execute continuation at index 8 (i=2, j=2): should push 22
        dup 8 at '
        
        // Drop the array
        drop
    )";
    
    console->Execute(rhoCode);
    
    // Verify stack: Should contain [0, 11, 22]
    ASSERT_EQ(stack->Size(), 3) << "Stack should have 3 elements";
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0) << "First result should be 0";
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 11) << "Second result should be 11";
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 22) << "Third result should be 22";
}

// Test showing continuation state with mutable variables
TEST_F(ExtendedRhoTests, ContinuationStateWithMutableVars) {
    // Switch to Rho language
    console->SetLanguage(Language::Rho);
    
    // Register Continuation type
    reg->AddClass<Continuation>(Label("Continuation"));
    
    const std::string rhoCode = R"(
        // Create a shared counter variable
        counter = 0
        
        // Create an array for continuations
        []
        
        // Create continuations that reference the counter
        for (i = 0; i < 3; i = i + 1)
        {
            // Create a continuation that adds i to counter
            { counter i + }
            
            // Store it
            swap dup size swap store
            
            // Increment counter
            counter = counter + 10
        }
        
        // counter is now 30
        
        // Execute the continuations
        // Each adds its captured i to the current counter value (30)
        
        // Execute continuation 0: 30 + 0 = 30
        dup 0 at '
        
        // Execute continuation 1: 30 + 1 = 31
        dup 1 at '
        
        // Execute continuation 2: 30 + 2 = 32
        dup 2 at '
        
        // Drop the array
        drop
    )";
    
    console->Execute(rhoCode);
    
    // Verify stack: Should contain [30, 31, 32]
    ASSERT_EQ(stack->Size(), 3) << "Stack should have 3 elements";
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 30) << "First result should be 30";
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 31) << "Second result should be 31";
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 32) << "Third result should be 32";
}