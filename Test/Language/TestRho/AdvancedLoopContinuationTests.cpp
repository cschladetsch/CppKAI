#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test 1: Continuation chain created by nested loops
TEST(AdvancedLoopContinuations, ContinuationChainFromNestedLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create a chain of continuations using nested loops
        chain = { 1 }  // Start with identity
        
        for (i = 0; i < 3; i = i + 1) {
            j = 0
            while (j < 2) {
                old_chain = chain
                chain = { old_chain ' j + i * }
                j = j + 1
            }
        }
        
        // Execute the chained continuation
        chain '
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // ((((1 + 0) * 0) + 1) * 0) + 0) * 1) + 1) * 1) + 0) * 2) + 1) * 2 =
    // complex
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 2: Loop generators creating loop continuations
TEST(AdvancedLoopContinuations, LoopGeneratorsCreatingLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create continuations that themselves contain loops
        generators = []
        
        for (n = 2; n <= 4; n = n + 1) {
            {
                sum = 0
                i = 1
                while (i <= n) {
                    sum = sum + i
                    i = i + 1
                }
                sum
            }
            generators dup size swap store
        }
        
        // Execute all generators and sum results
        total = 0
        i = 0
        do {
            generators i at '
            total = total +
            i = i + 1
        } while (i < generators size)
        
        total  // Should be 3 + 6 + 10 = 19
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 19);
}

// Test 3: Continuation state machine with loops
TEST(AdvancedLoopContinuations, ContinuationStateMachine) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create state machine using continuations
        states = []
        
        // State 0: Increment
        { state 1 + }
        states dup size swap store
        
        // State 1: Double
        { state 2 * }
        states dup size swap store
        
        // State 2: Add 10
        { state 10 + }
        states dup size swap store
        
        // Run state machine
        state = 1
        current_state = 0
        
        for (i = 0; i < 5; i = i + 1) {
            states current_state at '
            state =
            current_state = (current_state + 1) % 3
        }
        
        state  // Should be ((1+1)*2+10+1)*2 = 28
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 28);
}

// Test 4: Mutual recursion with loop continuations
TEST(AdvancedLoopContinuations, MutualRecursionWithLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create mutually recursive continuations using loops
        even_check = {
            n = 
            if (n == 0) {
                true
            } else {
                n - 1
                odd_check '
            }
        }
        
        odd_check = {
            n = 
            if (n == 0) {
                false
            } else {
                n - 1
                even_check '
            }
        }
        
        // Test with loop-generated values
        results = []
        i = 0
        while (i < 6) {
            i
            even_check '
            results dup size swap store
            i = i + 1
        }
        
        // Count true values
        count = 0
        j = 0
        do {
            if (results j at) {
                count = count + 1
            }
            j = j + 1
        } while (j < results size)
        
        count  // Should be 3 (0, 2, 4 are even)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 3);
}

