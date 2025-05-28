#include <KAI/Console/Console.h>

#include "TestCommon.h"

using namespace kai;

TEST(MinimalFunction, JustStore) {
    // Test that basic Store operation works
    Console console;

    // Push a value and a name, then store
    console.Execute("42");     // Push 42
    console.Execute("'x");     // Push pathname 'x
    console.Execute("Store");  // Store 42 as x

    // Now retrieve x
    console.Execute("x");

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have x's value";

    auto result = stack->Pop();
    ASSERT_TRUE(result.IsType<int>()) << "Result should be integer";
    EXPECT_EQ(ConstDeref<int>(result), 42);
}

TEST(MinimalFunction, ManualFunctionCall) {
    Console console;

    // Define a function manually using Pi-like operations
    console.Execute(R"(
fun simpleAdd(x, y)
    return x + y
)");

    // Now manually set up a function call
    // Push arguments
    console.Execute("10");  // First argument
    console.Execute("20");  // Second argument

    // Get the function and call it
    console.Execute("simpleAdd");  // Get the function
    console.Execute("Suspend");    // Call it

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";

    auto result = stack->Pop();
    if (result.IsType<int>()) {
        EXPECT_EQ(ConstDeref<int>(result), 30) << "10 + 20 = 30";
    } else {
        FAIL() << "Result is not an integer, type: "
               << result.GetTypeNumber().ToInt();
    }
}