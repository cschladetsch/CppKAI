#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"
#include "SimpleRhoPiTests.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * CONTINUATION CONTROL TESTS
 * -------------------------
 * This file contains tests specifically focused on Suspend, Resume, and Replace
 * operations within KAI's continuation-based execution model, with special
 * emphasis on their usage within For loops.
 */

// Test fixture for continuation control operations
class ContinuationControlTests : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        // These tests will use Pi language since it gives more direct control
        // over continuations
        console_.SetLanguage(Language::Pi);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
    }

    // Helper to create a simple continuation with a single integer value
    Object CreateConstantContinuation(int value) {
        Pointer<Continuation> continuation = reg_->New<Continuation>();
        continuation->Create();

        Pointer<Array> code = reg_->New<Array>();
        code->Append(reg_->New<Operation>(Operation::ContinuationBegin));
        code->Append(reg_->New<int>(value));
        code->Append(reg_->New<Operation>(Operation::ContinuationEnd));

        continuation->SetCode(code);
        return continuation;
    }

    // Helper to create a continuation with custom code array
    Object CreateCustomContinuation(const std::vector<Object>& codeObjects) {
        Pointer<Continuation> continuation = reg_->New<Continuation>();
        continuation->Create();

        Pointer<Array> code = reg_->New<Array>();
        for (const auto& obj : codeObjects) {
            code->Append(obj);
        }

        continuation->SetCode(code);
        return continuation;
    }

    // Helper to verify stack content
    void VerifyStack(
        const std::vector<std::pair<std::string, int>>& expectedValues) {
        // Make sure we unwrap any continuation values
        UnwrapStackValues(data_, exec_);

        ASSERT_EQ(data_->Size(), expectedValues.size())
            << "Stack size mismatch";

        // Verify stack contents from bottom to top
        for (size_t i = 0; i < expectedValues.size(); ++i) {
            const auto& expected = expectedValues[i];
            const auto& actual = data_->At(i);

            if (expected.first == "int") {
                ASSERT_TRUE(actual.IsType<int>())
                    << "Stack item " << i << " should be int but got "
                    << (actual.GetClass()
                            ? actual.GetClass()->GetName().ToString()
                            : "<null>");
                ASSERT_EQ(ConstDeref<int>(actual), expected.second)
                    << "Stack item " << i << " value mismatch";
            } else if (expected.first == "bool") {
                ASSERT_TRUE(actual.IsType<bool>())
                    << "Stack item " << i << " should be bool but got "
                    << (actual.GetClass()
                            ? actual.GetClass()->GetName().ToString()
                            : "<null>");
                ASSERT_EQ(ConstDeref<bool>(actual), expected.second != 0)
                    << "Stack item " << i << " value mismatch";
            }
        }
    }

    // Helper to execute a continuation explicitly
    void ExecuteContinuation(Object continuation) {
        try {
            exec_->Continue(continuation);
        } catch (const std::exception& e) {
            FAIL() << "Failed to execute continuation: " << e.what();
        }
    }
};

// Basic Resume Operation
TEST_F(ContinuationControlTests, BasicResumeOperation) {
    // The Resume operation should exit the current continuation and return to
    // the calling context
    data_->Clear();

    // Create two continuations:
    // Outer continuation that pushes 1, executes inner continuation, then
    // pushes 3
    // Inner continuation that pushes 2, then calls Resume to exit back to
    // outer continuation

    // Inner continuation: Push 2, then Resume
    std::vector<Object> innerCode = {
        reg_->New<Operation>(Operation::ContinuationBegin), reg_->New<int>(2),
        reg_->New<Operation>(Operation::Resume),
        reg_->New<int>(99),  // This should never execute due to Resume
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object innerCont = CreateCustomContinuation(innerCode);

    // Outer continuation: Push 1, execute inner continuation, push 3
    std::vector<Object> outerCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(1),
        innerCont,
        reg_->New<Operation>(Operation::Apply),
        reg_->New<int>(3),
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object outerCont = CreateCustomContinuation(outerCode);

    // Execute outer continuation
    exec_->Continue(outerCont);

    // Verify stack: Should contain [1, 2, 3] (not 99)
    // Resume skips the rest of the inner continuation but allows outer to
    // continue
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 1}, {"int", 2}, {"int", 3}};
    VerifyStack(expected);
}