// Test 5: Dynamic continuation dispatch table
TEST(AdvancedLoopContinuations, DynamicContinuationDispatch) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Build dispatch table dynamically
        operations = []
        
        // Use for loop to create operations
        for (op = 0; op < 4; op = op + 1) {
            if (op == 0) {
                { value 2 + }
            } else if (op == 1) {
                { value 3 * }
            } else if (op == 2) {
                { value 5 - }
            } else {
                { value 2 / }
            }
            operations dup size swap store
        }
        
        // Process values through operations
        value = 10
        sequence = [2, 1, 0, 3, 2]  // Operation sequence
        
        i = 0
        while (i < sequence size) {
            op_index = sequence i at
            operations op_index at '
            value =
            i = i + 1
        }
        
        value  // ((10-5)*3+2)/2-5 = 5
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Test 6: Continuation accumulator pattern with all loop types
TEST(AdvancedLoopContinuations, ContinuationAccumulatorAllLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Build complex accumulator using all loop types
        accumulator = { 0 }
        
        // For loop adds constants
        for (i = 1; i <= 3; i = i + 1) {
            old_acc = accumulator
            accumulator = { old_acc ' i + }
        }
        
        // While loop multiplies
        j = 2
        while (j <= 3) {
            old_acc = accumulator
            accumulator = { old_acc ' j * }
            j = j + 1
        }
        
        // Do-while subtracts
        k = 1
        do {
            old_acc = accumulator
            accumulator = { old_acc ' k - }
            k = k + 1
        } while (k <= 2)
        
        accumulator '  // (((0+1+2+3)*2*3)-1-2) = 33
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 33);
}

// Test 7: Continuation pipeline with conditional routing
TEST(AdvancedLoopContinuations, ContinuationPipelineRouting) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create processing pipeline
        processors = []
        
        // Even processor
        { n 2 * }
        processors dup size swap store
        
        // Odd processor  
        { n 3 * 1 + }
        processors dup size swap store
        
        // Process array of numbers
        results = []
        
        for (n = 0; n < 6; n = n + 1) {
            processor_index = n % 2
            n =
            processors processor_index at '
            results dup size swap store
        }
        
        // Sum results
        sum = 0
        i = 0
        do {
            sum = sum + results i at
            i = i + 1
        } while (i < results size)
        
        sum  // 0 + 4 + 4 + 10 + 8 + 16 = 42
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

// Test 8: Lazy evaluation with continuation thunks
TEST(AdvancedLoopContinuations, LazyEvaluationThunks) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create lazy computations
        lazy_values = []
        
        // Generate expensive computations lazily
        for (base = 2; base <= 4; base = base + 1) {
            {
                result = 1
                exp = 0
                while (exp < base) {
                    result = result * base
                    exp = exp + 1
                }
                result
            }
            lazy_values dup size swap store
        }
        
        // Selectively evaluate based on condition
        sum = 0
        i = 0
        needed = [true, false, true]
        
        do {
            if (needed i at) {
                lazy_values i at '
                sum = sum +
            }
            i = i + 1
        } while (i < lazy_values size)
        
        sum  // 2^2 + 4^4 = 4 + 256 = 260
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 260);
}

// Test 9: Continuation-based memoization
TEST(AdvancedLoopContinuations, ContinuationMemoization) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Memoized Fibonacci using continuations
        memo = []
        
        // Initialize base cases
        { 0 } memo dup size swap store  // fib(0) = 0
        { 1 } memo dup size swap store  // fib(1) = 1
        
        // Build memoized values
        n = 2
        while (n <= 8) {
            {
                // Compute fib(n) using memoized values
                memo (n - 1) at '
                memo (n - 2) at ' +
            }
            memo dup size swap store
            n = n + 1
        }
        
        // Get multiple Fibonacci numbers
        sum = 0
        for (i = 3; i <= 6; i = i + 1) {
            memo i at '
            sum = sum +
        }
        
        sum  // fib(3) + fib(4) + fib(5) + fib(6) = 2 + 3 + 5 + 8 = 18
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 18);
}

// Test 10: Continuation composition with loop transformers
TEST(AdvancedLoopContinuations, ContinuationCompositionTransformers) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create transformation continuations
        transformers = []
        
        // Add transformer
        { x x offset + }
        transformers dup size swap store
        
        // Scale transformer
        { x x scale * }
        transformers dup size swap store
        
        // Power transformer
        { x x x * }
        transformers dup size swap store
        
        // Compose transformations
        offset = 5
        scale = 2
        x = 3
        
        // Apply transformations in sequence
        i = 0
        do {
            transformers i at '
            x =
            i = i + 1
        } while (i < transformers size)
        
        x  // ((3 + 5) * 2)^2 = 16^2 = 256
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 256);
}

// Test 11: Event-driven continuation dispatch
TEST(AdvancedLoopContinuations, EventDrivenContinuationDispatch) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Event handlers as continuations
        handlers = []
        
        // Handler for event type 0: increment
        { state state 1 + }
        handlers dup size swap store
        
        // Handler for event type 1: double
        { state state 2 * }
        handlers dup size swap store
        
        // Handler for event type 2: reset
        { state 0 }
        handlers dup size swap store
        
        // Process event stream
        events = [0, 0, 1, 0, 2, 0, 1]
        state = 0
        
        for (i = 0; i < events size; i = i + 1) {
            event_type = events i at
            handlers event_type at '
            state =
        }
        
        state  // ((0+1+1)*2+1 = 5, reset to 0, +1)*2 = 2
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Test 12: Continuation-based coroutines
TEST(AdvancedLoopContinuations, ContinuationCoroutines) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Producer coroutine
        produced = []
        producer = {
            i = 0
            while (i < 5) {
                i i *  // Produce squares
                produced dup size swap store
                i = i + 1
            }
        }
        
        // Consumer coroutine
        consumed = 0
        consumer = {
            j = 0
            do {
                if (j < produced size) {
                    produced j at
                    consumed = consumed +
                }
                j = j + 1
            } while (j < 5)
        }
        
        // Run producer then consumer
        producer '
        consumer '
        
        consumed  // 0 + 1 + 4 + 9 + 16 = 30
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 30);
}

