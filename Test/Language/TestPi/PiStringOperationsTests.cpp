#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test fixture for Pi string operations
struct PiStringOperationsTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test suite for Pi string operations
TEST_F(PiStringOperationsTests, StringConcatenation) {
    console_.Execute("\"Hello \" \"World\" +");
    auto stack = console_.GetExecutor()->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello World");
}

TEST_F(PiStringOperationsTests, StringLength) {
    console_.Execute("\"Hello\" size");
    auto stack = console_.GetExecutor()->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// TODO: at operation not implemented in Pi language yet
// TEST(PiStringOperations, StringIndexing) {
//     kai::Console console;
//     console.SetLanguage(kai::Language::Pi);
//     auto exec = console.GetExecutor();
//
//     console.Execute("\"ABCDE\" 2 at");
//     auto stack = exec->GetDataStack();
//
//     ASSERT_EQ(stack->Size(), 1);
//     // Character operations in Pi return int values
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), static_cast<int>('C'));
// }

// TODO: slice operation not implemented in Pi language yet
// TEST(PiStringOperations, StringSlicing) {
//     kai::Console console;
//     console.SetLanguage(kai::Language::Pi);
//     auto exec = console.GetExecutor();
//
//     console.Execute("\"Hello World\" 0 5 slice");
//     auto stack = exec->GetDataStack();
//
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello");
// }

// TODO: toint/tofloat operations not implemented in Pi language yet
// TEST(PiStringOperations, StringToNumber) {
//     kai::Console console;
//     console.SetLanguage(kai::Language::Pi);
//     auto exec = console.GetExecutor();
//
//     console.Execute("\"123\" toint");
//     auto stack = exec->GetDataStack();
//
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 123);
// }