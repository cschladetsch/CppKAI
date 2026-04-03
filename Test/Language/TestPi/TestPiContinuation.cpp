#include "TestLangCommon.h"

using namespace kai;

struct TestPiContinuation : TestLangCommon {};

TEST_F(TestPiContinuation, SimpleContinuationCreation) {
    console_.SetLanguage(Language::Pi);
    auto exec = console_.GetExecutor();

    // Test that { 100 } creates a continuation object, not directly pushes 100
    console_.Execute("{ 100 }");
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1) << "Expected one item on stack";

    auto top = stack->Top();
    EXPECT_TRUE(top.IsType<Continuation>())
        << "Expected a Continuation object on stack, got: "
        << (top.GetClass() ? top.GetClass()->GetName().ToString() : "null");

    if (top.IsType<Continuation>()) {
        auto cont = ConstDeref<Continuation>(top);
        auto code = cont.GetCode();
        ASSERT_TRUE(code.Exists()) << "Continuation should have code";

        // The continuation should contain: ContinuationBegin, 100,
        // ContinuationEnd Or at least contain the value 100
        bool found100 = false;
        for (int i = 0; i < code->Size(); ++i) {
            auto item = code->At(i);
            if (item.IsType<int>() && ConstDeref<int>(item) == 100) {
                found100 = true;
                break;
            }
        }
        EXPECT_TRUE(found100) << "Continuation should contain the value 100";
    }
}

TEST_F(TestPiContinuation, ContinuationExecution) {
    console_.SetLanguage(Language::Pi);
    auto exec = console_.GetExecutor();

    // Test that continuations can be executed with &
    console_.Execute("{ 100 }");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    EXPECT_TRUE(exec->GetDataStack()->Top().IsType<Continuation>());

    // Now execute it with &
    console_.Execute("&");
    ASSERT_EQ(exec->GetDataStack()->Size(), 1);
    auto result = exec->GetDataStack()->Top();
    EXPECT_TRUE(result.IsType<int>())
        << "Expected int after executing continuation";
    if (result.IsType<int>()) {
        EXPECT_EQ(ConstDeref<int>(result), 100);
    }
}