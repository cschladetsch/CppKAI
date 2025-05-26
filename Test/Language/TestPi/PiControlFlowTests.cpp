#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Test suite for Pi control flow operations
TEST(PiControlFlow, ConditionalExecution) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Test if-then
    console.Execute("true { 100 } if");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
    
    stack->Clear();
    
    // Test if-then with false condition
    console.Execute("false { 200 } if");
    EXPECT_EQ(stack->Size(), 0);
}

TEST(PiControlFlow, IfElseExecution) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Test if-then-else with true
    console.Execute("true { 1 } { 2 } ifelse");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 1);
    
    stack->Clear();
    
    // Test if-then-else with false
    console.Execute("false { 1 } { 2 } ifelse");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST(PiControlFlow, WhileLoop) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Count from 1 to 5
    console.Execute("1 { dup 5 <= } { dup 1 + } while drop");
    auto stack = exec->GetDataStack();
    
    // Stack should be empty after drop
    EXPECT_EQ(stack->Size(), 0);
}

TEST(PiControlFlow, ForLoop) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Sum numbers from 1 to 5
    console.Execute("0 1 5 { + } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15); // 1+2+3+4+5
}

TEST(PiControlFlow, NestedBlocks) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Nested conditional blocks
    console.Execute("true { false { 1 } { 2 } ifelse } { 3 } ifelse");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}