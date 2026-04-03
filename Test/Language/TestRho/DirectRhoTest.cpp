#include <gtest/gtest.h>

#include "KAI/Core/Console.h"

using namespace kai;

TEST(DirectRhoTest, SimpleAssignment) {
    Console console;
    console.SetLanguage(Language::Rho);
    auto exec = console.GetExecutor();

    // Test 1: Simple assignment
    console.Execute("x = 42");
    exec->GetDataStack()->Clear();

    // Test 2: Retrieve value
    console.Execute("x");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have x value";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "x should be int";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 42) << "x should be 42";
}

TEST(DirectRhoTest, SimpleArithmetic) {
    Console console;
    console.SetLanguage(Language::Rho);
    auto exec = console.GetExecutor();

    // Test arithmetic
    console.Execute("2 + 3");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have result";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be int";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5) << "2 + 3 should be 5";
}

TEST(DirectRhoTest, SimpleIfThen) {
    Console console;
    console.SetLanguage(Language::Rho);
    auto exec = console.GetExecutor();

    // Test if-then using Rho indentation syntax
    console.Execute("result = 0");
    exec->GetDataStack()->Clear();
    console.Execute("if true\n\tresult = 100");
    exec->GetDataStack()->Clear();
    console.Execute("result");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have result";
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be int";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 100) << "Result should be 100";
}

TEST(DirectRhoTest, SimpleIfElse) {
    Console console;
    console.SetLanguage(Language::Rho);
    auto exec = console.GetExecutor();

    // Test if-else with true condition using Rho indentation syntax
    console.Execute("r1 = 0");
    exec->GetDataStack()->Clear();
    console.Execute("if true\n\tr1 = 10\nelse\n\tr1 = 20");
    exec->GetDataStack()->Clear();
    console.Execute("r1");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have r1";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 10) << "r1 should be 10";

    // Test if-else with false condition using Rho indentation syntax
    stack->Clear();
    console.Execute("r2 = 0");
    stack->Clear();
    console.Execute("if false\n\tr2 = 30\nelse\n\tr2 = 40");
    stack->Clear();
    console.Execute("r2");

    ASSERT_EQ(stack->Size(), 1) << "Stack should have r2";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 40) << "r2 should be 40";
}