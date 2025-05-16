#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi/Rho tests
class RhoPiTests : public TestLangCommon {
protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
    }
};

// Test directly unwrapping the ContinuationBegin-value-ContinuationEnd pattern
TEST_F(RhoPiTests, ContinuationBeginValueEndPattern) {
    // Create a continuation with the pattern we've observed in Pi execution
    Pointer<Continuation> cont = reg_->New<Continuation>();
    cont->SetCode(reg_->New<Array>());
    cont->GetCode()->Append(reg_->New<Operation>(Operation::ContinuationBegin));
    cont->GetCode()->Append(reg_->New<int>(5)); // The result value
    cont->GetCode()->Append(reg_->New<Operation>(Operation::ContinuationEnd));
    
    // Push it onto the stack
    data_->Push(cont);
    
    // The UnwrapStackValues method should handle this pattern
    UnwrapStackValues();
    
    // Check that we got an unwrapped integer
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Test binary operations with Pi using direct execution
TEST_F(RhoPiTests, PiBinaryOperations) {
    // Test addition: 2 + 3 = 5
    data_->Clear();
    data_->Push(reg_->New<int>(2));
    data_->Push(reg_->New<int>(3));
    exec_->Eval(reg_->New<Operation>(Operation::Plus));
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
    
    // Test subtraction: 10 - 4 = 6
    data_->Clear();
    data_->Push(reg_->New<int>(10));
    data_->Push(reg_->New<int>(4));
    exec_->Eval(reg_->New<Operation>(Operation::Minus));
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 6);
    
    // Test multiplication: 3 * 7 = 21
    data_->Clear();
    data_->Push(reg_->New<int>(3));
    data_->Push(reg_->New<int>(7));
    exec_->Eval(reg_->New<Operation>(Operation::Multiply));
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 21);
    
    // Test division: 15 / 3 = 5
    data_->Clear();
    data_->Push(reg_->New<int>(15));
    data_->Push(reg_->New<int>(3));
    exec_->Eval(reg_->New<Operation>(Operation::Divide));
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Test Pi text execution to ensure we get proper unwrapping
TEST_F(RhoPiTests, PiTextExecution) {
    // Test simple addition
    data_->Clear();
    console_.Execute("2 3 +");
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
    
    // Test complex expression
    data_->Clear();
    console_.Execute("10 2 / 3 4 * +");  // 5 + 12 = 17
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 17);
    
    // Test comparison operations
    data_->Clear();
    console_.Execute("5 3 >");  // 5 > 3 = true
    
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

// Test for Rho directly with our enhanced unwrapping mechanism
TEST_F(RhoPiTests, RhoTextExecution) {
    console_.SetLanguage(Language::Rho);
    
    // Test simple expression
    data_->Clear();
    console_.Execute("2 + 3");
    
    // Unwrap any continuations to get primitive values
    UnwrapStackValues();
    
    // Check the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
    
    // Test more complex expression
    data_->Clear();
    console_.Execute("10 / 2 + 3 * 4");  // 5 + 12 = 17
    
    // Unwrap any continuations to get primitive values
    UnwrapStackValues();
    
    // Check the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 17);
}

// Test manually creating a binary operation continuation and unwrapping it
TEST_F(RhoPiTests, ManualBinaryOpContinuation) {
    // Create a continuation that adds 2 + 3
    Pointer<Continuation> cont = reg_->New<Continuation>();
    cont->SetCode(reg_->New<Array>());
    
    // Binary operation pattern: [val1, val2, op]
    cont->GetCode()->Append(reg_->New<int>(2));
    cont->GetCode()->Append(reg_->New<int>(3));
    cont->GetCode()->Append(reg_->New<Operation>(Operation::Plus));
    
    // Push the continuation
    data_->Push(cont);
    
    // Unwrap it to get the primitive value
    UnwrapStackValues();
    
    // Check the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Test a more complex Rho expression that combines multiple operations
TEST_F(RhoPiTests, ComplexRhoExpression) {
    console_.SetLanguage(Language::Rho);
    
    // A more complex expression with parentheses and precedence
    data_->Clear();
    console_.Execute("(2 + 3) * 4");  // 5 * 4 = 20
    
    // Unwrap any continuations to get primitive values
    UnwrapStackValues();
    
    // Check the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 20);
}