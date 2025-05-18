#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// A very simple test for basic Pi operations
TEST(SimpleTest, BasicPi) {
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

    // Verify they have registries
    ASSERT_TRUE(a.GetRegistry() != nullptr)
        << "First integer has null registry";
    ASSERT_TRUE(b.GetRegistry() != nullptr)
        << "Second integer has null registry";

    // Perform direct addition
    Object result = exec->PerformBinaryOp(a, b, Operation::Plus);

    // Verify result exists
    ASSERT_TRUE(result.Exists()) << "Result doesn't exist";

    // Verify it has a registry
    ASSERT_TRUE(result.GetRegistry() != nullptr) << "Result has null registry";

    // Verify type and value
    ASSERT_TRUE(result.IsType<int>()) << "Result is not an int";
    ASSERT_EQ(ConstDeref<int>(result), 5) << "Result is not 5";

    // Now test executing a Pi expression directly
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Execute a simple addition
    console.Execute("2 3 +");

    // Check the result
    ASSERT_FALSE(stack->Empty()) << "Stack is empty after Pi execution";

    // Always unwrap the result to get primitive values
    Object execResult = stack->Top();
    cout << "Pi execution result type: " << execResult.GetClass()->GetName()
         << endl;

    // Unwrap the result if needed
    Object unwrapped = execResult;

    // If it's a continuation, try to extract the primitive value
    if (execResult.IsType<Continuation>()) {
        Pointer<Continuation> cont = execResult;
        if (cont->GetCode().Exists() && cont->GetCode()->Size() > 0) {
            // Try to get first element if it's a simple value
            if (cont->GetCode()->Size() == 1) {
                unwrapped = cont->GetCode()->At(0);
            }
        }
    }

    cout << "Unwrapped result type: " << unwrapped.GetClass()->GetName()
         << endl;

    // Replace the stack value
    stack->Pop();
    stack->Push(unwrapped);

    // Verify type and value of the Pi execution result
    ASSERT_TRUE(stack->Top().IsType<int>())
        << "Pi execution result is not an int, but: "
        << stack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5)
        << "Pi execution result is not 5, but: " << stack->Top().ToString();

    cout << "Basic test successful!" << endl;
}