// Test 13: Map-reduce pattern with continuations
TEST(AdvancedLoopContinuations, MapReduceWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Map function: square and add 1
        mapper = { x x x * 1 + }
        
        // Reduce function: sum
        reducer = { acc val acc val + }
        
        // Input data
        data = [1, 2, 3, 4, 5]
        
        // Map phase
        mapped = []
        for (i = 0; i < data size; i = i + 1) {
            x = data i at
            mapper '
            mapped dup size swap store
        }
        
        // Reduce phase
        acc = 0
        j = 0
        while (j < mapped size) {
            val = mapped j at
            reducer '
            acc =
            j = j + 1
        }
        
        acc  // (1^2+1) + (2^2+1) + (3^2+1) + (4^2+1) + (5^2+1) = 2+5+10+17+26 = 60
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 60);
}

// Test 14: Continuation-based stream processing
TEST(AdvancedLoopContinuations, StreamProcessingContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Stream filters and transformers
        filters = []
        
        // Filter: keep even
        { n n 2 % 0 == }
        filters dup size swap store
        
        // Filter: greater than 5
        { n n 5 > }
        filters dup size swap store
        
        // Process stream with filters
        stream = [2, 7, 4, 9, 6, 3, 8, 5, 10]
        results = []
        
        for (i = 0; i < stream size; i = i + 1) {
            n = stream i at
            pass = true
            
            // Apply all filters
            j = 0
            do {
                filters j at '
                pass = pass &&
                j = j + 1
            } while (j < filters size && pass)
            
            if (pass) {
                n results dup size swap store
            }
        }
        
        // Sum filtered results
        sum = 0
        k = 0
        while (k < results size) {
            sum = sum + results k at
            k = k + 1
        }
        
        sum  // 6 + 8 + 10 = 24 (even AND > 5)
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 24);
}

// Test 15: Recursive descent with loop continuations
TEST(AdvancedLoopContinuations, RecursiveDescentWithLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Simulate recursive descent evaluation
        // Expression: 2 * (3 + 4) - 5
        
        // Parser continuations
        eval_expr = {
            op = ops shift
            if (op == "+") {
                left right +
            } else if (op == "-") {
                left right -
            } else if (op == "*") {
                left right *
            } else {
                left  // Just return left for literals
            }
        }
        
        // Build expression tree
        ops = ["*", "+", "-"]
        values = [2, 3, 4, 5]
        
        // Evaluate using loop
        stack = []
        v_idx = 0
        
        for (i = 0; i < ops size; i = i + 1) {
            if (i == 1) {
                // Handle nested expression
                left = values v_idx at
                v_idx = v_idx + 1
                right = values v_idx at
                v_idx = v_idx + 1
                eval_expr '
                stack dup size swap store
            } else {
                values v_idx at
                stack dup size swap store
                v_idx = v_idx + 1
            }
        }
        
        // Final evaluation
        right = values v_idx at
        left = stack 1 at
        ops = ["*"]
        eval_expr '
        
        left = 
        right = stack 0 at
        ops = ["-"]
        eval_expr '
        
        // Result: 2 * (3 + 4) - 5 = 2 * 7 - 5 = 14 - 5 = 9
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 9);
}

// Test 16: Continuation-based finite state machine
TEST(AdvancedLoopContinuations, ContinuationFiniteStateMachine) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Binary counter state machine
        states = []
        
        // State 0: count 0s
        { 
            char =
            if (char == 0) {
                count0 count0 1 +
                0  // Stay in state 0
            } else {
                count1 count1 1 +
                1  // Go to state 1
            }
        }
        states dup size swap store
        
        // State 1: count 1s
        {
            char =
            if (char == 1) {
                count1 count1 1 +
                1  // Stay in state 1
            } else {
                count0 count0 1 +
                0  // Go to state 0
            }
        }
        states dup size swap store
        
        // Process binary string
        input = [0, 1, 1, 0, 0, 1, 0, 1, 1, 1]
        count0 = 0
        count1 = 0
        state = 0
        
        i = 0
        do {
            char = input i at
            states state at '
            state =
            i = i + 1
        } while (i < input size)
        
        count0 + count1  // Should count all bits = 10
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test 17: Continuation threading through nested loops
TEST(AdvancedLoopContinuations, ContinuationThreading) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Thread value through continuation transformations
        thread_value = {
            value =
            
            // First transformation loop
            i = 0
            while (i < 3) {
                value = value + (i + 1)
                i = i + 1
            }
            
            // Second transformation loop
            for (j = 1; j <= 2; j = j + 1) {
                value = value * j
            }
            
            // Third transformation loop
            k = 0
            do {
                value = value - k
                k = k + 1
            } while (k < 2)
            
            value
        }
        
        // Thread multiple values
        results = []
        for (start = 1; start <= 3; start = start + 1) {
            value = start
            thread_value '
            results dup size swap store
        }
        
        // Sum all results
        sum = 0
        m = 0
        while (m < results size) {
            sum = sum + results m at
            m = m + 1
        }
        
        sum  // Complex calculation
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}

