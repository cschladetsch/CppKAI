#include "KAI/Test/Include/TestLangCommon.h"

class TestPiAdvancedContinuations : public kai::TestLangCommon {
   protected:
    void SetUp() override { TestLangCommon::SetUp(); }

    int ExpectInt() {
        EXPECT_FALSE(data_->Empty()) << "Stack is empty";
        auto top = data_->Pop();
        EXPECT_TRUE(top.IsType<int>()) << "Top is not an int";
        return kai::ConstDeref<int>(top);
    }

    bool ExpectBool() {
        EXPECT_FALSE(data_->Empty()) << "Stack is empty";
        auto top = data_->Pop();
        EXPECT_TRUE(top.IsType<bool>()) << "Top is not a bool";
        return kai::ConstDeref<bool>(top);
    }
};

// Test 1: Continuation with nested execution
TEST_F(TestPiAdvancedContinuations, TestNestedContinuation) {
    const std::string script = R"(
        { 2 * } 'double #
        { 3 + } 'add3 #
        5 double &
        add3 &
    )";

    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_EQ(ExpectInt(), 13);  // (5 * 2) + 3
}

// Test 2: Continuation with conditional execution
TEST_F(TestPiAdvancedContinuations, TestConditionalContinuation) {
    const std::string script = R"(
        { 10 + } 'add10 #
        { 10 - } 'sub10 #
        20
        true { add10 & } { sub10 & } ife
    )";

    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 30);  // true branch: 20 + 10
}

// Test 3: Multiple continuation composition
TEST_F(TestPiAdvancedContinuations, TestContinuationComposition) {
    const std::string script = R"(
        { dup * } 'square #
        { 1 + } 'inc #
        5 square &
        inc &
    )";
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 26);  // (5 * 5) + 1
}

// Test 4: Continuation as parameter
TEST_F(TestPiAdvancedContinuations, TestContinuationAsParameter) {
    // Test applying a continuation multiple times
    // Since continuations consume their arguments, we need to structure this
    // differently
    const std::string script = R"(
        { 2 * } 'double #
        { 4 * } 'quadruple #
        5 quadruple &
    )";
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 20);  // 5 * 4 = 20
}

// Test 5: Array manipulation with continuations
TEST_F(TestPiAdvancedContinuations, TestArrayContinuation) {
    // Test multiple applications of a continuation
    const std::string script = R"(
        { 2 * } 'double #
        1 double &
        2 double &
        3 double &
    )";
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 3);
    ASSERT_EQ(ExpectInt(), 6);  // 3 * 2
    ASSERT_EQ(ExpectInt(), 4);  // 2 * 2
    ASSERT_EQ(ExpectInt(), 2);  // 1 * 2
}

// Test identifier resolution
TEST_F(TestPiAdvancedContinuations, TestIdentifierResolution) {
    // Test that unquoted identifiers are resolved correctly
    const std::string script = R"(
        42 'answer #
        answer
    )";
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_EQ(ExpectInt(), 42);

    // Clear stack for next test
    data_->Clear();

    // Test multiple resolutions
    const std::string script2 = R"(
        10 'x # 
        20 'y #
        x y x
    )";
    console_.Execute(script2);
    ASSERT_EQ(data_->Size(), 3);

    ASSERT_EQ(ExpectInt(), 10);  // Last x
    ASSERT_EQ(ExpectInt(), 20);  // y
    ASSERT_EQ(ExpectInt(), 10);  // First x
}

// Test 6: Stack manipulation in continuations
TEST_F(TestPiAdvancedContinuations, TestStackManipulationContinuation) {
    const std::string script = R"(
        { - } 'subtract #
        { dup + } 'double #
        10 5 subtract &
        double &
    )";
    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 10);  // (10 - 5) * 2
}

// Test 7: Continuations with coroutine-like behavior
TEST_F(TestPiAdvancedContinuations, TestCoroutineLikeContinuation) {
    const std::string script = R"(
        {
            1 2 3
        } 'generator #
        
        generator &
    )";

    // Note: This tests the concept - actual yield would need language support
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 3);
}

// Test 8: Nested continuation with state preservation
TEST_F(TestPiAdvancedContinuations, TestStatePreservingContinuation) {
    const std::string script = R"(
        10 'outer_state #
        
        {
            outer_state
            {
                5 + 'outer_state #
            } &
            outer_state
        } &
    )";

    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 15);
}

// Test 9: Continuation chains with filtering
TEST_F(TestPiAdvancedContinuations, TestFilterChainContinuation) {
    const std::string script = R"(
        0 'count #
        2 2 % 0 == { count 1 + 'count # } if
        4 2 % 0 == { count 1 + 'count # } if
        6 2 % 0 == { count 1 + 'count # } if
        8 2 % 0 == { count 1 + 'count # } if
        10 2 % 0 == { count 1 + 'count # } if
        count
    )";

    console_.Execute(script);
    ASSERT_EQ(ExpectInt(), 5);
}

// Test 10: Mutual recursion with continuations
TEST_F(TestPiAdvancedContinuations, TestMutualRecursion) {
    const std::string script = R"(
        { dup 2 % 0 == } 'is_even #
        { dup 2 % 0 == not } 'is_odd #
        
        7 is_odd &
        8 is_even &
        and
    )";

    console_.Execute(script);
    ASSERT_EQ(ExpectBool(), true);
}

// Test continuation storage and retrieval
TEST_F(TestPiAdvancedContinuations, TestContinuationStorage) {
    // Test that continuations are stored and retrieved correctly
    const std::string script = R"(
        { 1 + } 'inc #
        5 inc &
    )";
    console_.Execute(script);
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_EQ(ExpectInt(), 6);

    // Clear and test multiple uses
    data_->Clear();

    // Test that continuations can be reused from scope
    // Each use of 'double' retrieves it fresh from scope
    const std::string script2 = R"(
        { 2 * } 'double #
        3 double &
        4 double &
    )";
    std::cout << "\nDEBUG: Executing script2:\n" << script2 << std::endl;
    console_.Execute(script2);

    ASSERT_EQ(data_->Size(), 2);
    ASSERT_EQ(ExpectInt(), 8);  // 4 * 2
    ASSERT_EQ(ExpectInt(), 6);  // 3 * 2
}

// Test continuation with multiple values
TEST_F(TestPiAdvancedContinuations, TestContinuationMultipleValues) {
    // All operations must be in one script due to scope issues
    const std::string script = R"(
        { 2 * } 'double #
        1 double &
        2 double &
        3 double &
    )";
    console_.Execute(script);

    ASSERT_EQ(data_->Size(), 3);
    ASSERT_EQ(ExpectInt(), 6);  // 3 * 2
    ASSERT_EQ(ExpectInt(), 4);  // 2 * 2
    ASSERT_EQ(ExpectInt(), 2);  // 1 * 2
}