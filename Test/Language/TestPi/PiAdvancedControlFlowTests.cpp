#include "KAI/Test/Include/TestLangCommon.h"

class TestPiAdvancedControlFlow : public kai::TestLangCommon {
protected:
    void SetUp() override {
        TestLangCommon::SetUp();
    }
    
    int ExpectInt() {
        EXPECT_FALSE(data_->Empty()) << "Stack is empty";
        auto top = data_->Top();
        EXPECT_TRUE(top.IsType<int>()) << "Top is not an int";
        return kai::ConstDeref<int>(top);
    }
};

// Test 21: Complex nested if-else with short-circuit evaluation
TEST_F(TestPiAdvancedControlFlow, TestShortCircuitEvaluation) {
    const std::string script = R"(
        { check_value:
            dup 0 > over 100 < and if:
                dup 50 > if:
                    "high" swap
                else:
                    "low" swap
                endif
            else:
                "out of range" swap
            endif
        } 'check_value &
        
        75 check_value
        25 check_value
        -5 check_value
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 6);  // 3 pairs of (string, number)
}

// Test 22: Loop with multiple exit conditions
TEST_F(TestPiAdvancedControlFlow, TestMultipleExitLoop) {
    const std::string script = R"(
        0 'sum &
        0 'count &
        
        1 100 loop:
            dup 'sum +  'sum &
            'count 1 + 'count &
            
            'sum 100 > if:
                "sum exceeded" mark
                break
            endif
            
            'count 15 > if:
                "count exceeded" mark
                break
            endif
        drop
        
        'sum 'count
    )";
    
    console_.Execute(script);
    ASSERT_TRUE(data_->Size() >= 2);
}

// Test 23: Nested loop with variable step
TEST_F(TestPiAdvancedControlFlow, TestVariableStepLoop) {
    const std::string script = R"(
        0 'total &
        
        1 3 loop:
            'i &
            1 'i 1 'i loop:
                'j &
                'i 'j * 'total + 'total &
            drop
        drop
        
        'total
    )";
    
    console_.Execute(script);
    ASSERT_GT(ExpectInt(), 0);
}

// Test 24: Switch-like pattern with function dispatch
TEST_F(TestPiAdvancedControlFlow, TestFunctionDispatch) {
    const std::string script = R"(
        { case1: 10 + } 'case1 &
        { case2: 20 * } 'case2 &
        { case3: 2 / } 'case3 &
        { default: drop 0 } 'default &
        
        { switch:
            'val &
            'val 1 == if: case1 exec else:
            'val 2 == if: case2 exec else:
            'val 3 == if: case3 exec else:
            default exec
            endif endif endif
        } 'switch &
        
        5 1 switch
        5 2 switch
        5 3 switch
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 3);
}

// Test 25: Dynamic loop bounds with computed limits
TEST_F(TestPiAdvancedControlFlow, TestDynamicLoopBounds) {
    const std::string script = R"(
        { compute_limit:
            dup 10 < if:
                5 *
            else:
                2 /
            endif
        } 'compute_limit &
        
        8 compute_limit 'limit &
        0 'sum &
        
        1 'limit loop:
            'sum + 'sum &
        drop
        
        'sum
    )";
    
    console_.Execute(script);
    ASSERT_GT(ExpectInt(), 0);
}

// Test 26: Conditional accumulation with filtering
TEST_F(TestPiAdvancedControlFlow, TestConditionalAccumulation) {
    const std::string script = R"(
        [ 1 2 3 4 5 6 7 8 9 10 ] 'data &
        0 'even_sum &
        0 'odd_sum &
        
        'data size 0 swap 1 - loop:
            'data swap at
            dup 2 % 0 == if:
                'even_sum + 'even_sum &
            else:
                'odd_sum + 'odd_sum &
            endif
        drop
        
        'even_sum 'odd_sum
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 2);
    auto odd = ExpectInt();
    auto even = ExpectInt();
    ASSERT_EQ(even, 30);  // 2+4+6+8+10
    ASSERT_EQ(odd, 25);   // 1+3+5+7+9
}

// Test 27: Nested conditionals with early returns
TEST_F(TestPiAdvancedControlFlow, TestNestedConditionalsEarlyReturn) {
    const std::string script = R"(
        { classify:
            dup 0 < if:
                drop "negative"
                return
            endif
            
            dup 0 == if:
                drop "zero"
                return
            endif
            
            dup 2 % 0 == if:
                dup 4 % 0 == if:
                    "divisible by 4"
                else:
                    "even"
                endif
            else:
                "odd"
            endif
            swap drop
        } 'classify &
        
        -5 classify
        0 classify
        3 classify
        8 classify
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 4);
}

// Test 28: Loop unrolling simulation
TEST_F(TestPiAdvancedControlFlow, TestLoopUnrolling) {
    const std::string script = R"(
        { process_batch:
            'arr &
            'arr size 'len &
            0 'sum &
            0 'i &
            
            { 'i 'len < } {
                'i 3 + 'len <= if:
                    'arr 'i at
                    'arr 'i 1 + at +
                    'arr 'i 2 + at +
                    'arr 'i 3 + at +
                    'sum + 'sum &
                    'i 4 + 'i &
                else:
                    'arr 'i at 'sum + 'sum &
                    'i 1 + 'i &
                endif
            } while
            
            'sum
        } 'process_batch &
        
        [ 1 2 3 4 5 6 7 8 9 10 ] process_batch
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 55);
}

// Test 29: Complex control flow with exception-like behavior
TEST_F(TestPiAdvancedControlFlow, TestExceptionLikeFlow) {
    const std::string script = R"(
        { safe_divide:
            'divisor &
            'dividend &
            
            'divisor 0 == if:
                null "Division by zero"
            else:
                'dividend 'divisor / null
            endif
        } 'safe_divide &
        
        { calculate:
            safe_divide
            dup null == not if:
                10 +
                5 safe_divide
                dup null == not if:
                    2 *
                endif
            endif
        } 'calculate &
        
        20 4 calculate
    )";
    
    console_.Execute(script);
}

// Test 30: State machine with complex transitions
TEST_F(TestPiAdvancedControlFlow, TestStateMachine) {
    const std::string script = R"(
        "start" 'state &
        0 'counter &
        
        { transition:
            'event &
            
            'state "start" == if:
                'event "go" == if:
                    "running" 'state &
                    'counter 1 + 'counter &
                endif
            else:
            'state "running" == if:
                'event "pause" == if:
                    "paused" 'state &
                else:
                'event "stop" == if:
                    "stopped" 'state &
                endif endif
            else:
            'state "paused" == if:
                'event "resume" == if:
                    "running" 'state &
                    'counter 1 + 'counter &
                else:
                'event "stop" == if:
                    "stopped" 'state &
                endif endif
            endif endif endif
        } 'transition &
        
        "go" transition
        "pause" transition
        "resume" transition
        "stop" transition
        
        'state 'counter
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 2);
}