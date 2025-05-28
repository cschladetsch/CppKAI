#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using namespace kai;

TEST(DebugFunctionCall, MinimalFunctionExecution) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    try {
        // First define the function
        console.Execute(R"(
fun add(a, b)
    a + b
)", Structure::Program);
        
        // Clear stack
        console.GetExecutor()->GetDataStack()->Clear();
        
        // Now call it
        console.Execute("add(2, 3)", Structure::Expression);
        
        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty());
        EXPECT_EQ(ConstDeref<int>(stack->Top()), 5);
    }
    catch (const Exception::Base& e) {
        FAIL() << "Exception: " << e.ToString();
    }
}