#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// A very basic test to check simple binary operations with Pi
TEST(PiBasicTest, SimpleBinaryOps) {
    // Create a console
    Console console;
    console.SetLanguage(Language::Pi);

    // Get registry and add int type
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create a basic executor to test binary operations directly
    auto exec = console.GetExecutor();

    // Create two integers
    Object a = reg.New<int>(2);
    Object b = reg.New<int>(3);

    // Verify they exist
    ASSERT_TRUE(a.Exists()) << "First integer doesn't exist";
    ASSERT_TRUE(b.Exists()) << "Second integer doesn't exist";

    // Perform direct addition
    Object result = exec->PerformBinaryOp(a, b, Operation::Plus);

    // Verify result exists
    ASSERT_TRUE(result.Exists()) << "Result doesn't exist";

    // Verify it has a registry
    ASSERT_TRUE(result.GetRegistry() != nullptr) << "Result has null registry";

    // Verify type and value
    ASSERT_TRUE(result.IsType<int>()) << "Result is not an int";
    ASSERT_EQ(ConstDeref<int>(result), 5) << "Result is not 5";

    cout << "Simple binary operation test passed successfully!" << endl;
}

// A simple test for Pi stack operations
TEST(PiBasicTest, SimpleStackOps) {
    // Create a console with Pi language
    Console console;
    console.SetLanguage(Language::Pi);

    // Get registry and add int type
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Get executor and stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    try {
        // Push values directly to stack
        stack->Push(reg.New<int>(2));
        stack->Push(reg.New<int>(3));

        // Manually perform addition and replace values
        Object a = stack->Pop();
        Object b = stack->Pop();
        Object result = exec->PerformBinaryOp(b, a, Operation::Plus);
        stack->Push(result);

        // Verify result
        ASSERT_FALSE(stack->Empty()) << "Stack is empty";
        ASSERT_TRUE(stack->Top().Exists()) << "Result doesn't exist";
        ASSERT_TRUE(stack->Top().IsType<int>())
            << "Result is not an int, but: "
            << stack->Top().GetClass()->GetName();
        ASSERT_EQ(ConstDeref<int>(stack->Top()), 5)
            << "Result is not 5, but: " << stack->Top().ToString();

        cout << "Simple Pi stack operation test passed successfully!" << endl;
    } catch (const std::exception& e) {
        cout << "Exception during Pi stack operation: " << e.what() << endl;
        FAIL();
    }
}