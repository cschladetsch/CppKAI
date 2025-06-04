#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test 1: If-else with continuation selection in loops
TEST(ComplexControlFlow, IfElseContinuationSelection) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create different continuations for odd/even processing
        even_proc = { n n 2 / }
        odd_proc = { n n 3 * 1 + }
        
        results = []
        
        for (i = 0; i < 8; i = i + 1) {
            n = i
            if (i % 2 == 0) {
                even_proc '
            } else {
                odd_proc '
            }
            results dup size swap store
        }
        
        // Sum results
        sum = 0
        j = 0
        while (j < results size) {
            sum = sum + results j at
            j = j + 1
        }
        sum  // 0 + 4 + 1 + 10 + 2 + 16 + 3 + 22 = 58
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 58);
}

// Test 2: Nested if with logical NOT in loops
TEST(ComplexControlFlow, NestedIfWithLogicalNot) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        count = 0
        
        for (i = 0; i < 10; i = i + 1) {
            if (!(i % 3 == 0)) {
                if (!(i % 2 == 0)) {
                    // Not divisible by 3 AND not divisible by 2
                    count = count + 1
                }
            }
        }
        
        count  // 1, 5, 7 = 3 numbers
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);
}

// Test 3: Complex AND/OR conditions with continuations
TEST(ComplexControlFlow, ComplexLogicalConditions) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Continuation that checks complex conditions
        validator = {
            n =
            (n > 5 && n < 15) || (n % 4 == 0 && n < 20)
        }
        
        valid_count = 0
        i = 0
        
        do {
            n = i
            if (validator ') {
                valid_count = valid_count + 1
            }
            i = i + 1
        } while (i <= 20)
        
        valid_count  // Count numbers that match condition
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 4: Short-circuit evaluation with continuations
TEST(ComplexControlFlow, ShortCircuitEvaluation) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Track evaluation count
        eval_count = 0
        
        // Expensive check continuation
        expensive_check = {
            eval_count = eval_count + 1
            n > 10
        }
        
        // Process with short-circuit
        results = []
        
        for (n = 0; n < 15; n = n + 1) {
            // Short-circuit: if n < 5, don't evaluate expensive check
            if (n < 5 || expensive_check ') {
                n results dup size swap store
            }
        }
        
        eval_count  // Should be 10 (only called for n >= 5)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test 5: Conditional continuation chaining
TEST(ComplexControlFlow, ConditionalContinuationChaining) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Build conditional chain
        chain = { x x }  // Identity
        
        i = 0
        while (i < 5) {
            old_chain = chain
            if (i % 2 == 0) {
                chain = { old_chain ' 2 + }
            } else {
                chain = { old_chain ' 3 * }
            }
            i = i + 1
        }
        
        // Execute chain
        5 chain '  // ((((5 + 2) * 3 + 2) * 3) + 2) = 77
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 77);
}

// Test 6: Guard patterns with continuations
TEST(ComplexControlFlow, GuardPatternsWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Guard continuations
        guards = []
        
        // Guard 1: positive numbers
        { n n > 0 }
        guards dup size swap store
        
        // Guard 2: even numbers
        { n n % 2 == 0 }
        guards dup size swap store
        
        // Guard 3: less than 10
        { n n < 10 }
        guards dup size swap store
        
        // Process with all guards
        passed = 0
        
        for (n = -5; n < 15; n = n + 1) {
            all_pass = true
            g = 0
            
            // Check all guards
            do {
                if (!guards g at ') {
                    all_pass = false
                    break
                }
                g = g + 1
            } while (g < guards size)
            
            if (all_pass) {
                passed = passed + 1
            }
        }
        
        passed  // 2, 4, 6, 8 = 4 numbers
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);
}

