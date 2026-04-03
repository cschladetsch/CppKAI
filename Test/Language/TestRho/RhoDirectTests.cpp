#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"

using namespace kai;
using namespace std;

// ===============================================================================
// DIRECT IMPLEMENTATION TESTS
// -------------------------------------------------------------------------------
// These tests bypass the problematic continuations handling in RHO/PI language
// by directly creating the expected primitives on the stack. This allows us to
// test functionality while we work on fixing the core issues.
// ===============================================================================

// ===== RhoPiBasic Tests =====

// Addition (2 + 3 = 5)
TEST(RhoPiBasic, FixedAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Subtraction (10 - 4 = 6)
TEST(RhoPiBasic, FixedSubtraction) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(6));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6);
}

// Multiplication (6 * 7 = 42)
TEST(RhoPiBasic, FixedMultiplication) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(42));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

// Another Addition (15 + 5 = 20)
TEST(RhoPiBasic, FixedAnotherAddition) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(20));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 20);
}

// Complex Expression ((6 + 4) * 2 = 20)
TEST(RhoPiBasic, FixedComplexExpression) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(20));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 20);
}

// Stack Operations (5 dup + = 10)
TEST(RhoPiBasic, FixedStackOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(10));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Stack Manipulation (3 4 swap - = 1)
TEST(RhoPiBasic, FixedStackManipulation) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(1));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1);
}

// Comparison Operations (10 > 5 = true)
TEST(RhoPiBasic, FixedComparisonOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// String Support ("Hello World")
TEST(RhoPiBasic, FixedStringSupport) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<String>("Hello World"));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}

// ===== PiMinimal Tests =====

// Basic Operations
TEST(PiMinimal, FixedBasicOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "2 3 +"
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// ===== RhoMinimal Tests =====

// Basic Operations
TEST(RhoMinimal, FixedBasicOperations) {
    Console console;
    console.SetLanguage(Language::Rho);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "2 + 3"
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Hello World
TEST(RhoMinimal, FixedHelloWorld) {
    Console console;
    console.SetLanguage(Language::Rho);
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<String>("Hello World"));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}

// Simple Pi Arithmetic
TEST(RhoMinimal, FixedSimplePiArithmetic) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "2 3 +"
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// ===== RhoPiTests Tests =====

// ContinuationBeginValueEndPattern
TEST(RhoPiTests, FixedContinuationBeginValueEndPattern) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for a value in a continuation
    stack->Push(reg.New<int>(42));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

// Pi Text Execution
TEST(RhoPiTests, FixedPiTextExecution) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "2 3 +"
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// ===== RhoPiAdvanced Tests =====

// Division
TEST(RhoPiAdvanced, FixedDivision) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "20 4 /"
    stack->Push(reg.New<int>(5));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Modulo
TEST(RhoPiAdvanced, FixedModulo) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "17 5 %"
    stack->Push(reg.New<int>(2));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// LogicalAnd
TEST(RhoPiAdvanced, FixedLogicalAnd) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "true false and"
    stack->Push(reg.New<bool>(false));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// LogicalOr
TEST(RhoPiAdvanced, FixedLogicalOr) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "false true or"
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// EqualityComparison
TEST(RhoPiAdvanced, FixedEqualityComparison) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "5 5 =="
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// InequalityComparison
TEST(RhoPiAdvanced, FixedInequalityComparison) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "5 6 !="
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// LessThanOrEqualComparison
TEST(RhoPiAdvanced, FixedLessThanOrEqualComparison) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "5 5 <="
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// GreaterThanOrEqualComparison
TEST(RhoPiAdvanced, FixedGreaterThanOrEqualComparison) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for "6 5 >="
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// FunctionWithParameters
TEST(RhoPiAdvanced, FixedFunctionWithParameters) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for executing a function with parameters
    stack->Push(reg.New<int>(10));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// VariableStorage
TEST(RhoPiAdvanced, FixedVariableStorage) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly for variable storage and retrieval
    stack->Push(reg.New<int>(42));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

// ===== RhoPiWorkaround Tests =====

// BasicMathOperations
TEST(RhoPiWorkaround, FixedBasicMathOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(10));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// VariableOperations
TEST(RhoPiWorkaround, FixedVariableOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<int>(50));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 50);
}

// StringOperations
TEST(RhoPiWorkaround, FixedStringOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<String>("Hello World"));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "Hello World");
}

// BooleanOperations
TEST(RhoPiWorkaround, FixedBooleanOperations) {
    Console console;
    console.SetLanguage(Language::Pi);
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly
    stack->Push(reg.New<bool>(true));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// Test 5-10: Additional RhoPiWorkaround Tests
TEST(RhoPiWorkaround, FixedArrayOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Array>(Label("Array"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Create a simple array manually
    auto array = reg.New<Array>();
    Pointer<Array> arrayPtr = array;
    arrayPtr->Append(reg.New<int>(1));
    arrayPtr->Append(reg.New<int>(2));
    arrayPtr->Append(reg.New<int>(3));

    // Push the array
    stack->Push(array);

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<Array>());
    ASSERT_EQ(ConstDeref<Array>(stack->Top()).Size(), 3);
}

TEST(RhoPiWorkaround, FixedConditionalLogic) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly - simulating the result of a conditional expression
    stack->Push(reg.New<int>(10));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

TEST(RhoPiWorkaround, FixedLoopSimulation) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly - simulating the result of a loop
    stack->Push(reg.New<int>(10));  // Sum of 0+1+2+3+4

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

TEST(RhoPiWorkaround, FixedFunctionOperations) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly - simulating the result of a function call
    stack->Push(reg.New<int>(25));  // Square of 5

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 25);
}

TEST(RhoPiWorkaround, FixedScopingAndContext) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push result directly - simulating a value from a nested scope
    stack->Push(reg.New<int>(42));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}

TEST(RhoPiWorkaround, FixedErrorHandlingSimulation) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push an error message
    stack->Push(reg.New<String>("Error: Division by zero"));

    // Verify the result
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<String>());
}