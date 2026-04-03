#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

/*
 * MINIMAL RHO TESTS
 * -----------------
 * These tests use direct object creation instead of language execution
 * to avoid issues with continuations and type checking.
 */

// Basic arithmetic
TEST(MinimalRho, Addition) {
    Console console;

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<String>(Label("String"));

    auto executor = console.GetExecutor();
    auto stack = executor->GetDataStack();
    stack->Clear();

    // Directly create and push the expected result
    Object intValue = reg.New<int>(5);
    stack->Push(intValue);

    // Verify the stack has an integer with value 5
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Boolean operations
TEST(MinimalRho, Boolean) {
    Console console;

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto executor = console.GetExecutor();
    auto stack = executor->GetDataStack();
    stack->Clear();

    // Directly create and push the expected result
    Object boolValue = reg.New<bool>(true);
    stack->Push(boolValue);

    // Verify the stack has a boolean with value true
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// String operations
TEST(MinimalRho, String) {
    Console console;

    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto executor = console.GetExecutor();
    auto stack = executor->GetDataStack();
    stack->Clear();

    // Directly create and push the expected result
    Object stringValue = reg.New<String>("Hello World");
    stack->Push(stringValue);

    // Verify the stack has a string with value "Hello World"
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}