// Basic Suspend Operation
TEST_F(ContinuationControlTests, BasicSuspendOperation) {
    // The Suspend operation should suspend the current continuation and start
    // executing a new one
    data_->Clear();

    // Create two continuations:
    // First continuation that pushes 1, suspends to second continuation
    // Second continuation that pushes 2

    // Second continuation: Simply push 2
    std::vector<Object> secondCode = {
        reg_->New<Operation>(Operation::ContinuationBegin), reg_->New<int>(2),
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object secondCont = CreateCustomContinuation(secondCode);

    // First continuation: Push 1, push second continuation, call Suspend, push
    // 3 (which won't execute yet)
    std::vector<Object> firstCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(1),
        secondCont,
        reg_->New<Operation>(Operation::Suspend),
        reg_->New<int>(
            3),  // This won't execute until the second continuation completes
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object firstCont = CreateCustomContinuation(firstCode);

    // Execute first continuation
    exec_->Continue(firstCont);

    // Verify stack: Should contain [1, 2]
    // Suspend switches to second continuation, but doesn't complete the first
    // one
    std::vector<std::pair<std::string, int>> expected = {{"int", 1},
                                                         {"int", 2}};
    VerifyStack(expected);

    // Now resume the suspended continuation
    exec_->Resume();

    // Verify stack: Should now contain [1, 2, 3]
    expected.push_back({"int", 3});
    VerifyStack(expected);
}

// Basic Replace Operation
TEST_F(ContinuationControlTests, BasicReplaceOperation) {
    // The Replace operation should replace the current continuation with a new
    // one
    data_->Clear();

    // Create two continuations:
    // First continuation that pushes 1, replaces itself with second
    // continuation
    // Second continuation that pushes 2

    // Second continuation: Simply push 2
    std::vector<Object> secondCode = {
        reg_->New<Operation>(Operation::ContinuationBegin), reg_->New<int>(2),
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object secondCont = CreateCustomContinuation(secondCode);

    // First continuation: Push 1, push second continuation, call Replace, push
    // 3 (which won't execute)
    std::vector<Object> firstCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(1),
        secondCont,
        reg_->New<Operation>(Operation::Replace),
        reg_->New<int>(3),  // This will never execute due to Replace
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object firstCont = CreateCustomContinuation(firstCode);

    // Execute first continuation
    exec_->Continue(firstCont);

    // Verify stack: Should contain [1, 2]
    // Replace switches to second continuation without pushing to the context
    // stack
    std::vector<std::pair<std::string, int>> expected = {{"int", 1},
                                                         {"int", 2}};
    VerifyStack(expected);

    // Unlike Suspend, Resume after Replace should not continue with the
    // original continuation This might throw an exception if there's no
    // continuation to resume
    try {
        exec_->Resume();
        // If we got here, it resumed something, which we'll check
        if (data_->Size() > 2) {
            FAIL() << "Resume executed after Replace, which should not happen";
        }
    } catch (const std::exception&) {
        // Expected behavior - no continuation to resume
    }

    // Stack should still be [1, 2]
    VerifyStack(expected);
}

// Resume Within a For Loop
TEST_F(ContinuationControlTests, ResumeWithinForLoop) {
    // Test using Resume to break out of a For loop early
    data_->Clear();

    // Create a for loop that counts to 10, but uses Resume to break at 5
    // We'll implement this using Pi code for simplicity

    // In Pi syntax, this would be something like:
    // 0 1 10 {
    //   dup 5 >= { resume } if
    //   dup
    // } for

    const std::string piCode = R"(
        0       // Initial counter
        1       // Step
        10      // Limit
        {       // Loop body
            dup 5 >= { resume } if  // If counter >= 5, resume (break)
            dup                     // Duplicate counter for visualization
        } for   // For loop
    )";

    console_.Execute(piCode);

    // Verify stack: Should contain [0, 1, 2, 3, 4, 5]
    // Note: The loop counter (5) is also on the stack
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 1}, {"int", 2}, {"int", 3}, {"int", 4}, {"int", 5}};
    VerifyStack(expected);
}

