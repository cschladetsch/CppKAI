#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test 1: Do-while with continuation capture inside loop
TEST(RhoDoWhileCombined, DoWhileWithContinuationCapture) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create array to store continuations
        continuations = []
        counter = 0
        
        do
            // Capture current counter value in continuation
            { counter }
            continuations dup size swap store
            
            counter = counter + 1
        while counter < 3
        
        // Execute captured continuations
        continuations 0 at '  // Should push 0
        continuations 1 at '  // Should push 1
        continuations 2 at '  // Should push 2
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 2);
}

// Test 2: Nested do-while and for loop with shared continuations
TEST(RhoDoWhileCombined, NestedDoWhileForWithSharedContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        results = []
        outer = 0
        
        do
            for (i = 0; i < 2; i = i + 1)
            {
                // Create continuation that captures both loop variables
                { outer 10 * i + }
                results dup size swap store
            }
            outer = outer + 1
        while outer < 2
        
        // Execute continuations: should get 0, 1, 10, 11
        results 0 at '
        results 1 at '
        results 2 at '
        results 3 at '
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 10);
    ASSERT_EQ(ConstDeref<int>(stack->At(3)), 11);
}

// Test 3: Do-while that modifies continuations created in for loop
TEST(RhoDoWhileCombined, DoWhileModifyingForContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create continuations in for loop
        funcs = []
        for (i = 0; i < 3; i = i + 1)
        {
            { i 2 * }  // Double the index
            funcs dup size swap store
        }
        
        // Modify results using do-while
        sum = 0
        index = 0
        do
            funcs index at '  // Execute continuation
            sum = sum + 
            index = index + 1
        while index < funcs size
        
        sum  // Should be 0 + 2 + 4 = 6
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6);
}

// Test 4: For loop inside do-while with continuation state preservation
TEST(RhoDoWhileCombined, ForInsideDoWhileWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        total = 0
        multiplier = 1
        
        do
            // For loop creates and executes continuations
            for (i = 0; i < 3; i = i + 1)
            {
                // Continuation captures both multiplier and i
                { multiplier i * }
                '  // Execute immediately
                total = total + 
            }
            
            multiplier = multiplier + 1
        while multiplier <= 2
        
        total  // Should be (1*0 + 1*1 + 1*2) + (2*0 + 2*1 + 2*2) = 3 + 6 = 9
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 9);
}

// Test 5: Do-while with continuation-based conditional execution
TEST(RhoDoWhileCombined, DoWhileConditionalContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        count = 0
        evens = []
        odds = []
        
        do
            // Create different continuations based on parity
            if (count % 2 == 0)
            {
                { count 10 * }  // Even: multiply by 10
                evens dup size swap store
            }
            else
            {
                { count 5 * }   // Odd: multiply by 5
                odds dup size swap store
            }
            
            count = count + 1
        while count < 5
        
        // Execute all even continuations
        sum = 0
        for (i = 0; i < evens size; i = i + 1)
        {
            evens i at '
            sum = sum + 
        }
        
        sum  // Should be 0*10 + 2*10 + 4*10 = 60
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 60);
}

// Test 6: Continuation with do-while that breaks based on for loop result
TEST(RhoDoWhileCombined, ContinuationBreakFromForInDoWhile) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        attempts = 0
        found = false
        
        do
            attempts = attempts + 1
            
            // Search using for loop
            for (i = 0; i < 5; i = i + 1)
            {
                // Create continuation that checks condition
                { i attempts * }
                '  // Execute
                
                if (== 6)  // If result equals 6
                {
                    found = true
                    break
                }
                drop  // Drop result if not found
            }
            
            if (found)
            {
                break
            }
        while attempts < 10
        
        attempts  // Should be 2 (because 3*2 = 6)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Test 7: Do-while generating continuations that use for loops
TEST(RhoDoWhileCombined, DoWhileGeneratingForLoopContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        generators = []
        level = 0
        
        do
            // Create continuation that contains a for loop
            // Each continuation sums numbers up to 'level'
            {
                sum = 0
                for (j = 0; j <= level; j = j + 1)
                {
                    sum = sum + j
                }
                sum
            }
            generators dup size swap store
            
            level = level + 1
        while level < 3
        
        // Execute all generators
        generators 0 at '  // Sum 0 to 0 = 0
        generators 1 at '  // Sum 0 to 1 = 1
        generators 2 at '  // Sum 0 to 2 = 3
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 3);
}

// Test 8: Nested continuations with do-while and for interleaved
TEST(RhoDoWhileCombined, NestedContinuationsInterleaved) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create nested continuation structure
        outer_cont = {
            x = 0
            do
                inner_cont = {
                    for (y = 0; y < 2; y = y + 1)
                    {
                        x y +
                    }
                }
                inner_cont '
                x = x + 1
            while x < 2
        }
        
        // Execute outer continuation
        outer_cont '
        
        // Results should be: 0, 1 (from x=0), then 1, 2 (from x=1)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 0);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(2)), 1);
    ASSERT_EQ(ConstDeref<int>(stack->At(3)), 2);
}

// Test 9: Do-while with continuation-based accumulator pattern
TEST(RhoDoWhileCombined, DoWhileContinuationAccumulator) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Build a chain of continuations using do-while
        base = { 1 }
        counter = 0
        
        do
            // Wrap previous continuation
            old_base = base
            base = {
                old_base '  // Call previous
                2 *         // Double the result
            }
            counter = counter + 1
        while counter < 3
        
        // Execute the chained continuation
        base '  // Should be 1 * 2 * 2 * 2 = 8
        
        // Now use for loop to apply it multiple times
        result = 0
        for (i = 0; i < 3; i = i + 1)
        {
            base '
            result = result + 
        }
        
        result  // Should be 8 + 8 + 8 = 24
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 8);   // First execution
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 24);  // Sum of three executions
}

// Test 10: Complex control flow with continuations jumping between loops
TEST(RhoDoWhileCombined, ComplexControlFlowWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Matrix of continuations created by nested loops
        matrix = []
        phase = 0
        
        do
            row = []
            for (col = 0; col < 3; col = col + 1)
            {
                // Each continuation computes a unique value
                { phase 10 * col + }
                row dup size swap store
            }
            matrix dup size row store
            
            phase = phase + 1
        while phase < 2
        
        // Execute continuations in a specific pattern
        result = 0
        
        // Diagonal pattern
        for (i = 0; i < 2; i = i + 1)
        {
            matrix i at i at '  // matrix[i][i]
            result = result + 
        }
        
        // Anti-diagonal pattern
        do
            j = 0
            matrix j at (2 - j) at '  // matrix[0][2]
            result = result + 
            j = j + 1
        while j < 1
        
        result  // Should be 0 + 11 + 2 = 13
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 13);
}