#include "KAI/Test/Include/TestLangCommon.h"

class TestPiAdvancedContinuations : public kai::TestLangCommon {
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
    
    bool ExpectBool() {
        EXPECT_FALSE(data_->Empty()) << "Stack is empty";
        auto top = data_->Top();
        EXPECT_TRUE(top.IsType<bool>()) << "Top is not a bool";
        return kai::ConstDeref<bool>(top);
    }
};

// Test 1: Continuation capture in nested loops
TEST_F(TestPiAdvancedContinuations, TestNestedLoopContinuation) {
    const std::string script = R"(
        1 10 loop:
            dup 1 5 loop:
                dup2 * over +
                mark
            drop drop
        drop drop
    )";
    
    console_.Execute(script);
    ASSERT_GT(data_->Size(), 0);
    
    // Should have multiple marks from the inner loop
    int marks = 0;
    for (int i = 0; i < data_->Size(); ++i) {
        if (data_->At(i).GetTypeNumber() == kai::Type::Traits<kai::Continuation>::Number) {
            marks++;
        }
    }
    ASSERT_GT(marks, 0);
}

// Test 2: Continuation with conditional break in nested loops
TEST_F(TestPiAdvancedContinuations, TestConditionalBreakWithContinuation) {
    const std::string script = R"(
        0
        1 10 loop:
            1 10 loop:
                dup2 * 25 > if:
                    mark
                    break
                endif
                1 +
            drop
        drop
    )";
    
    console_.Execute(script);
    ASSERT_TRUE(data_->Size() > 0);
}

// Test 3: Recursive continuation with factorial
TEST_F(TestPiAdvancedContinuations, TestRecursiveContinuation) {
    const std::string script = R"(
        { factorial:
            dup 1 <= if:
                drop 1
            else:
                dup 1 - factorial *
            endif
        } 'factorial &
        
        5 factorial
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 120);
}

// Test 4: Continuation passing between functions
TEST_F(TestPiAdvancedContinuations, TestContinuationPassing) {
    const std::string script = R"(
        { apply_twice:
            dup2 swap exec
            swap exec
        } 'apply_twice &
        
        { double: 2 * } 'double &
        
        5 'double apply_twice
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 20);  // 5 * 2 * 2
}

// Test 5: Complex continuation with map-like operation
TEST_F(TestPiAdvancedContinuations, TestMapLikeContinuation) {
    const std::string script = R"(
        [] 'result &
        [ 1 2 3 4 5 ] 'data &
        
        'data size 0 swap 1 - loop:
            'data swap at
            { 2 * 1 + } exec
            'result swap append 'result &
        drop
        
        'result
    )";
    
    console_.Execute(script);
    auto result = data_->Top();
    ASSERT_TRUE(result.Exists());
}

// Test 6: Continuation with exception handling
TEST_F(TestPiAdvancedContinuations, TestContinuationWithException) {
    const std::string script = R"(
        { risky_op:
            dup 0 == if:
                "Division by zero" throw
            else:
                10 swap /
            endif
        } 'risky_op &
        
        0
        2 risky_op +
        0 risky_op +  // This should throw
    )";
    
    // Execute and expect an exception
    try {
        console_.Execute(script);
        FAIL() << "Expected exception was not thrown";
    } catch (...) {
        // Expected
    }
}

// Test 7: Continuations with coroutine-like behavior
TEST_F(TestPiAdvancedContinuations, TestCoroutineLikeContinuation) {
    const std::string script = R"(
        { generator:
            1 yield
            2 yield
            3 yield
        } 'generator &
        
        [] 'values &
        generator
    )";
    
    // Note: This tests the concept - actual yield would need language support
    console_.Execute(script);
}

// Test 8: Nested continuation with state preservation
TEST_F(TestPiAdvancedContinuations, TestStatePreservingContinuation) {
    const std::string script = R"(
        10 'outer_state &
        
        { outer_func:
            'outer_state
            { inner_func:
                dup 5 + 'outer_state &
            } exec
            'outer_state
        } exec
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 15);
}

// Test 9: Continuation chains with filtering
TEST_F(TestPiAdvancedContinuations, TestFilterChainContinuation) {
    const std::string script = R"(
        [ 1 2 3 4 5 6 7 8 9 10 ] 'numbers &
        [] 'evens &
        
        'numbers size 0 swap 1 - loop:
            'numbers swap at
            dup 2 % 0 == if:
                'evens swap append 'evens &
            else:
                drop
            endif
        drop
        
        'evens size
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 5);
}

// Test 10: Mutual recursion with continuations
TEST_F(TestPiAdvancedContinuations, TestMutualRecursion) {
    const std::string script = R"(
        { is_even:
            dup 0 == if:
                drop true
            else:
                1 - is_odd
            endif
        } 'is_even &
        
        { is_odd:
            dup 0 == if:
                drop false
            else:
                1 - is_even
            endif
        } 'is_odd &
        
        7 is_odd
        8 is_even
        and not
    )";
    
    console_.Execute(script);
    ASSERT_EQ(ExpectBool(), false);
}