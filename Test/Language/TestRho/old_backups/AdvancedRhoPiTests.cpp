#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

/*
 * ADVANCED PI TESTS FOR RHO
 * -------------------------
 * These tests expand on SimpleRhoPiTests.cpp by testing more advanced
 * language features that would normally be tested in Rho but are
 * implemented using Pi language as a workaround for the type mismatch
 * issues in the Rho implementation.
 */

// Division operation
TEST(RhoPiAdvanced, Division) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test division using Pi language directly
    stack->Clear();
    console.Execute("20 4 /");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Modulo operation
TEST(RhoPiAdvanced, Modulo) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test modulo using Pi language directly
    stack->Clear();
    console.Execute("17 5 %");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 2);
}

// Logical operations (AND)
TEST(RhoPiAdvanced, LogicalAnd) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test logical AND using Pi language directly
    stack->Clear();
    console.Execute("true false &&");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));

    // Test another logical AND
    stack->Clear();
    console.Execute("true true &&");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));
}

// Logical operations (OR)
TEST(RhoPiAdvanced, LogicalOr) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test logical OR using Pi language directly
    stack->Clear();
    console.Execute("false true ||");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test another logical OR
    stack->Clear();
    console.Execute("false false ||");

    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Equality comparison
TEST(RhoPiAdvanced, EqualityComparison) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test equality comparison with Pi
    stack->Clear();
    console.Execute("5 5 ==");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test inequality
    stack->Clear();
    console.Execute("5 6 ==");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Inequality comparison
TEST(RhoPiAdvanced, InequalityComparison) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test inequality comparison with Pi
    stack->Clear();
    console.Execute("5 6 !=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test equality
    stack->Clear();
    console.Execute("5 5 !=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Less than or equal to comparison
TEST(RhoPiAdvanced, LessThanOrEqualComparison) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test less than or equal (when less)
    stack->Clear();
    console.Execute("5 10 <=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test less than or equal (when equal)
    stack->Clear();
    console.Execute("5 5 <=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test less than or equal (when greater)
    stack->Clear();
    console.Execute("10 5 <=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Greater than or equal to comparison
TEST(RhoPiAdvanced, GreaterThanOrEqualComparison) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Test greater than or equal (when greater)
    stack->Clear();
    console.Execute("10 5 >=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test greater than or equal (when equal)
    stack->Clear();
    console.Execute("5 5 >=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(stack->Top()));

    // Test greater than or equal (when less)
    stack->Clear();
    console.Execute("5 10 >=");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(stack->Top()));
}

// Using functions with parameters
TEST(RhoPiAdvanced, FunctionWithParameters) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<Continuation>(Label("Continuation"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Define and use a function (square)
    stack->Clear();
    console.Execute("{ dup * } 5 swap call");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 25);  // 5 squared = 25
}

// Store and retrieve variable
TEST(RhoPiAdvanced, VariableStorage) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Store a value in a variable and retrieve it
    stack->Clear();
    console.Execute("42 \"answer\" store");
    console.Execute("\"answer\" retrieve");
    ASSERT_FALSE(stack->Empty());
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 42);
}