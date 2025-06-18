#include <gtest/gtest.h>

#include <iostream>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Executor/Continuation.h"
#include "TestLangCommon.h"

struct PiMinMaxSimpleTests : kai::TestLangCommon {
    kai::Registry* reg_;
    kai::Executor* exec_;

    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
        reg_ = &console_.GetRegistry();
        exec_ = &*console_.GetExecutor();

        // Clear stacks
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    void TearDown() override {
        exec_->ClearStacks();
        exec_->ClearContext();
        TestLangCommon::TearDown();
    }
};

TEST_F(PiMinMaxSimpleTests, IntegerMinMax) {
    // Test integer min
    console_.Execute("5 3 min");
    auto stack = exec_->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    stack->Clear();

    // Test integer max
    console_.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
    stack->Clear();

    // Test with negative numbers
    console_.Execute("0 5 - 3 min");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), -5);
    stack->Clear();

    console_.Execute("0 5 - 3 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    stack->Clear();
}

TEST_F(PiMinMaxSimpleTests, StringMinMax) {
    // Test with strings
    console_.Execute("\"apple\" \"banana\" min");
    auto stack = exec_->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "apple");
    stack->Clear();

    console_.Execute("\"apple\" \"banana\" max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "banana");
    stack->Clear();
}

TEST_F(PiMinMaxSimpleTests, ChainedMinMax) {
    // Test chained min operations
    console_.Execute("10 20 30 min min");
    auto stack = exec_->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 10);
    stack->Clear();

    // Test chained max operations
    console_.Execute("10 20 30 max max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
    stack->Clear();
}