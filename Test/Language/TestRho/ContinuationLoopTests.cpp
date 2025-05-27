#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Simple test to demonstrate continuations storing state from loops
TEST(RhoLoopContinuations, ContinuationCapturesLoopVariable) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    // Simple test: create 3 continuations in a loop, each captures loop
    // variable
    const std::string code = R"(
        []  // array to store continuations
        
        // Create 3 continuations
        for (i = 0; i < 3; i = i + 1) {
            { i }  // continuation that pushes i
            swap dup size swap store
        }
        
        // Execute each continuation
        dup 0 at '  // should push 0
        dup 1 at '  // should push 1
        dup 2 at '  // should push 2
        
        drop  // drop the array
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 2);
}