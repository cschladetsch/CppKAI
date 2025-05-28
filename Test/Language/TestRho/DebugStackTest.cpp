#include <KAI/Console/Console.h>

#include "TestCommon.h"

using namespace kai;

TEST(DebugStack, FunctionCallStack) {
    Console console;

    // First, let's test that basic operations work
    console.Execute("x = 10");
    console.Execute("y = 20");
    console.Execute("z = x + y");

    auto stack = console.GetExecutor()->GetDataStack();
    stack->Clear();

    // Now test function definition and call separately
    console.Execute("fun testFunc(a, b)\n    return a + b");

    // Clear stack and push test values
    stack->Clear();
    stack->Push(console.GetRegistry()->New<int>(100));
    stack->Push(console.GetRegistry()->New<int>(200));

    // Check stack before function call
    ASSERT_EQ(stack->Size(), 2) << "Stack should have 2 values";
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 100)
        << "First value should be 100";
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 200)
        << "Second value should be 200";

    // Now call the function with values already on stack
    console.Execute("testFunc");
    console.Execute("Suspend");

    // The function should have consumed the values and returned the sum
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";
    auto result = stack->Pop();
    ASSERT_TRUE(result.IsType<int>()) << "Result should be integer";
    EXPECT_EQ(ConstDeref<int>(result), 300) << "100 + 200 = 300";
}