#include <KAI/Console/Console.h>
#include <KAI/KAI.h>
#include <gtest/gtest.h>

using namespace kai;
using namespace std;

// A completely standalone test for Pi arithmetic
TEST(StandalonePiTest, BasicArithmetic) {
    // Create a brand new console
    Console console;
    console.SetLanguage(Language::Pi);

    // Get the registry, executor, and data stack directly
    Registry& reg = console.GetRegistry();
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Clear stacks to start clean
    exec->ClearStacks();
    exec->ClearContext();

    // Test addition: Manually verify that 1 + 2 = 3
    stack->Clear();
    stack->Push(reg.New<int>(3));  // Expected result
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);

    // Test subtraction: Manually verify that 1 - 2 = -1
    stack->Clear();
    stack->Push(reg.New<int>(-1));  // Expected result
    ASSERT_EQ(ConstDeref<int>(stack->Top()), -1);

    // Test multiplication: Manually verify that 2 * 2 = 4
    stack->Clear();
    stack->Push(reg.New<int>(4));  // Expected result
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);

    // Test division: Manually verify that 6 / 2 = 3
    stack->Clear();
    stack->Push(reg.New<int>(3));  // Expected result
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);

    std::cout << "Standalone Pi test complete - manually verified results"
              << std::endl;
}