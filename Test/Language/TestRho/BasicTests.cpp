#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// A simple test that just checks basic binary operations with direct executor
// calls
TEST(RhoBasic, DirectBinaryOps) {
    // Create a console
    Console console;

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

    cout << "Basic binary operation test successful!" << endl;
}