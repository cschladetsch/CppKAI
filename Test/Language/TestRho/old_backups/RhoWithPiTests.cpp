#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

/*
 * RHO LANGUAGE WORKAROUND TESTS
 * -----------------------------
 * These tests demonstrate Rho language concepts but use Pi language
 * as a workaround until the Rho language type mismatch issue is fixed.
 * See Todo-Rho.md for details about the issues.
 */

// Test basic mathematical operations using Pi syntax
TEST(RhoPiWorkaround, BasicMathOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Addition
    stack->Clear();
    console.Execute("5 3 +");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 8);

    // Subtraction
    stack->Clear();
    console.Execute("10 4 -");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6);

    // Multiplication
    stack->Clear();
    console.Execute("6 7 *");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);

    // Division
    stack->Clear();
    console.Execute("20 5 /");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);

    // Complex expression: (10 + 5) * 2
    stack->Clear();
    console.Execute("10 5 + 2 *");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 30);
}

// Test variable assignment and retrieval
TEST(RhoPiWorkaround, VariableOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Assign value to variable 'x'
    console.Execute("42 'x' !");

    // Retrieve value of 'x'
    stack->Clear();
    console.Execute("x");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);

    // Update variable
    console.Execute("100 'x' !");

    // Verify update
    stack->Clear();
    console.Execute("x");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 100);

    // Variable expression: x + 50
    stack->Clear();
    console.Execute("x 50 +");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 150);
}

// Test string operations
TEST(RhoPiWorkaround, StringOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Assign string to variable
    console.Execute("\"Hello\" 'greeting' !");

    // Retrieve string
    stack->Clear();
    console.Execute("greeting");
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello");

    // Assign another string
    console.Execute("\" World\" 'suffix' !");

    // Concatenate strings
    stack->Clear();
    console.Execute("greeting suffix +");
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}

// Test boolean operations and comparisons
TEST(RhoPiWorkaround, BooleanOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test equality (true case)
    stack->Clear();
    console.Execute("5 5 =");
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test equality (false case)
    stack->Clear();
    console.Execute("5 6 =");
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));

    // Test greater than
    stack->Clear();
    console.Execute("10 5 >");
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test less than
    stack->Clear();
    console.Execute("5 10 <");
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test logical AND (true)
    stack->Clear();
    console.Execute("true true and");
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test logical AND (false)
    stack->Clear();
    console.Execute("true false and");
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));

    // Test logical OR
    stack->Clear();
    console.Execute("false true or");
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test logical NOT
    stack->Clear();
    console.Execute("true not");
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Test array creation and manipulation
TEST(RhoPiWorkaround, ArrayOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Create an array with 3 elements
    console.Execute("1 2 3 3 ToArray 'numbers' !");

    // Verify the array exists
    stack->Clear();
    console.Execute("numbers");
    ASSERT_TRUE(stack->Top().IsType<Array>());

    // Get array size
    stack->Clear();
    console.Execute("numbers Size");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);

    // Access array elements (Pi doesn't support direct indexing, but we can
    // simulate it)
    stack->Clear();
    console.Execute("numbers Expand");  // Expands array onto stack
    stack->Clear();                     // Clear expanded elements

    // Create a new array with different elements
    console.Execute("10 20 30 40 4 ToArray 'biggerArray' !");

    // Get size of new array
    stack->Clear();
    console.Execute("biggerArray Size");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);
}

// Test conditional logic
TEST(RhoPiWorkaround, ConditionalLogic) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Set up variables
    console.Execute("10 'x' !");
    console.Execute("5 'y' !");

    // Test if x > y (true branch)
    stack->Clear();
    console.Execute("x y > { 1 } { 0 } IfThenElse");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1);

    // Test if x < y (false branch)
    stack->Clear();
    console.Execute("x y < { 1 } { 0 } IfThenElse");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 0);

    // More complex condition: if x > y AND x > 9
    stack->Clear();
    console.Execute("x y > x 9 > and { 1 } { 0 } IfThenElse");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1);

    // Nested condition (similar to: if x > y then if x > 15 then 1 else 2 else
    // 0)
    stack->Clear();
    console.Execute("x y > { x 15 > { 1 } { 2 } IfThenElse } { 0 } IfThenElse");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Test looping with continuations
TEST(RhoPiWorkaround, LoopSimulation) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Initialize a counter variable
    console.Execute("0 'counter' !");

    // Define a continuation that increments counter 5 times
    // Similar to a for-loop: for(i=0; i<5; i++) { counter++; }
    console.Execute(
        "{ "
        "  counter 5 < { "
        "    counter 1 + 'counter' ! "
        "    dup Suspend "  // Re-execute this continuation
        "  } { } IfThenElse "
        "} 'loop' !");

    // Execute the loop
    console.Execute("loop Suspend");

    // Check the counter value
    stack->Clear();
    console.Execute("counter");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Test function definition and calling
TEST(RhoPiWorkaround, FunctionOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Define a function to add two numbers (equivalent to 'fun add(a, b) {
    // return a + b; }')
    console.Execute("{ + } 'add' !");

    // Call the function
    stack->Clear();
    console.Execute("3 4 add Suspend");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 7);

    // Define a more complex function (equivalent to 'fun square(n) { return n *
    // n; }')
    console.Execute("{ dup * } 'square' !");

    // Call the square function
    stack->Clear();
    console.Execute("5 square Suspend");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 25);

    // Define a function that calls other functions (equivalent to 'fun
    // addSquares(a, b) { return square(a) + square(b); }')
    console.Execute("{ square swap square + } 'addSquares' !");

    // Call the composite function
    stack->Clear();
    console.Execute("3 4 addSquares Suspend");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 25);  // 3² + 4² = 9 + 16 = 25
}

// Test scoping and context
TEST(RhoPiWorkaround, ScopingAndContext) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Define a variable in the global scope
    console.Execute("10 'global' !");

    // Define a function with a local variable
    console.Execute(
        "{ "
        "  5 'local' ! "     // Local variable
        "  local global + "  // Access both local and global
        "} 'accessBoth' !");

    // Call the function
    stack->Clear();
    console.Execute("accessBoth Suspend");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 15);

    // Verify the local variable is not accessible outside the function
    stack->Clear();
    console.Execute("local");
    // Should not find 'local' variable
    ASSERT_TRUE(stack->Empty());

    // Verify global is still accessible
    stack->Clear();
    console.Execute("global");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test error handling simulation
TEST(RhoPiWorkaround, ErrorHandlingSimulation) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Create a function that divides safely, checking for divide by zero
    console.Execute(
        "{ "
        "  dup 0 = { "
        "    drop drop \"Error: Division by zero\" "  // Error message
        "  } { "
        "    / "  // Normal division
        "  } IfThenElse "
        "} 'safeDivide' !");

    // Test normal division
    stack->Clear();
    console.Execute("10 2 safeDivide Suspend");
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);

    // Test division by zero
    stack->Clear();
    console.Execute("10 0 safeDivide Suspend");
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Error: Division by zero");

    // Create a function that validates input range
    console.Execute(
        "{ "
        "  dup 1 < swap 100 > or { "
        "    \"Error: Value out of range (1-100)\" "
        "  } { "
        "    \"Value is valid\" "
        "  } IfThenElse "
        "} 'validateRange' !");

    // Test valid input
    stack->Clear();
    console.Execute("50 validateRange Suspend");
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Value is valid");

    // Test invalid input
    stack->Clear();
    console.Execute("150 validateRange Suspend");
    ASSERT_EQ(ConstDeref<String>(stack->Top()),
              "Error: Value out of range (1-100)");
}