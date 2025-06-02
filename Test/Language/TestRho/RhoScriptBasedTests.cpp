#include <gtest/gtest.h>
#include "RhoTestBase.h"

// Test suite demonstrating script-based testing
struct RhoScriptBasedTests : RhoTestBase {
};

TEST_F(RhoScriptBasedTests, BasicAddFunction) {
    RunScriptAndExpect<int>("basic_add_function.rho", 5);
}

TEST_F(RhoScriptBasedTests, BasicMultiplyFunction) {
    RunScriptAndExpect<int>("basic_multiply_function.rho", 20);
}

TEST_F(RhoScriptBasedTests, NestedFunctionCalls) {
    RunScriptAndExpect<int>("nested_function_calls.rho", 10);
}

TEST_F(RhoScriptBasedTests, ForLoopSum) {
    RunScriptAndExpect<int>("for_loop_sum.rho", 15); // 1+2+3+4+5
}

TEST_F(RhoScriptBasedTests, WhileLoopFactorial) {
    RunScriptAndExpect<int>("while_loop_factorial.rho", 120); // 5!
}

TEST_F(RhoScriptBasedTests, IfElseBasic) {
    RunScriptAndExpect<int>("if_else_basic.rho", 1); // x=10 > 5
}

TEST_F(RhoScriptBasedTests, ArrayOperations) {
    RunScriptAndExpect<int>("array_operations.rho", 15); // sum of [1,2,3,4,5]
}

TEST_F(RhoScriptBasedTests, PiBlockBasic) {
    RunScriptAndExpect<int>("pi_block_basic.rho", 5); // 2 + 3
}

TEST_F(RhoScriptBasedTests, FunctionWithLocals) {
    RunScriptAndExpect<int>("function_with_locals.rho", 10); // 3*2 + 4
}