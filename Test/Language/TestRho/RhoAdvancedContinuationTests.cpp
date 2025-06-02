#include "KAI/Test/Include/TestLangCommon.h"

class TestRhoAdvancedContinuations : public kai::TestLangCommon {
   protected:
    void SetUp() override { TestLangCommon::SetUp(); }

    void ExecScriptReturning(const char *script) { console_.Execute(script); }
};

// Test 11: Nested loop continuations with early return
TEST_F(TestRhoAdvancedContinuations, TestNestedLoopEarlyReturn) {
    const char *script = R"(
        fun find_pair(target) {
            do i = 1 to 10 {
                do j = 1 to 10 {
                    if (i * j == target) {
                        return [i, j]
                    }
                }
            }
            return []
        }
        
        find_pair(42)
    )";

    console_.Execute(script);
    auto result = data_->Top();
    ASSERT_TRUE(result.Exists());
}

// Test 12: Continuation with lambda composition
TEST_F(TestRhoAdvancedContinuations, TestLambdaComposition) {
    const char *script = R"(
        compose = fun(f, g) {
            return fun(x) { f(g(x)) }
        }
        
        add5 = fun(x) { x + 5 }
        double = fun(x) { x * 2 }
        triple = fun(x) { x * 3 }
        
        pipeline = compose(compose(add5, double), triple)
        pipeline(4)
    )";

    console_.Execute(script);
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 29);  // ((4 * 3) * 2) + 5
}

// Test 13: Advanced pattern matching with continuations
TEST_F(TestRhoAdvancedContinuations, TestPatternMatchingContinuation) {
    const char *script = R"(
        fun process_value(val) {
            match val {
                case n when n < 0 => {
                    return fun() { "negative: " + n }
                }
                case 0 => {
                    return fun() { "zero" }
                }
                case n when n > 0 && n < 10 => {
                    return fun() { "single digit: " + n }
                }
                case n => {
                    return fun() { "large: " + n }
                }
            }
        }
        
        handler = process_value(5)
        handler()
    )";

    console_.Execute(script);
}

// Test 14: Continuation-based state machine
TEST_F(TestRhoAdvancedContinuations, TestStateMachineContinuation) {
    const char *script = R"(
        fun create_state_machine() {
            state = "idle"
            
            transitions = {
                idle: fun(event) {
                    if (event == "start") {
                        state = "running"
                        return "started"
                    }
                    return "invalid"
                },
                running: fun(event) {
                    if (event == "pause") {
                        state = "paused"
                        return "paused"
                    } else if (event == "stop") {
                        state = "idle"
                        return "stopped"
                    }
                    return "invalid"
                },
                paused: fun(event) {
                    if (event == "resume") {
                        state = "running"
                        return "resumed"
                    } else if (event == "stop") {
                        state = "idle"
                        return "stopped"
                    }
                    return "invalid"
                }
            }
            
            return fun(event) {
                handler = transitions[state]
                if (handler) {
                    return handler(event)
                }
                return "unknown state"
            }
        }
        
        machine = create_state_machine()
        machine("start")
        machine("pause")
        machine("resume")
    )";

    console_.Execute(script);
}

// Test 15: Complex continuation with nested closures
TEST_F(TestRhoAdvancedContinuations, TestNestedClosureContinuation) {
    const char *script = R"(
        fun create_accumulator(initial) {
            sum = initial
            
            return {
                add: fun(x) {
                    sum = sum + x
                    return sum
                },
                multiply: fun(x) {
                    sum = sum * x
                    return sum
                },
                get: fun() { sum },
                reset: fun() { sum = initial }
            }
        }
        
        acc = create_accumulator(10)
        acc.add(5)
        acc.multiply(2)
        acc.get()
    )";

    console_.Execute(script);
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 30);  // (10 + 5) * 2
}

// Test 16: Continuation with exception propagation
TEST_F(TestRhoAdvancedContinuations, TestExceptionPropagation) {
    const char *script = R"(
        fun safe_divide(a, b) {
            if (b == 0) {
                throw "Division by zero"
            }
            return a / b
        }
        
        fun calculate(x, y, z) {
            try {
                result1 = safe_divide(x, y)
                result2 = safe_divide(result1, z)
                return result2
            } catch (e) {
                return "Error: " + e
            }
        }
        
        calculate(100, 5, 0)
    )";

    console_.Execute(script);
}

// Test 17: Generator pattern with continuations
TEST_F(TestRhoAdvancedContinuations, TestGeneratorPattern) {
    const char *script = R"(
        fun fibonacci_generator(n) {
            a = 0
            b = 1
            count = 0
            
            return fun() {
                if (count >= n) {
                    return null
                }
                
                if (count == 0) {
                    count = count + 1
                    return a
                } else if (count == 1) {
                    count = count + 1
                    return b
                } else {
                    temp = a + b
                    a = b
                    b = temp
                    count = count + 1
                    return temp
                }
            }
        }
        
        fib = fibonacci_generator(7)
        results = []
        
        do {
            val = fib()
            if (val != null) {
                results = results + [val]
            }
        } while (val != null)
        
        results
    )";

    console_.Execute(script);
}

// Test 18: Memoization with continuations
TEST_F(TestRhoAdvancedContinuations, TestMemoizationContinuation) {
    const char *script = R"(
        fun memoize(f) {
            cache = {}
            
            return fun(x) {
                if (cache[x] != null) {
                    return cache[x]
                }
                
                result = f(x)
                cache[x] = result
                return result
            }
        }
        
        fun expensive_calc(n) {
            if (n <= 1) {
                return n
            }
            return expensive_calc(n - 1) + expensive_calc(n - 2)
        }
        
        fast_calc = memoize(expensive_calc)
        fast_calc(10)
    )";

    console_.Execute(script);
}

// Test 19: Async-like behavior with continuations
TEST_F(TestRhoAdvancedContinuations, TestAsyncLikeContinuation) {
    const char *script = R"(
        fun async_operation(value, callback) {
            // Simulate async by wrapping in continuation
            return fun() {
                result = value * 2
                callback(result)
            }
        }
        
        fun chain_async(initial) {
            results = []
            
            op1 = async_operation(initial, fun(r1) {
                results = results + [r1]
                
                op2 = async_operation(r1, fun(r2) {
                    results = results + [r2]
                    
                    op3 = async_operation(r2, fun(r3) {
                        results = results + [r3]
                    })
                    op3()
                })
                op2()
            })
            op1()
            
            return results
        }
        
        chain_async(5)
    )";

    console_.Execute(script);
}

// Test 20: Complex control flow with Pi blocks
TEST_F(TestRhoAdvancedContinuations, TestComplexPiIntegration) {
    const char *script = R"(
        fun matrix_operation(matrix) {
            result = []
            
            do i = 0 to #matrix - 1 {
                row = matrix[i]
                new_row = {|
                    0
                    '@ size 0 swap 1 - loop:
                        '@ swap at
                        dup 2 % 0 == if:
                            2 *
                        else:
                            3 * 1 +
                        endif
                        +
                    drop
                |} (row)
                
                result = result + [new_row]
            }
            
            return result
        }
        
        matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
        matrix_operation(matrix)
    )";

    console_.Execute(script);
}