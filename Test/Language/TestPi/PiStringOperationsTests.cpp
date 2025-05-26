#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Test suite for Pi string operations
TEST(PiStringOperations, StringConcatenation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto &exec = console.GetExecutor();
    
    console.Execute("\"Hello \" \"World\" +");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello World");
}

TEST(PiStringOperations, StringLength) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto &exec = console.GetExecutor();
    
    console.Execute("\"Hello\" size");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST(PiStringOperations, StringIndexing) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto &exec = console.GetExecutor();
    
    console.Execute("\"ABCDE\" 2 at");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<char>(stack->Top()), 'C');
}

TEST(PiStringOperations, StringSlicing) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto &exec = console.GetExecutor();
    
    console.Execute("\"Hello World\" 0 5 slice");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello");
}

TEST(PiStringOperations, StringToNumber) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto &exec = console.GetExecutor();
    
    console.Execute("\"123\" toint");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 123);
}