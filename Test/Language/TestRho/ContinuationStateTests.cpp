#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test showing that continuations capture and store state from loops
TEST(ExtendedRhoTests, ContinuationStateInLoop) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    // Create a Rho program that:
    // 1. Creates continuations inside a loop
    // 2. Each continuation captures the loop variable
    // 3. Shows that when called later, each continuation remembers its state
    const std::string rhoCode = R"(
        // Create an array to store continuations
        []
        
        // Loop from 0 to 4
        for (i = 0; i < 5; i = i + 1)
        {
            // Create a continuation that captures the current value of i
            // The continuation will push the captured value when called
            { i } 
            
            // Store the continuation in the array
            swap dup size swap store
        }
        
        // Now we have an array of 5 continuations
        // Let's execute them to show they preserved their state
        
        // Execute continuation 0 (should push 0)
        dup 0 at '
        
        // Execute continuation 2 (should push 2) 
        dup 2 at '
        
        // Execute continuation 4 (should push 4)
        dup 4 at '
        
        // Drop the array
        drop
    )";

    console.Execute(rhoCode);

    // Get the data stack
    auto stack = console.GetExecutor()->GetDataStack();

    // Verify stack: Should contain [0, 2, 4]
    // Each continuation remembered the value of i when it was created
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 4);
}

// Test showing continuation state with nested loops
TEST(ExtendedRhoTests, ContinuationStateInNestedLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string rhoCode = R"(
        // Create a 2D array of continuations
        []
        
        // Outer loop
        for (i = 0; i < 3; i = i + 1)
        {
            // Inner loop
            for (j = 0; j < 3; j = j + 1) 
            {
                // Create a continuation that captures both i and j
                // It will compute i * 10 + j when called
                { i 10 * j + }
                
                // Store in array
                swap dup size swap store
            }
        }
        
        // Now we have 9 continuations
        // Execute some of them to verify state preservation
        
        // Execute continuation at index 0 (i=0, j=0): should push 0
        dup 0 at '
        
        // Execute continuation at index 4 (i=1, j=1): should push 11
        dup 4 at '
        
        // Execute continuation at index 8 (i=2, j=2): should push 22
        dup 8 at '
        
        // Drop the array
        drop
    )";

    console.Execute(rhoCode);

    // Get the data stack
    auto stack = console.GetExecutor()->GetDataStack();

    // Verify stack: Should contain [0, 11, 22]
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 11);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 22);
}

// Test showing continuation state with mutable variables
TEST(ExtendedRhoTests, ContinuationStateWithMutableVars) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string rhoCode = R"(
        // Create a shared counter variable
        counter = 0
        
        // Create an array for continuations
        []
        
        // Create continuations that reference the counter
        for (i = 0; i < 3; i = i + 1)
        {
            // Create a continuation that adds i to counter
            { counter i + }
            
            // Store it
            swap dup size swap store
            
            // Increment counter
            counter = counter + 10
        }
        
        // counter is now 30
        
        // Execute the continuations
        // Each adds its captured i to the current counter value (30)
        
        // Execute continuation 0: 30 + 0 = 30
        dup 0 at '
        
        // Execute continuation 1: 30 + 1 = 31
        dup 1 at '
        
        // Execute continuation 2: 30 + 2 = 32
        dup 2 at '
        
        // Drop the array
        drop
    )";

    console.Execute(rhoCode);

    // Get the data stack
    auto stack = console.GetExecutor()->GetDataStack();

    // Verify stack: Should contain [30, 31, 32]
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 30);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 31);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 32);
}

// Test showing continuation state preservation with loop break
TEST(ExtendedRhoTests, ContinuationStateWithLoopBreak) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Register required types
    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string rhoCode = R"(
        // Array to store continuations
        []
        
        // First loop - will break early
        for (i = 0; i < 10; i = i + 1)
        {
            // Create continuation with current i value
            { i 100 + }
            
            // Store it
            swap dup size swap store
            
            // Break when i reaches 2
            if (i == 2) 
            {
                break
            }
        }
        
        // Second loop - normal completion
        for (j = 0; j < 3; j = j + 1)
        {
            // Create continuation with j value
            { j 200 + }
            
            // Store it
            swap dup size swap store
        }
        
        // We should have 6 continuations total:
        // 3 from first loop (i = 0, 1, 2)
        // 3 from second loop (j = 0, 1, 2)
        
        // Execute them to verify state
        dup 0 at '  // i=0: 100
        dup 2 at '  // i=2: 102
        dup 3 at '  // j=0: 200
        dup 5 at '  // j=2: 202
        
        // Drop the array
        drop
    )";

    console.Execute(rhoCode);

    // Get the data stack
    auto stack = console.GetExecutor()->GetDataStack();

    // Verify stack: Should contain [100, 102, 200, 202]
    ASSERT_EQ(stack->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 100);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 102);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 200);
    ASSERT_EQ(ConstDeref<int>(stack->At(3)), 202);
}