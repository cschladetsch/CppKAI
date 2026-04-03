#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Object.h"

using namespace kai;
using namespace std;

// This test verifies basic examples from the Pi tutorial with reliable
// operations only
TEST(PiTutorial, BasicStackOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& registry = console.GetRegistry();
    registry.AddClass<int>(Label("int"));
    registry.AddClass<bool>(Label("bool"));
    registry.AddClass<String>(Label("String"));
    registry.AddClass<Array>(Label("Array"));

    Value<Stack> stack = console.GetExecutor()->GetDataStack();

    // Execute the Pi code directly to test the interpreter
    console.Execute("1 2 3 dup");

    ASSERT_EQ(stack->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 3);  // Top of stack
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 2);
    ASSERT_EQ(ConstDeref<int>(stack->At(3)), 1);  // Bottom of stack

    // This test just checks that the stack manipulation operations work as
    // described in the tutorial.
    SUCCEED();
}

// This test verifies basic arithmetic operations from the Pi tutorial
TEST(PiTutorial, BasicArithmetic) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& registry = console.GetRegistry();
    registry.AddClass<int>(Label("int"));

    Value<Stack> stack = console.GetExecutor()->GetDataStack();

    // Test addition
    console.Execute("2 3 +");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 5);

    stack->Clear();

    // Test subtraction
    console.Execute("5 3 -");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 2);

    stack->Clear();

    // Test multiplication
    console.Execute("4 5 *");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 20);

    // This validates that the arithmetic operations work as described in the
    // tutorial
    SUCCEED();
}

// This test validates that the examples in the tutorial can be demonstrated
TEST(PiTutorial, TutorialExamples) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& registry = console.GetRegistry();
    registry.AddClass<int>(Label("int"));
    registry.AddClass<bool>(Label("bool"));
    registry.AddClass<String>(Label("String"));
    registry.AddClass<Array>(Label("Array"));

    Value<Stack> stack = console.GetExecutor()->GetDataStack();

    // Example: Using dup for square function
    console.Execute("4 dup *");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 16);

    stack->Clear();

    // Example: Using swap
    console.Execute("1 2 swap");
    ASSERT_EQ(stack->Size(), 2);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 2);

    stack->Clear();

    // Example: Using conditionals (true branch)
    console.Execute("1 2 true ife");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 1);

    stack->Clear();

    // Example: Using conditionals (false branch)
    console.Execute("1 2 false ife");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 2);

    // This validates that the examples in the tutorial can be demonstrated
    SUCCEED();
}