// Test 18: Continuation-based loop unrolling
TEST(AdvancedLoopContinuations, ContinuationLoopUnrolling) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create unrolled loop continuations
        unrolled = []
        
        // Unroll factor of 4
        for (i = 0; i < 4; i = i + 1) {
            {
                sum sum data_ptr at +
                data_ptr data_ptr 1 +
            }
            unrolled dup size swap store
        }
        
        // Process data with unrolled loop
        data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
        sum = 0
        data_ptr = 0
        
        // Main unrolled loop
        iterations = data size / 4
        j = 0
        while (j < iterations) {
            // Execute all 4 unrolled operations
            k = 0
            do {
                unrolled k at '
                sum =
                data_ptr =
                k = k + 1
            } while (k < 4)
            j = j + 1
        }
        
        sum  // 1+2+3+4+5+6+7+8+9+10+11+12 = 78
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 78);
}

// Test 19: Continuation-based parallel reduction pattern
TEST(AdvancedLoopContinuations, ParallelReductionPattern) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Simulate parallel reduction with continuations
        // Each continuation processes a chunk
        
        chunk_processors = []
        chunk_size = 3
        
        // Create chunk processors
        for (chunk = 0; chunk < 3; chunk = chunk + 1) {
            {
                start = chunk * chunk_size
                end = start + chunk_size
                chunk_sum = 0
                
                i = start
                while (i < end && i < data size) {
                    chunk_sum = chunk_sum + data i at
                    i = i + 1
                }
                
                chunk_sum
            }
            chunk_processors dup size swap store
        }
        
        // Data to process
        data = [1, 2, 3, 4, 5, 6, 7, 8, 9]
        
        // Execute all chunk processors and combine results
        total = 0
        j = 0
        do {
            chunk_processors j at '
            total = total +
            j = j + 1
        } while (j < chunk_processors size)
        
        total  // 1+2+3+4+5+6+7+8+9 = 45
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 45);
}

// Test 20: Complex continuation composition with all loop types
TEST(AdvancedLoopContinuations, ComplexContinuationComposition) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Build a complex computation pipeline using continuations
        
        // Stage 1: Data generators
        generators = []
        for (g = 0; g < 3; g = g + 1) {
            {
                base = (g + 1) * 2
                i = 0
                result = []
                while (i < 3) {
                    base + i
                    result dup size swap store
                    i = i + 1
                }
                result
            }
            generators dup size swap store
        }
        
        // Stage 2: Data transformers
        transformers = []
        t = 0
        do {
            if (t == 0) {
                { arr 
                    sum = 0
                    for (i = 0; i < arr size; i = i + 1) {
                        sum = sum + arr i at
                    }
                    sum
                }
            } else {
                { arr
                    product = 1
                    j = 0
                    while (j < arr size) {
                        product = product * arr j at
                        j = j + 1
                    }
                    product
                }
            }
            transformers dup size swap store
            t = t + 1
        } while (t < 2)
        
        // Stage 3: Combine results
        results = []
        
        // Process each generator with each transformer
        for (g_idx = 0; g_idx < generators size; g_idx = g_idx + 1) {
            for (t_idx = 0; t_idx < transformers size; t_idx = t_idx + 1) {
                generators g_idx at '
                arr =
                transformers t_idx at '
                results dup size swap store
            }
        }
        
        // Final reduction
        final = 0
        k = 0
        while (k < results size) {
            final = final + results k at
            k = k + 1
        }
        
        final  // Complex result from multi-stage pipeline
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_GT(ConstDeref<int>(stack->Top()), 0);
}