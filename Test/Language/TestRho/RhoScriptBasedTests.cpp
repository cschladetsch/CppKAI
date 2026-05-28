#include <gtest/gtest.h>
#include "RhoTestBase.h"

struct RhoScriptBasedTests : RhoTestBase {};

TEST_F(RhoScriptBasedTests, BasicAddFunction) {
    RunScriptAndExpect<int>("BasicAddFunction.rho", 5);
}
TEST_F(RhoScriptBasedTests, BasicMultiplyFunction) {
    RunScriptAndExpect<int>("BasicMultiplyFunction.rho", 20);
}
TEST_F(RhoScriptBasedTests, NestedFunctionCalls) {
    RunScriptAndExpect<int>("NestedFunctionCalls.rho", 12);
}
TEST_F(RhoScriptBasedTests, ForLoopSum) {
    RunScriptAndExpect<int>("ForLoopSum.rho", 15);
}
TEST_F(RhoScriptBasedTests, WhileLoopFactorial) {
    RunScriptAndExpect<int>("WhileLoopFactorial.rho", 120);
}
TEST_F(RhoScriptBasedTests, IfElseBasic) {
    RunScriptAndExpect<int>("IfElseBasic.rho", 1);
}
TEST_F(RhoScriptBasedTests, ArrayOperations) {
    RunScriptAndExpect<int>("ArrayOperations.rho", 15);
}
TEST_F(RhoScriptBasedTests, PiBlockBasic) {
    RunScriptAndExpect<int>("PiBlockBasic.rho", 5);
}
TEST_F(RhoScriptBasedTests, FunctionWithLocals) {
    RunScriptAndExpect<int>("FunctionWithLocals.rho", 10);
}
TEST_F(RhoScriptBasedTests, PiBlockAdvanced) {
    ExecScriptFile("PiBlockAdvanced.rho");
}
TEST_F(RhoScriptBasedTests, NestedLoopsRecursion) {
    ExecScriptFile("NestedLoopsRecursion.rho");
}

TEST_F(RhoScriptBasedTests, ScopingTests) {
    ExecScriptFile("ScopingTests.rho");
}
TEST_F(RhoScriptBasedTests, CollectionTests) {
    ExecScriptFile("CollectionTests.rho");
}
TEST_F(RhoScriptBasedTests, StringTests) {
    ExecScriptFile("StringTests.rho");
}
TEST_F(RhoScriptBasedTests, PiRhoInteropTests) {
    ExecScriptFile("PiRhoInteropTests.rho");
}
TEST_F(RhoScriptBasedTests, HigherOrderTests) {
    ExecScriptFile("HigherOrderTests.rho");
}
