#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test do-while loop implementation in Rho
TEST(RhoDoWhileTests, BasicDoWhileLoop) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    // Test basic do-while loop that executes at least once
    const std::string code = R"(
        counter = 0
        do
            counter = counter + 1
        while counter < 3
        
        counter  // Push final value to stack
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);
}

// Test do-while that should execute only once (condition false after first
// iteration)
TEST(RhoDoWhileTests, DoWhileExecutesOnce) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 10
        do
            counter = counter + 1
        while counter < 5  // False after first iteration
        
        counter  // Should be 11
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 11);
}

// Test nested do-while loops
TEST(RhoDoWhileTests, NestedDoWhileLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        outer = 0
        total = 0
        
        do
            inner = 0
            do
                total = total + 1
                inner = inner + 1
            while inner < 2
            
            outer = outer + 1
        while outer < 3
        
        total  // Should be 6 (3 outer * 2 inner)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6);
}

// Test do-while with break
TEST(RhoDoWhileTests, DoWhileWithBreak) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 0
        do
            counter = counter + 1
            if (counter == 2)
            {
                break
            }
        while counter < 10
        
        counter  // Should be 2
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Test do-while with continue
TEST(RhoDoWhileTests, DoWhileWithContinue) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 0
        sum = 0
        
        do
            counter = counter + 1
            
            // Add odd numbers
            if (counter % 2 != 0)
            {
                sum = sum + counter
            }
        while counter < 5
        
        sum  // Should be 1 + 3 + 5 = 9
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 9);
}