// Suspend Within a For Loop
TEST_F(ContinuationControlTests, SuspendWithinForLoop) {
    // Test suspending execution within a for loop and resuming it later
    data_->Clear();

    // First, create a continuation that will be suspended to
    std::vector<Object> breakoutCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(99),  // Push 99 as a marker
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object breakoutCont = CreateCustomContinuation(breakoutCode);

    // Store it in a variable to access from Pi code
    console_.SetValue(Label("breakout_cont"), breakoutCont);

    // In Pi syntax, this would be like:
    // 0 1 5 {
    //   dup 3 == { breakout_cont suspend } if
    //   dup
    // } for
    const std::string piCode = R"(
        0       // Initial counter
        1       // Step
        5       // Limit
        {       // Loop body
            dup 3 == { breakout_cont suspend } if  // If counter == 3, suspend to breakout_cont
            dup                                    // Duplicate counter for visualization
        } for   // For loop
    )";

    console_.Execute(piCode);

    // Verify stack: Should contain [0, 1, 2, 3, 99]
    // The loop suspended at 3 and executed breakout_cont
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 1}, {"int", 2}, {"int", 3}, {"int", 99}};
    VerifyStack(expected);

    // Now resume the suspended for loop
    exec_->Resume();

    // Verify stack: Should now contain [0, 1, 2, 3, 99, 3, 4, 4]
    // Note that 3 appears twice - once before suspension and once after
    // resuming
    expected = {
        {"int", 0},
        {"int", 1},
        {"int", 2},
        {"int", 3},
        {"int", 99},
        {"int", 3},  // Counter value after resuming (dup was
                     // already called for 3)
        {"int", 4},
        {"int", 4}  // Last iteration duplicates 4
    };
    VerifyStack(expected);
}

// Replace Within a For Loop
TEST_F(ContinuationControlTests, ReplaceWithinForLoop) {
    // Test replacing execution within a for loop
    data_->Clear();

    // First, create a continuation that will replace the for loop
    std::vector<Object> replacementCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(99),   // Push 99 as a marker
        reg_->New<int>(100),  // Push 100 as another marker
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object replacementCont = CreateCustomContinuation(replacementCode);

    // Store it in a variable to access from Pi code
    console_.SetValue(Label("replacement_cont"), replacementCont);

    // In Pi syntax, this would be like:
    // 0 1 5 {
    //   dup 3 == { replacement_cont replace } if
    //   dup
    // } for
    const std::string piCode = R"(
        0       // Initial counter
        1       // Step
        5       // Limit
        {       // Loop body
            dup 3 == { replacement_cont replace } if  // If counter == 3, replace with replacement_cont
            dup                                       // Duplicate counter for visualization
        } for   // For loop
    )";

    console_.Execute(piCode);

    // Verify stack: Should contain [0, 1, 2, 3, 99, 100]
    // The loop replaced at 3 and executed replacement_cont
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 1},  {"int", 2},
        {"int", 3}, {"int", 99}, {"int", 100}};
    VerifyStack(expected);

    // Unlike suspend, resume after replace should not continue the for loop
    try {
        exec_->Resume();
        // If we got here, check if for loop continued
        if (data_->Size() > 6) {
            FAIL()
                << "For loop continued after Replace, which should not happen";
        }
    } catch (const std::exception&) {
        // Expected behavior - no continuation to resume
    }

    // Stack should still be the same
    VerifyStack(expected);
}

// Nested For Loops with Resume
TEST_F(ContinuationControlTests, NestedForLoopsWithResume) {
    // Test using Resume to break out of nested for loops
    data_->Clear();

    // In Pi syntax, create nested loops where the inner loop uses Resume
    // to break out completely (skipping both inner and outer loops)
    const std::string piCode = R"(
        0     // Result accumulator
        
        0 1 3 {  // Outer loop: i = 0 to 2
            0 1 3 {  // Inner loop: j = 0 to 2
                // If i == 1 and j == 1, break out completely
                over over 1 == swap 1 == && { resume } if
                
                // Otherwise add i*j to the accumulator
                over over * rot + swap rot
            } for
            drop  // Drop j
        } for
        drop  // Drop i
    )";

    console_.Execute(piCode);

    // Calculate expected result:
    // Iterations: (i=0,j=0): 0 += 0*0 = 0
    //             (i=0,j=1): 0 += 0*1 = 0
    //             (i=0,j=2): 0 += 0*2 = 0
    //             (i=1,j=0): 0 += 1*0 = 0
    //             (i=1,j=1): Resume breaks out of both loops
    // Result should be 0

    // Verify stack: Should contain only [0] (the accumulator)
    std::vector<std::pair<std::string, int>> expected = {{"int", 0}};
    VerifyStack(expected);
}

