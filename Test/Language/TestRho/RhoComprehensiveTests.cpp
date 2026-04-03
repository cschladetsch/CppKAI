#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Comprehensive test suite for Rho language - 100 tests covering edge cases and core features
struct RhoComprehensiveTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

// ============================================================================
// ARITHMETIC TESTS (10 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, Addition) {
    console_.Execute("result = 5 + 3");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 8);
}

TEST_F(RhoComprehensiveTests, Subtraction) {
    console_.Execute("result = 10 - 4");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, Multiplication) {
    console_.Execute("result = 7 * 6");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

TEST_F(RhoComprehensiveTests, Division) {
    console_.Execute("result = 20 / 5");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 4);
}

TEST_F(RhoComprehensiveTests, Modulo) {
    console_.Execute("result = 17 % 5");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 2);
}

TEST_F(RhoComprehensiveTests, NegativeNumbers) {
    console_.Execute("result = 0 - 5 + 3");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), -2);
}

TEST_F(RhoComprehensiveTests, ChainedArithmetic) {
    console_.Execute("result = 2 + 3 * 4");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 14);
}

TEST_F(RhoComprehensiveTests, ParenthesesPrecedence) {
    console_.Execute("result = (2 + 3) * 4");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, MultipleOperations) {
    console_.Execute("result = 100 - 50 + 25 - 10");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 65);
}

TEST_F(RhoComprehensiveTests, ComplexExpression) {
    console_.Execute("result = ((10 + 5) * 2 - 6) / 3");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 8);
}

// ============================================================================
// COMPARISON TESTS (10 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, EqualityTrue) {
    console_.Execute("result = 5 == 5");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, EqualityFalse) {
    console_.Execute("result = 5 == 6");
    console_.Execute("result");
    ASSERT_FALSE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, NotEqual) {
    console_.Execute("result = 5 != 6");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, LessThan) {
    console_.Execute("result = 3 < 5");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, LessThanOrEqual) {
    console_.Execute("result = 5 <= 5");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, GreaterThan) {
    console_.Execute("result = 7 > 3");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, GreaterThanOrEqual) {
    console_.Execute("result = 5 >= 5");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, ComparisonChain) {
    console_.Execute("result = 3 < 5");
    console_.Execute("result2 = 5 < 10");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, NegativeComparison) {
    console_.Execute("result = 0 - 5 < 0 - 3");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, ZeroComparison) {
    console_.Execute("result = 0 == 0");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

// ============================================================================
// VARIABLE TESTS (10 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, SimpleAssignment) {
    console_.Execute("x = 42");
    console_.Execute("x");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

TEST_F(RhoComprehensiveTests, MultipleAssignments) {
    console_.Execute("a = 1");
    console_.Execute("b = 2");
    console_.Execute("c = 3");
    console_.Execute("c");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 3);
}

TEST_F(RhoComprehensiveTests, VariableReassignment) {
    console_.Execute("x = 10");
    console_.Execute("x = 20");
    console_.Execute("x");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, VariableInExpression) {
    console_.Execute("x = 5");
    console_.Execute("y = x + 10");
    console_.Execute("y");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 15);
}

TEST_F(RhoComprehensiveTests, VariableUpdate) {
    console_.Execute("count = 0");
    console_.Execute("count = count + 1");
    console_.Execute("count");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, MultipleVariableExpression) {
    console_.Execute("a = 10");
    console_.Execute("b = 20");
    console_.Execute("result = a + b");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 30);
}

TEST_F(RhoComprehensiveTests, VariableChaining) {
    console_.Execute("x = 5");
    console_.Execute("y = x");
    console_.Execute("z = y");
    console_.Execute("z");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 5);
}

TEST_F(RhoComprehensiveTests, ComplexVariableExpression) {
    console_.Execute("a = 2");
    console_.Execute("b = 3");
    console_.Execute("c = 4");
    console_.Execute("result = a * b + c");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, NegativeVariableValue) {
    console_.Execute("neg = 0 - 10");
    console_.Execute("neg");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), -10);
}

