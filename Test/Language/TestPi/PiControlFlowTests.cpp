#include "TestLangCommon.h"

// Test suite for Pi control flow operations
struct PiControlFlowTest : kai::TestLangCommon {
};

// TODO: Code blocks { ... } are not properly translated to continuations in Pi
// TEST_F(PiControlFlowTest, ConditionalExecution) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//     
//     // Test if-then
//     console_.Execute("true { 100 } if");
//     auto stack = exec->GetDataStack();
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
//     
//     stack->Clear();
//     
//     // Test if-then with false condition
//     console_.Execute("false { 200 } if");
//     EXPECT_EQ(stack->Size(), 0);
// }

// TODO: Code blocks { ... } are not properly translated to continuations in Pi
// TEST_F(PiControlFlowTest, IfElseExecution) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//     
//     // Test if-then-else with true
//     console_.Execute("true { 1 } { 2 } ife");
//     auto stack = exec->GetDataStack();
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 1);
//     
//     stack->Clear();
//     
//     // Test if-then-else with false
//     console_.Execute("false { 1 } { 2 } ife");
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
// }

// TODO: while operation not implemented in Pi language yet
// TEST_F(PiControlFlowTest, WhileLoop) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//     
//     // Count from 1 to 5
//     console_.Execute("1 { dup 5 <= } { dup 1 + } while drop");
//     auto stack = exec->GetDataStack();
//     
//     // Stack should be empty after drop
//     EXPECT_EQ(stack->Size(), 0);
// }

// TODO: for operation not implemented in Pi language yet
// TEST_F(PiControlFlowTest, ForLoop) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//     
//     // Sum numbers from 1 to 5
//     console_.Execute("0 1 5 { + } for");
//     auto stack = exec->GetDataStack();
//     
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15); // 1+2+3+4+5
// }

// TODO: Code blocks { ... } are not properly translated to continuations in Pi
// TEST_F(PiControlFlowTest, NestedBlocks) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//     
//     // Nested conditional blocks
//     console_.Execute("true { false { 1 } { 2 } ife } { 3 } ife");
//     auto stack = exec->GetDataStack();
//     
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
// }