// Test 7: Conditional break/continue with continuations
TEST(ComplexControlFlow, ConditionalBreakContinue) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Continuation that determines break/continue
        should_skip = { n n % 3 == 0 }
        should_stop = { n n > 15 }
        
        sum = 0
        count = 0
        
        i = 0
        while (true) {
            if (should_stop ') {
                break
            }
            
            if (should_skip ') {
                i = i + 1
            } else {
                sum = sum + i
                count = count + 1
                i = i + 1
            }
        }
        
        sum  // Sum of non-multiples of 3 up to 15
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 8: Ternary-like conditional continuations
TEST(ComplexControlFlow, TernaryConditionalContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Ternary operation using continuations
        true_branch = { a a 2 * }
        false_branch = { a a 3 + }
        
        results = []
        
        for (i = 0; i < 6; i = i + 1) {
            a = i
            // Ternary: i > 3 ? i * 2 : i + 3
            if (i > 3) {
                true_branch '
            } else {
                false_branch '
            }
            results dup size swap store
        }
        
        // Calculate sum
        sum = 0
        j = 0
        do {
            sum = sum + results j at
            j = j + 1
        } while (j < results size)
        
        sum  // 3 + 4 + 5 + 6 + 8 + 10 = 36
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 36);
}

// Test 9: Multi-level if-else with loop state
TEST(ComplexControlFlow, MultiLevelIfElseWithLoopState) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        // Categorize numbers into multiple buckets
        small = 0
        medium = 0
        large = 0
        huge = 0
        
        i = 0
        while (i <= 100) {
            if (i < 10) {
                small = small + 1
            } else if (i < 50) {
                medium = medium + 1
            } else if (i < 90) {
                large = large + 1
            } else {
                huge = huge + 1
            }
            
            i = i + 5  // Step by 5
        }
        
        small + medium * 10 + large * 100 + huge * 1000
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 10: Continuation dispatch with complex conditions
TEST(ComplexControlFlow, ContinuationDispatchComplexConditions) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Dispatch table with complex selection logic
        handlers = []
        
        // Handler for small positive
        { n n + 10 }
        handlers dup size swap store
        
        // Handler for large positive
        { n n * 2 }
        handlers dup size swap store
        
        // Handler for negative
        { n 0 - n }
        handlers dup size swap store
        
        // Handler for zero
        { n 100 }
        handlers dup size swap store
        
        // Process values with dispatch
        values = [-5, 0, 3, 15, -10, 8]
        results = []
        
        for (i = 0; i < values size; i = i + 1) {
            n = values i at
            
            // Complex dispatch logic
            if (n == 0) {
                handler_idx = 3
            } else if (n < 0) {
                handler_idx = 2
            } else if (n < 10) {
                handler_idx = 0
            } else {
                handler_idx = 1
            }
            
            handlers handler_idx at '
            results dup size swap store
        }
        
        // Sum results
        sum = 0
        j = 0
        while (j < results size) {
            sum = sum + results j at
            j = j + 1
        }
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 11: Logical operator precedence with continuations
TEST(ComplexControlFlow, LogicalOperatorPrecedence) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Test AND/OR precedence
        check1 = { n n > 5 }
        check2 = { n n < 15 }
        check3 = { n n % 2 == 0 }
        
        matches = 0
        
        for (n = 0; n < 20; n = n + 1) {
            // (n > 5 && n < 15) || (n % 2 == 0)
            if ((check1 ' && check2 ') || check3 ') {
                matches = matches + 1
            }
        }
        
        matches
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 12: Nested loop control with conditional continuations
TEST(ComplexControlFlow, NestedLoopControlConditional) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Matrix search with early exit
        found_value = -1
        search_target = 42
        
        // Create search predicate
        is_target = { val val == search_target }
        
        // Matrix values
        matrix = [[10, 20, 30], [40, 42, 50], [60, 70, 80]]
        
        i = 0
        outer_loop: while (i < matrix size) {
            row = matrix i at
            j = 0
            
            do {
                val = row j at
                if (is_target ') {
                    found_value = i * 10 + j
                    break outer_loop
                }
                j = j + 1
            } while (j < row size)
            
            i = i + 1
        }
        
        found_value  // Should be 11 (row 1, col 1)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 11);
}