TEST_F(RhoComprehensiveTests, ZeroVariable) {
    console_.Execute("zero = 0");
    console_.Execute("zero");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

// ============================================================================
// FUNCTION TESTS (20 tests) - Using implicit return
// ============================================================================

TEST_F(RhoComprehensiveTests, SimpleFunctionDefinition) {
    console_.Execute("add = fun(a, b)\n    a + b");
    console_.Execute("add(3, 4)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 7);
}

TEST_F(RhoComprehensiveTests, FunctionWithOneParameter) {
    console_.Execute("square = fun(x)\n    x * x");
    console_.Execute("square(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 25);
}

TEST_F(RhoComprehensiveTests, FunctionWithThreeParameters) {
    console_.Execute("sum3 = fun(a, b, c)\n    a + b + c");
    console_.Execute("sum3(1, 2, 3)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, StandaloneFunctionSyntax) {
    console_.Execute("fun double(x)\n    x * 2");
    console_.Execute("double(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, FunctionReturningConstant) {
    console_.Execute("getFortyTwo = fun()\n    42");
    console_.Execute("getFortyTwo()");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

TEST_F(RhoComprehensiveTests, FunctionCallingFunction) {
    console_.Execute("double = fun(x)\n    x * 2");
    console_.Execute("quadruple = fun(x)\n    double(double(x))");
    console_.Execute("quadruple(3)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 12);
}

TEST_F(RhoComprehensiveTests, FunctionWithLocalVariable) {
    console_.Execute("compute = fun(x)\n    temp = x * 2\n    temp + 1");
    console_.Execute("compute(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 11);
}

TEST_F(RhoComprehensiveTests, FunctionWithMultipleStatements) {
    console_.Execute("calc = fun(x)\n    a = x + 1\n    b = a * 2\n    b");
    console_.Execute("calc(4)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, RecursiveCountdown) {
    console_.Execute("countdown = fun(n)\n    if n <= 0\n        0\n    else\n        countdown(n - 1)");
    console_.Execute("countdown(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, SimpleFactorial) {
    console_.Execute("factorial = fun(n)\n    if n <= 1\n        1\n    else\n        n * factorial(n - 1)");
    console_.Execute("factorial(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 120);
}

TEST_F(RhoComprehensiveTests, FunctionReturningSum) {
    console_.Execute("sumRange = fun(start, end)\n    if start > end\n        0\n    else\n        start + sumRange(start + 1, end)");
    console_.Execute("sumRange(1, 5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 15);
}

TEST_F(RhoComprehensiveTests, FunctionWithNegativeParameter) {
    console_.Execute("abs = fun(x)\n    if x < 0\n        0 - x\n    else\n        x");
    console_.Execute("abs(0 - 5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 5);
}

TEST_F(RhoComprehensiveTests, FunctionReturningBoolean) {
    console_.Execute("isPositive = fun(x)\n    x > 0");
    console_.Execute("isPositive(5)");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, FunctionWithComplexLogic) {
    console_.Execute("maxFunc = fun(a, b)\n    if a > b\n        a\n    else\n        b");
    console_.Execute("maxFunc(10, 20)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, MinFunction) {
    console_.Execute("minFunc = fun(a, b)\n    if a < b\n        a\n    else\n        b");
    console_.Execute("minFunc(10, 20)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, FunctionParameterShadowing) {
    console_.Execute("x = 100");
    console_.Execute("f = fun(x)\n    x + 1");
    console_.Execute("f(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, MultipleReturnPaths) {
    console_.Execute("sign = fun(x)\n    if x < 0\n        0 - 1\n    else\n        if x > 0\n            1\n        else\n            0");
    console_.Execute("sign(0)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, FunctionComposition) {
    console_.Execute("add1 = fun(x)\n    x + 1");
    console_.Execute("mul2 = fun(x)\n    x * 2");
    console_.Execute("result = mul2(add1(5))");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 12);
}

TEST_F(RhoComprehensiveTests, FunctionStoredInVariable) {
    console_.Execute("myFunc = fun(x)\n    x + 10");
    console_.Execute("result = myFunc(5)");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 15);
}

TEST_F(RhoComprehensiveTests, FunctionReassignment) {
    console_.Execute("f = fun(x)\n    x + 1");
    console_.Execute("f = fun(x)\n    x * 2");
    console_.Execute("f(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

// ============================================================================
// IF/ELSE TESTS (15 tests) - Using functions to work around if-expression limitation
// ============================================================================

TEST_F(RhoComprehensiveTests, SimpleIfTrue) {
    console_.Execute("test = fun()\n    if true\n        1\n    else\n        0");
    console_.Execute("test()");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, SimpleIfFalse) {
    console_.Execute("test = fun()\n    if false\n        1\n    else\n        0");
    console_.Execute("test()");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, IfWithComparison) {
    console_.Execute("test = fun()\n    if 5 > 3\n        100\n    else\n        200");
    console_.Execute("test()");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 100);
}

TEST_F(RhoComprehensiveTests, IfWithVariable) {
    console_.Execute("test = fun(x)\n    if x > 5\n        1\n    else\n        0");
    console_.Execute("test(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, NestedIf) {
    console_.Execute("test = fun(x)\n    if x > 5\n        if x > 15\n            2\n        else\n            1\n    else\n        0");
    console_.Execute("test(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfElseIfElse) {
    console_.Execute("test = fun(x)\n    if x < 5\n        1\n    else\n        if x < 15\n            2\n        else\n            3");
    console_.Execute("test(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 2);
}

TEST_F(RhoComprehensiveTests, IfWithExpression) {
    console_.Execute("test = fun(x)\n    if x * 2 > 8\n        1\n    else\n        0");
    console_.Execute("test(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfReturningComputation) {
    console_.Execute("test = fun(x)\n    if x > 5\n        x * 2\n    else\n        x / 2");
    console_.Execute("test(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, IfWithEquality) {
    console_.Execute("test = fun(x)\n    if x == 42\n        1\n    else\n        0");
    console_.Execute("test(42)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfWithNotEqual) {
    console_.Execute("test = fun(x)\n    if x != 10\n        1\n    else\n        0");
    console_.Execute("test(5)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfInFunction) {
    console_.Execute("abs = fun(x)\n    if x < 0\n        0 - x\n    else\n        x");
    console_.Execute("abs(0 - 10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, MultipleIfStatements) {
    console_.Execute("test1 = fun(x)\n    if x > 0\n        1\n    else\n        0");
    console_.Execute("test2 = fun(y)\n    if y > 0\n        1\n    else\n        0");
    console_.Execute("test2(10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfWithZero) {
    console_.Execute("test = fun(x)\n    if x == 0\n        1\n    else\n        0");
    console_.Execute("test(0)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, IfBothBranchesExecutable) {
    console_.Execute("test = fun(toggle)\n    if toggle\n        10\n    else\n        20");
    console_.Execute("test(true)");
    data_->Clear();
    console_.Execute("test(false)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, ComplexCondition) {
    console_.Execute("test = fun(x, y)\n    if x < y\n        if x > 0\n            1\n        else\n            2\n    else\n        3");
    console_.Execute("test(5, 10)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

// ============================================================================
// LOOP TESTS (15 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, SimpleWhileLoop) {
    console_.Execute("i = 0\nsum = 0\nwhile i < 5\n    i = i + 1\n    sum = sum + i\nsum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 15);
}

TEST_F(RhoComprehensiveTests, WhileLoopCountdown) {
    console_.Execute("n = 5; while n > 0; n = n - 1; n");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, WhileLoopProduct) {
    console_.Execute("i = 1\nprod = 1\nwhile i <= 4\n    prod = prod * i\n    i = i + 1\nprod");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 24);
}

TEST_F(RhoComprehensiveTests, NestedWhileLoops) {
    console_.Execute("i = 0\nsum = 0\nwhile i < 3\n    j = 0\n    while j < 2\n        sum = sum + 1\n        j = j + 1\n    i = i + 1\nsum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, WhileWithCondition) {
    console_.Execute("x = 10; while x > 5; x = x - 1; x");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 5);
}

TEST_F(RhoComprehensiveTests, ForLoopSum) {
    console_.Execute("sum = 0; for i = 0; i < 5; i = i + 1; sum = sum + i; sum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, ForLoopProduct) {
    console_.Execute("prod = 1; for i = 1; i <= 5; i = i + 1; prod = prod * i; prod");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 120);
}

TEST_F(RhoComprehensiveTests, ForLoopWithStep) {
    console_.Execute("sum = 0; for i = 0; i < 10; i = i + 2; sum = sum + i; sum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 20);
}

TEST_F(RhoComprehensiveTests, ForLoopCountdown) {
    console_.Execute("result = 0; for i = 10; i > 0; i = i - 1; result = i; result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, NestedForLoops) {
    console_.Execute("sum = 0; for i = 0; i < 3; i = i + 1; for j = 0; j < 2; j = j + 1; sum = sum + 1; sum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, WhileLoopZeroIterations) {
    console_.Execute("sum = 0\ni = 10\nwhile i < 5\n    sum = sum + 1\n    i = i + 1\nsum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, ForLoopSingleIteration) {
    console_.Execute("count = 0; for i = 0; i < 1; i = i + 1; count = count + 1; count");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
}

TEST_F(RhoComprehensiveTests, WhileWithVariable) {
    console_.Execute("limit = 5\ni = 0\nsum = 0\nwhile i < limit\n    sum = sum + i\n    i = i + 1\nsum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, LoopWithFunctionCall) {
    console_.Execute("double = fun(x)\n    x * 2");
    console_.Execute("sum = 0; for i = 1; i <= 3; i = i + 1; sum = sum + double(i); sum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 12);
}

TEST_F(RhoComprehensiveTests, ComplexLoopCondition) {
    console_.Execute("sum = 0\ni = 0\nwhile i * i < 25\n    sum = sum + i\n    i = i + 1\nsum");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

// ============================================================================
// PI BLOCK TESTS (10 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, SimplePiBlock) {
    console_.Execute("result = pi{ 5 3 + }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 8);
}

TEST_F(RhoComprehensiveTests, PiBlockMultiplication) {
    console_.Execute("result = pi{ 7 6 * }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

TEST_F(RhoComprehensiveTests, PiBlockWithDup) {
    console_.Execute("result = pi{ 5 dup * }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 25);
}

TEST_F(RhoComprehensiveTests, PiBlockWithSwap) {
    console_.Execute("result = pi{ 10 3 swap - }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), -7);
}

TEST_F(RhoComprehensiveTests, PiBlockStackOps) {
    console_.Execute("result = pi{ 1 2 3 + + }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

TEST_F(RhoComprehensiveTests, PiBlockWithOver) {
    console_.Execute("result = pi{ 5 3 over + + }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 13);
}

TEST_F(RhoComprehensiveTests, PiBlockComparison) {
    console_.Execute("result = pi{ 5 3 > }");
    console_.Execute("result");
    ASSERT_TRUE(kai::ConstDeref<bool>(data_->Top()));
}

TEST_F(RhoComprehensiveTests, PiBlockNested) {
    console_.Execute("x = pi{ 2 3 + }");
    console_.Execute("result = pi{ x 2 * }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoComprehensiveTests, PiBlockInFunction) {
    console_.Execute("compute = fun(x)\n    pi{ x dup * }");
    console_.Execute("compute(7)");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 49);
}

TEST_F(RhoComprehensiveTests, PiBlockComplex) {
    console_.Execute("result = pi{ 10 5 swap over / }");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 2);
}

// ============================================================================
// EDGE CASE TESTS (10 tests)
// ============================================================================

TEST_F(RhoComprehensiveTests, LargeNumber) {
    console_.Execute("result = 1000000");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1000000);
}

TEST_F(RhoComprehensiveTests, VeryNegativeNumber) {
    console_.Execute("result = 0 - 999999");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), -999999);
}

TEST_F(RhoComprehensiveTests, DivisionBySmallNumber) {
    console_.Execute("result = 1000 / 1");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 1000);
}

TEST_F(RhoComprehensiveTests, MultiplicationByZero) {
    console_.Execute("result = 42 * 0");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, AdditionWithZero) {
    console_.Execute("result = 0 + 100");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 100);
}

TEST_F(RhoComprehensiveTests, SubtractionToZero) {
    console_.Execute("result = 42 - 42");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, ModuloByOne) {
    console_.Execute("result = 100 % 1");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 0);
}

TEST_F(RhoComprehensiveTests, IdentityOperation) {
    console_.Execute("x = 42");
    console_.Execute("result = x");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

TEST_F(RhoComprehensiveTests, DoubleNegation) {
    console_.Execute("result = 0 - (0 - 5)");
    console_.Execute("result");
    ASSERT_EQ(kai::ConstDeref<int>(data_->Top()), 5);
}

TEST_F(RhoComprehensiveTests, EmptyPiBlock) {
    EXPECT_THROW(console_.Execute("x = pi{}"), kai::Exception::Base);
}
