#include <gtest/gtest.h>

#include "RhoTestBase.h"

// Test suite demonstrating script-based testing
struct RhoScriptBasedTests : RhoTestBase {};

TEST_F(RhoScriptBasedTests, BasicAddFunction) {
    RunScriptAndExpect<int>("BasicAddFunction.rho", 5);
}

TEST_F(RhoScriptBasedTests, BasicMultiplyFunction) {
    RunScriptAndExpect<int>("BasicMultiplyFunction.rho", 20);
}

TEST_F(RhoScriptBasedTests, NestedFunctionCalls) {
    RunScriptAndExpect<int>("NestedFunctionCalls.rho", 10);
}

TEST_F(RhoScriptBasedTests, ForLoopSum) {
    RunScriptAndExpect<int>("ForLoopSum.rho", 15);  // 1+2+3+4+5
}

TEST_F(RhoScriptBasedTests, WhileLoopFactorial) {
    RunScriptAndExpect<int>("WhileLoopFactorial.rho", 120);  // 5!
}

TEST_F(RhoScriptBasedTests, IfElseBasic) {
    RunScriptAndExpect<int>("IfElseBasic.rho", 1);  // x=10 > 5
}

TEST_F(RhoScriptBasedTests, ArrayOperations) {
    RunScriptAndExpect<int>("ArrayOperations.rho", 15);  // sum of [1,2,3,4,5]
}

TEST_F(RhoScriptBasedTests, PiBlockBasic) {
    RunScriptAndExpect<int>("PiBlockBasic.rho", 5);  // 2 + 3
}

TEST_F(RhoScriptBasedTests, FunctionWithLocals) {
    RunScriptAndExpect<int>("FunctionWithLocals.rho", 10);  // 3*2 + 4
}