// Test 13: Conditional accumulator with continuations
TEST(ComplexControlFlow, ConditionalAccumulator) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Conditional accumulator continuations
        add_if_even = {
            sum n
            if (n % 2 == 0) {
                sum + n
            } else {
                sum
            }
        }
        
        multiply_if_odd = {
            product n
            if (n % 2 != 0) {
                product * n
            } else {
                product
            }
        }
        
        // Process with both accumulators
        sum_result = 0
        product_result = 1
        
        for (i = 1; i <= 6; i = i + 1) {
            sum_result = add_if_even '
            product_result = multiply_if_odd '
        }
        
        sum_result + product_result  // (2+4+6) + (1*3*5) = 12 + 15 = 27
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 27);
}

// Test 14: Error handling pattern with continuations
TEST(ComplexControlFlow, ErrorHandlingPattern) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Error handling continuations
        validate = { n n >= 0 && n <= 100 }
        process = { n n * n }
        handle_error = { n -1 }
        
        // Process with validation
        inputs = [5, -3, 50, 150, 25, -10, 75]
        results = []
        errors = 0
        
        i = 0
        while (i < inputs size) {
            n = inputs i at
            
            if (validate ') {
                process '
            } else {
                errors = errors + 1
                handle_error '
            }
            results dup size swap store
            
            i = i + 1
        }
        
        // Count valid results
        valid = 0
        j = 0
        do {
            if (results j at >= 0) {
                valid = valid + 1
            }
            j = j + 1
        } while (j < results size)
        
        valid * 1000 + errors  // 4 valid, 3 errors = 4003
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4003);
}

// Test 15: State machine with conditional transitions
TEST(ComplexControlFlow, StateMachineConditionalTransitions) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // State transitions based on input
        state_A = {
            input state
            if (input == 0) {
                1  // Go to B
            } else if (input == 1) {
                0  // Stay in A
            } else {
                2  // Go to C
            }
        }
        
        state_B = {
            input state
            if (input == 0) {
                0  // Go to A
            } else {
                2  // Go to C
            }
        }
        
        state_C = {
            input state
            2  // Always stay in C (terminal)
        }
        
        states = [state_A, state_B, state_C]
        
        // Process input sequence
        inputs = [0, 1, 0, 2, 1, 0]
        state = 0
        state_history = []
        
        for (i = 0; i < inputs size; i = i + 1) {
            state state_history dup size swap store
            input = inputs i at
            states state at '
            state =
        }
        
        // Final state
        state
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);  // Should end in state C
}

// Test 16: Complex filter chain with continuations
TEST(ComplexControlFlow, ComplexFilterChain) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Multiple filter continuations
        filters = []
        
        // Filter 1: Range check
        { n n >= 10 && n <= 50 }
        filters dup size swap store
        
        // Filter 2: Not multiple of 3
        { n !(n % 3 == 0) }
        filters dup size swap store
        
        // Filter 3: Sum of digits < 10
        {
            n =
            sum = 0
            temp = n
            while (temp > 0) {
                sum = sum + (temp % 10)
                temp = temp / 10
            }
            sum < 10
        }
        filters dup size swap store
        
        // Apply all filters
        count = 0
        
        for (n = 0; n <= 60; n = n + 1) {
            passes = true
            f = 0
            
            // Check all filters
            while (f < filters size && passes) {
                if (!filters f at ') {
                    passes = false
                }
                f = f + 1
            }
            
            if (passes) {
                count = count + 1
            }
        }
        
        count
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 17: Recursive continuations with conditionals
TEST(ComplexControlFlow, RecursiveContinuationsWithConditionals) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Recursive GCD with continuations
        gcd = {
            a b
            if (b == 0) {
                a
            } else {
                b (a % b) gcd '
            }
        }
        
        // Test with multiple pairs
        pairs = [[48, 18], [100, 35], [81, 27]]
        results = []
        
        i = 0
        do {
            pair = pairs i at
            a = pair 0 at
            b = pair 1 at
            gcd '
            results dup size swap store
            i = i + 1
        } while (i < pairs size)
        
        // Sum all GCDs
        sum = 0
        j = 0
        while (j < results size) {
            sum = sum + results j at
            j = j + 1
        }
        
        sum  // gcd(48,18) + gcd(100,35) + gcd(81,27) = 6 + 5 + 27 = 38
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 38);
}

