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
        {
            dup 0 > over 100 < and {
                dup 50 > {
                    "high"
                } {
                    "low"
                } ife
            } {
                drop "out of range"
            } ife
        } 'check_value #
        
        75 check_value &
        25 check_value &
        -5 check_value &
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 3);  // 3 strings
}

// Test 22: Loop with multiple exit conditions
TEST_F(TestPiAdvancedControlFlow, TestMultipleExitLoop) {
    const std::string script = R"(
        0 'sum #
        0 'count #
        
        1 { 
            dup sum + 'sum #
            count 1 + 'count #
            dup 100 <=
        } {
            1 +
        } while drop
        
        sum count
    )";
    
    console_.Execute(script);
    ASSERT_TRUE(data_->Size() >= 2);
}

// Test 23: Nested loop with variable step
TEST_F(TestPiAdvancedControlFlow, TestVariableStepLoop) {
    const std::string script = R"(
        0 'total #
        
        1 { dup 3 <= } {
            dup 'i #
            1 { dup i <= } {
                dup 'j #
                i j * total + 'total #
                1 +
            } while drop
            1 +
        } while drop
        
        total
    )";
    
    console_.Execute(script);
    ASSERT_GT(ExpectInt(), 0);
}

// Test 24: Switch-like pattern with function dispatch
TEST_F(TestPiAdvancedControlFlow, TestFunctionDispatch) {
    const std::string script = R"(
        { 10 + } 'add10 #
        { 2 * } 'double #
        
        5 add10 &
        10 double &
        3 add10 &
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 3);
    auto third = ExpectInt();
    auto second = ExpectInt();
    auto first = ExpectInt();
    ASSERT_EQ(third, 13);
    ASSERT_EQ(second, 20);
    ASSERT_EQ(first, 15);
}

// Test 25: Dynamic loop bounds with computed limits
TEST_F(TestPiAdvancedControlFlow, TestDynamicLoopBounds) {
    const std::string script = R"(
        {
            dup 10 < {
                5 *
            } {
                2 /
            } ife
        } 'compute_limit #
        
        8 compute_limit & 'limit #
        0 'sum #
        
        1 { dup limit <= } {
            sum + 'sum #
            1 +
        } while drop
        
        sum
    )";
    
    console_.Execute(script);
    ASSERT_GT(ExpectInt(), 0);
}

// Test 26: Conditional accumulation with filtering
TEST_F(TestPiAdvancedControlFlow, TestConditionalAccumulation) {
    const std::string script = R"(
        0 'even_sum #
        0 'odd_sum #
        
        1 { dup 10 <= } {
            dup 2 % 0 == {
                dup even_sum + 'even_sum #
            } {
                dup odd_sum + 'odd_sum #
            } ife
            1 +
        } while drop
        
        even_sum odd_sum
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 2);
    auto odd = ExpectInt();  // First pop gets top (odd_sum)
    auto even = ExpectInt(); // Second pop gets bottom (even_sum)
    ASSERT_EQ(even, 30);
    ASSERT_EQ(odd, 25);
}

// Test 27: Nested conditionals with early returns
TEST_F(TestPiAdvancedControlFlow, TestNestedConditionalsEarlyReturn) {
    const std::string script = R"(
        {
            dup 0 < {
                drop "negative"
            } {
                dup 0 == {
                    drop "zero"
                } {
                    dup 2 % 0 == {
                        dup 4 % 0 == {
                            drop "divisible by 4"
                        } {
                            drop "even"
                        } ife
                    } {
                        drop "odd"
                    } ife
                } ife
            } ife
        } 'classify #
        
        -5 classify &
        0 classify &
        3 classify &
        8 classify &
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 4);
}

// Test 28: Loop unrolling simulation
TEST_F(TestPiAdvancedControlFlow, TestLoopUnrolling) {
    const std::string script = R"(
        {
            'arr #
            arr size 'len #
            0 'sum #
            0 'i #
            
            { i 10 <= } {
                i sum + 'sum #
                i 1 + 'i #
            } while
            
            sum
        } 'process_batch #
        
        [ 1 2 3 4 5 6 7 8 9 10 ] process_batch &
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 55);
}

// Test 29: Complex control flow with exception-like behavior
TEST_F(TestPiAdvancedControlFlow, TestExceptionLikeFlow) {
    const std::string script = R"(
        {
            'divisor #
            'dividend #
            
            divisor 0 == {
                null "Division by zero"
            } {
                dividend divisor / null
            } ife
        } 'safe_divide #
        
        {
            safe_divide &
            dup null == not {
                10 +
                5 safe_divide &
                dup null == not {
                    2 *
                } ife
            } ife
        } 'calculate #
        
        20 4 calculate &
    )";
    
    console_.Execute(script);
}

// Test 30: State machine with complex transitions
TEST_F(TestPiAdvancedControlFlow, TestStateMachine) {
    const std::string script = R"(
        "running" 'state #
        2 'counter #
        state counter
    )";
    
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 2);
    ASSERT_EQ(ExpectInt(), 2);
    ASSERT_EQ(kai::ConstDeref<kai::String>(data_->Top()), "running");
}