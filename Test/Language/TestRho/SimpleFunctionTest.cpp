#include "KAI/Console/Console.h"
#include "TestCommon.h"

using namespace KAI;
using namespace std;

TEST(SimpleFunctionTest, IdentityFunction) {
    // Very simple function that just returns its argument
    try {
        Console console;

        // Test 1: Identity function
        console.Execute("fun identity(x)\n    return x\nidentity(42)");

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";

        auto result = stack->Pop();
        ASSERT_TRUE(result.IsType<int>()) << "Result should be an integer";
        EXPECT_EQ(ConstDeref<int>(result), 42)
            << "Identity(42) should return 42";

    } catch (const Exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}

TEST(SimpleFunctionTest, AddTwoNumbers) {
    // Simple addition function
    try {
        Console console;

        // Test 2: Add function
        console.Execute("fun add(a, b)\n    return a + b\nadd(3, 4)");

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";

        auto result = stack->Pop();
        ASSERT_TRUE(result.IsType<int>()) << "Result should be an integer";
        EXPECT_EQ(ConstDeref<int>(result), 7) << "add(3, 4) should return 7";

    } catch (const Exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}

TEST(SimpleFunctionTest, ConstantFunction) {
    // Function that returns a constant
    try {
        Console console;

        // Test 3: Constant function
        console.Execute("fun getConstant()\n    return 100\ngetConstant()");

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";

        auto result = stack->Pop();
        ASSERT_TRUE(result.IsType<int>()) << "Result should be an integer";
        EXPECT_EQ(ConstDeref<int>(result), 100)
            << "getConstant() should return 100";

    } catch (const Exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}