// Test 18: Conditional pipeline with early termination
TEST(ComplexControlFlow, ConditionalPipelineEarlyTermination) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Pipeline stages with conditions
        stages = []
        
        // Stage 1: Add 10
        {
            val = 
            val + 10
        }
        stages dup size swap store
        
        // Stage 2: Multiply by 2 if > 20
        {
            val =
            if (val > 20) {
                val * 2
            } else {
                val
            }
        }
        stages dup size swap store
        
        // Stage 3: Subtract 15 if even
        {
            val =
            if (val % 2 == 0) {
                val - 15
            } else {
                val
            }
        }
        stages dup size swap store
        
        // Process values through pipeline
        values = [5, 15, 25, 8]
        results = []
        
        for (v = 0; v < values size; v = v + 1) {
            val = values v at
            s = 0
            
            // Process through stages with early exit
            while (s < stages size) {
                stages s at '
                val =
                
                // Early exit if value becomes negative
                if (val < 0) {
                    val = -999  // Error marker
                    break
                }
                
                s = s + 1
            }
            
            results dup size swap store
        }
        
        // Sum positive results only
        sum = 0
        i = 0
        do {
            if (results i at > 0) {
                sum = sum + results i at
            }
            i = i + 1
        } while (i < results size)
        
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 19: Conditional continuation memoization
TEST(ComplexControlFlow, ConditionalContinuationMemoization) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Memoized computation with conditions
        cache = []
        compute_count = 0
        
        // Expensive computation
        compute = {
            n =
            compute_count = compute_count + 1
            
            if (n < 2) {
                n
            } else {
                // Simulate expensive recursive computation
                result = 0
                i = 0
                while (i <= n) {
                    result = result + i
                    i = i + 1
                }
                result
            }
        }
        
        // Get or compute with memoization
        get_value = {
            n =
            if (n < cache size && cache n at != -1) {
                cache n at
            } else {
                // Ensure cache is large enough
                while (cache size <= n) {
                    cache dup size -1 store
                }
                
                // Compute and cache
                val = compute '
                cache n val store
                val
            }
        }
        
        // Use memoized values
        test_values = [5, 3, 5, 7, 3, 5, 7]
        sum = 0
        
        for (i = 0; i < test_values size; i = i + 1) {
            n = test_values i at
            get_value '
            sum = sum +
        }
        
        compute_count  // Should be 4 (unique values: 5, 3, 7)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);
}

// Test 20: Complex control flow orchestration
TEST(ComplexControlFlow, ComplexControlFlowOrchestration) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Complex orchestration of multiple patterns
        
        // Pattern matchers
        is_prime = {
            n =
            if (n < 2) {
                false
            } else {
                prime = true
                d = 2
                while (d * d <= n && prime) {
                    if (n % d == 0) {
                        prime = false
                    }
                    d = d + 1
                }
                prime
            }
        }
        
        // Transformers based on conditions
        transform = {
            n =
            if (is_prime ') {
                n * 10  // Primes get multiplied by 10
            } else if (n % 2 == 0) {
                n / 2   // Even non-primes get halved
            } else {
                n + 7   // Odd non-primes get +7
            }
        }
        
        // Process range with multiple conditions
        result = 0
        
        for (base = 2; base <= 10; base = base + 1) {
            val = base
            
            // Apply transformation
            transform '
            val =
            
            // Conditional accumulation
            if (val > 15) {
                if (val % 3 == 0) {
                    result = result + val
                } else if (val % 5 == 0) {
                    result = result + (val / 5)
                } else {
                    result = result + 1
                }
            } else {
                result = result - 1
            }
        }
        
        result
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // Complex calculation result
    ASSERT_NE(ConstDeref<int>(stack->Top()), 0);
}