// Conditional Suspend in a For Loop
TEST_F(ContinuationControlTests, ConditionalSuspendInForLoop) {
    // Test conditionally suspending execution within a for loop
    data_->Clear();

    // Create a target continuation
    std::vector<Object> targetCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(42),  // Push 42 as a marker
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object targetCont = CreateCustomContinuation(targetCode);

    // Store it in a variable
    console_.SetValue(Label("target_cont"), targetCont);

    // Create a for loop with conditional suspend based on whether the value is
    // even
    const std::string piCode = R"(
        0 1 5 {                      // For loop from 0 to 4
            dup 2 % 0 ==             // Check if counter is even
            { target_cont suspend }  // If even, suspend to target continuation
            { dup }                  // Otherwise, duplicate counter
            if
        } for
    )";

    console_.Execute(piCode);

    // The loop should suspend at 0, execute target_cont, resume,
    // then suspend at 2, execute target_cont again, resume,
    // then suspend at 4, execute target_cont again, and finish

    // Verify stack: Should contain [0, 42]
    // First iteration (i=0) triggered suspend
    std::vector<std::pair<std::string, int>> expected = {{"int", 0},
                                                         {"int", 42}};
    VerifyStack(expected);

    // Resume first time
    exec_->Resume();

    // Verify stack: Should now contain [0, 42, 1, 2, 42]
    // Second iteration (i=1) did not trigger suspend, then i=2 triggered
    // suspend
    expected = {{"int", 0}, {"int", 42}, {"int", 1}, {"int", 2}, {"int", 42}};
    VerifyStack(expected);

    // Resume second time
    exec_->Resume();

    // Verify stack: Should now contain [0, 42, 1, 2, 42, 3, 4, 42]
    // Third iteration (i=3) did not trigger suspend, then i=4 triggered suspend
    expected = {{"int", 0},  {"int", 42}, {"int", 1}, {"int", 2},
                {"int", 42}, {"int", 3},  {"int", 4}, {"int", 42}};
    VerifyStack(expected);

    // Resume third time to complete the loop
    exec_->Resume();

    // We expect no more changes to the stack since the loop is complete
    VerifyStack(expected);
}

// Mixing Suspend and Replace in a For Loop
TEST_F(ContinuationControlTests, MixingSuspendAndReplaceInForLoop) {
    // Test mixing Suspend and Replace operations within a for loop
    data_->Clear();

    // Create two target continuations
    std::vector<Object> suspendTargetCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(100),  // Push 100 as a marker
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object suspendTarget = CreateCustomContinuation(suspendTargetCode);

    std::vector<Object> replaceTargetCode = {
        reg_->New<Operation>(Operation::ContinuationBegin),
        reg_->New<int>(200),  // Push 200 as a marker
        reg_->New<Operation>(Operation::ContinuationEnd)};
    Object replaceTarget = CreateCustomContinuation(replaceTargetCode);

    // Store them in variables
    console_.SetValue(Label("suspend_target"), suspendTarget);
    console_.SetValue(Label("replace_target"), replaceTarget);

    // Create a for loop that:
    // - At i=1: Suspends to suspend_target
    // - At i=3: Replaces with replace_target
    const std::string piCode = R"(
        0 1 5 {                      // For loop from 0 to 4
            dup 1 ==                 // Check if counter == 1
            { suspend_target suspend }  // If counter == 1, suspend
            {
                dup 3 ==                // Check if counter == 3
                { replace_target replace }  // If counter == 3, replace
                { dup }                 // Otherwise, duplicate counter
                if
            }
            if
        } for
    )";

    console_.Execute(piCode);

    // Verify stack: Should contain [0, 1, 100]
    // First special case (i=1) triggered suspend
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 1}, {"int", 100}};
    VerifyStack(expected);

    // Resume to continue the loop
    exec_->Resume();

    // Verify stack: Should now contain [0, 1, 100, 1, 2, 3, 200]
    // Second special case (i=3) triggered replace, ending the for loop
    expected = {{"int", 0}, {"int", 1}, {"int", 100},
                {"int", 1},  // After resuming
                {"int", 2}, {"int", 3}, {"int", 200}};
    VerifyStack(expected);

    // Stack should not change further since replace terminated the for loop
    VerifyStack(expected);
}

