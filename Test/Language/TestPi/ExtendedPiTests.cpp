#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Executor/Continuation.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * EXTENDED PI LANGUAGE TESTS
 * -------------------------
 * These tests extend the Pi language test suite with 20 additional test cases.
 * They cover various aspects of the Pi language including arithmetic operations, 
 * stack manipulation, numeric conversions, comparison operations, logical operations,
 * and more complex stack operations.
 */

struct ExtendedPiTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);

        // Ensure we have the basic types registered
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
        reg_->AddClass<Array>(Label("Array"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to execute Pi code and check result
    template <typename T>
    void ExecuteAndVerify(const std::string& code, const T& expected) {
        std::cout << "Executing Pi code: " << code << std::endl;
        
        // Execute the code
        console_.Execute(code, Structure::Expression);
        
        // Basic stack verification
        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after operation";
        
        // Get and verify result
        Object result = data_->Top();
        ASSERT_TRUE(result.Exists()) << "Result should exist";
        ASSERT_TRUE(result.IsType<T>()) 
            << "Expected result type " << typeid(T).name() 
            << " but got " << (result.Exists() ? result.GetClass()->GetName().ToString() : "null");
        
        T value = ConstDeref<T>(result);
        ASSERT_EQ(value, expected) 
            << "Expected value " << expected << " but got " << value;
    }
};

// 1. Basic integer addition
TEST_F(ExtendedPiTests, IntegerAddition) {
    ExecuteAndVerify<int>("5 7 +", 12);
}

// 2. Basic integer subtraction
TEST_F(ExtendedPiTests, IntegerSubtraction) {
    ExecuteAndVerify<int>("10 3 -", 7);
}

// 3. Basic integer multiplication
TEST_F(ExtendedPiTests, IntegerMultiplication) {
    ExecuteAndVerify<int>("6 7 *", 42);
}

// 4. Basic integer division
TEST_F(ExtendedPiTests, IntegerDivision) {
    ExecuteAndVerify<int>("20 4 /", 5);
}

// 5. Basic modulo operation
TEST_F(ExtendedPiTests, ModuloOperation) {
    ExecuteAndVerify<int>("17 5 Mod", 2);
}

// 6. Stack duplication
TEST_F(ExtendedPiTests, StackDuplication) {
    console_.Execute("5 Dup", Structure::Expression);
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after Dup";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 5);
}

// 7. Stack drop
TEST_F(ExtendedPiTests, StackDrop) {
    console_.Execute("5 10 Drop", Structure::Expression);
    ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after Drop";
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// 8. Stack swap
TEST_F(ExtendedPiTests, StackSwap) {
    console_.Execute("5 10 Swap", Structure::Expression);
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after Swap";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 5);
}

// 9. Stack over
TEST_F(ExtendedPiTests, StackOver) {
    console_.Execute("5 10 Over", Structure::Expression);
    ASSERT_EQ(data_->Size(), 3) << "Stack should have 3 items after Over";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 10);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 5);
}

// 10. Stack rotation
TEST_F(ExtendedPiTests, StackRotation) {
    console_.Execute("1 2 3 Rot", Structure::Expression);
    ASSERT_EQ(data_->Size(), 3) << "Stack should have 3 items after Rot";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 1);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 3);
}

// 11. Equality comparison
TEST_F(ExtendedPiTests, EqualityComparison) {
    ExecuteAndVerify<bool>("5 5 ==", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("5 6 ==", false);
}

// 12. Inequality comparison
TEST_F(ExtendedPiTests, InequalityComparison) {
    ExecuteAndVerify<bool>("5 6 !=", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("5 5 !=", false);
}

// 13. Less than comparison
TEST_F(ExtendedPiTests, LessThanComparison) {
    ExecuteAndVerify<bool>("5 10 <", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("10 5 <", false);
}

// 14. Greater than comparison
TEST_F(ExtendedPiTests, GreaterThanComparison) {
    ExecuteAndVerify<bool>("10 5 >", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("5 10 >", false);
}

// 15. Less than or equal comparison
TEST_F(ExtendedPiTests, LessThanOrEqualComparison) {
    ExecuteAndVerify<bool>("5 5 <=", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("5 10 <=", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("10 5 <=", false);
}

// 16. Greater than or equal comparison
TEST_F(ExtendedPiTests, GreaterThanOrEqualComparison) {
    ExecuteAndVerify<bool>("5 5 >=", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("10 5 >=", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("5 10 >=", false);
}

// 17. Complex stack manipulation
TEST_F(ExtendedPiTests, ComplexStackManipulation) {
    console_.Execute("1 2 3 4 Drop Swap Over", Structure::Expression);
    ASSERT_EQ(data_->Size(), 4) << "Stack should have 4 items after complex manipulation";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 1);
}

// 18. Boolean NOT operation
TEST_F(ExtendedPiTests, BooleanNotOperation) {
    ExecuteAndVerify<bool>("true Not", false);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("false Not", true);
}

// 19. Boolean AND operation
TEST_F(ExtendedPiTests, BooleanAndOperation) {
    ExecuteAndVerify<bool>("true true And", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("true false And", false);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("false false And", false);
}

// 20. Boolean OR operation
TEST_F(ExtendedPiTests, BooleanOrOperation) {
    ExecuteAndVerify<bool>("true true Or", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("true false Or", true);
    exec_->ClearStacks();
    ExecuteAndVerify<bool>("false false Or", false);
}