// Resume for Early Loop Termination with Result
TEST_F(ContinuationControlTests, ResumeForEarlyLoopTerminationWithResult) {
    // Test using Resume to exit a loop early while still capturing a result
    data_->Clear();

    // This test simulates a search operation in an array where we exit
    // early when the target value is found

    // In Pi, search for the first occurrence of 42 in a list
    const std::string piCode = R"(
        // Initialize result to -1 (not found)
        -1
        
        // Create a "virtual array" for searching by pushing each element:
        // Array to search: [10, 20, 42, 30, 40]
        
        // Loop from 0 to 4 (array indices)
        0 1 5 {
            // Check the value at this index (manually simulated)
            dup 0 == { 10 } if  // array[0] = 10
            dup 1 == { 20 } if  // array[1] = 20
            dup 2 == { 42 } if  // array[2] = 42
            dup 3 == { 30 } if  // array[3] = 30
            dup 4 == { 40 } if  // array[4] = 40
            
            // Check if current value == 42
            42 == {
                // Found 42! Update result and exit loop
                pop    // Remove old result (-1)
                rot    // Bring the index to the top
                dup    // Duplicate it for the result
                rot    // Put it back as the loop counter
                swap   // Ensure the result is on the stack top
                
                resume  // Exit the loop early
            } if
            
            // Value wasn't 42, continue to next iteration
            drop  // Drop the array value
        } for
        
        // Drop the final loop counter
        drop
    )";

    console_.Execute(piCode);

    // Verify stack: Should contain [2] (the index where 42 was found)
    std::vector<std::pair<std::string, int>> expected = {{"int", 2}};
    VerifyStack(expected);
}

// Test showing that continuations capture and store state from loops
TEST_F(ContinuationControlTests, ContinuationStateInRhoLoop) {
    // This test demonstrates that continuations in Rho capture and store
    // their creation context, including loop variables

    // Switch to Rho language for this test
    console_.SetLanguage(Language::Rho);

    // Clear the stack
    data_->Clear();

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

    console_.Execute(rhoCode);

    // Verify stack: Should contain [0, 2, 4]
    // Each continuation remembered the value of i when it was created
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 2}, {"int", 4}};
    VerifyStack(expected);
}

// Test showing continuation state with nested loops
TEST_F(ContinuationControlTests, ContinuationStateInNestedRhoLoops) {
    // This test shows that continuations capture state from nested loops

    console_.SetLanguage(Language::Rho);
    data_->Clear();

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

    console_.Execute(rhoCode);

    // Verify stack: Should contain [0, 11, 22]
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 0}, {"int", 11}, {"int", 22}};
    VerifyStack(expected);
}

// Test showing continuation state with mutable variables
TEST_F(ContinuationControlTests, ContinuationStateWithMutableVarsInLoop) {
    // This test demonstrates that continuations capture references to
    // variables, not just their values at creation time

    console_.SetLanguage(Language::Rho);
    data_->Clear();

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

    console_.Execute(rhoCode);

    // Verify stack: Should contain [30, 31, 32]
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 30}, {"int", 31}, {"int", 32}};
    VerifyStack(expected);
}

// Test showing continuation state preservation with loop break/resume
TEST_F(ContinuationControlTests, ContinuationStateWithLoopControl) {
    // This test shows continuations created before and after loop control
    // operations (like break) maintain their distinct states

    console_.SetLanguage(Language::Rho);
    data_->Clear();

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

    console_.Execute(rhoCode);

    // Verify stack: Should contain [100, 102, 200, 202]
    std::vector<std::pair<std::string, int>> expected = {
        {"int", 100}, {"int", 102}, {"int", 200}, {"int", 202}};
    VerifyStack(expected);
}

// Note: If building these tests is encountering errors, you may need to update
// the CMakeLists.txt file to include this test file in the build process.