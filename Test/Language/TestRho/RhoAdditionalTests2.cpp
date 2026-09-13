// Additional Rho language coverage, following the console_.Execute(code,
// Structure::Program) / data_->Top() pattern used throughout
// SimpleForLoopTests.cpp, WhileLoopTest.cpp and FunctionSyntaxTest.cpp: more
// for/while loop variations, if/else branches, functions (including
// recursion and closures), and basic string/array expressions.

#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct RhoAdditionalTests2 : TestLangCommon {};

// --- for loops ---

TEST_F(RhoAdditionalTests2, ForLoop_SumRange) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
sum = 0
for i = 1; i <= 10; i = i + 1
    sum = sum + i
sum
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(55, ConstDeref<int>(result));  // 1..10
}

TEST_F(RhoAdditionalTests2, ForLoop_ProductRange) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
product = 1
for i = 1; i <= 5; i = i + 1
    product = product * i
product
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(120, ConstDeref<int>(result));  // 5!
}

TEST_F(RhoAdditionalTests2, ForLoop_StepByTwo) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
count = 0
for i = 0; i < 10; i = i + 2
    count = count + 1
count
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));  // 0,2,4,6,8
}

TEST_F(RhoAdditionalTests2, ForLoop_ZeroIterations) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
sum = 100
for i = 5; i < 5; i = i + 1
    sum = sum + i
sum
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(100, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, ForLoop_Nested) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
total = 0
for i = 0; i < 3; i = i + 1
    for j = 0; j < 3; j = j + 1
        total = total + 1
total
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(9, ConstDeref<int>(result));
}

// --- while loops ---

TEST_F(RhoAdditionalTests2, WhileLoop_CountDown) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
n = 5
steps = 0
while n > 0
    n = n - 1
    steps = steps + 1
steps
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, WhileLoop_NeverExecutesWhenFalseUpfront) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
count = 0
while count > 0
    count = count + 1
count
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(0, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, WhileLoop_AccumulateSquares) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
sum = 0
i = 1
while i <= 4
    sum = sum + i * i
    i = i + 1
sum
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(30, ConstDeref<int>(result));  // 1+4+9+16
}

// --- if/else ---

TEST_F(RhoAdditionalTests2, IfElse_TrueBranch) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 10
result = 0
if x > 5
    result = 1
else
    result = 2
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(1, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, IfElse_FalseBranch) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 2
result = 0
if x > 5
    result = 1
else
    result = 2
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, If_NoElseBranchSkipped) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 1
result = 42
if x > 5
    result = 1
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, IfElse_EqualityCondition) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 7
result = 0
if x == 7
    result = 100
else
    result = 200
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(100, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, IfElse_InsideForLoop) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
evens = 0
for i = 0; i < 10; i = i + 1
    if i % 2 == 0
        evens = evens + 1
evens
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));  // 0,2,4,6,8
}

// --- functions ---

TEST_F(RhoAdditionalTests2, Function_SingleArgument) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
square = fun(x)
    x * x
result = square(6)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(36, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_TwoArguments) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
multiply = fun(a, b)
    a * b
result = multiply(6, 7)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_CalledMultipleTimes) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
increment = fun(x)
    x + 1
a = increment(1)
b = increment(a)
c = increment(b)
c
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(4, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_RecursiveFibonacci) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
fib = fun(n)
    if n <= 1
        n
    else
        fib(n - 1) + fib(n - 2)

result = fib(8)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(21, ConstDeref<int>(result));  // fib(8) = 21
}

TEST_F(RhoAdditionalTests2, Function_RecursiveCountdown) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
countdown = fun(n)
    if n <= 0
        0
    else
        1 + countdown(n - 1)

result = countdown(6)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_UsedInsideForLoop) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
triple = fun(x)
    x * 3
sum = 0
for i = 1; i <= 3; i = i + 1
    sum = sum + triple(i)
sum
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(18, ConstDeref<int>(result));  // 3+6+9
}

TEST_F(RhoAdditionalTests2, Function_ClosureCapturesOuterVariable) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
makeAdder = fun(x)
    fun(y)
        y + x

add10 = makeAdder(10)
result = add10(7)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(17, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_PassedAsArgument) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
apply = fun(f, x)
    f(x)
negate = fun(n)
    0 - n

result = apply(negate, 15)
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(-15, ConstDeref<int>(result));
}

// --- expressions / operators ---

TEST_F(RhoAdditionalTests2, Expression_OperatorPrecedence) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
result = 2 + 3 * 4
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(14, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Expression_Parentheses) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
result = (2 + 3) * 4
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(20, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Expression_ModuloOperator) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
result = 17 % 5
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Expression_NegativeNumberArithmetic) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
result = -5 + 8
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Expression_ChainedComparison) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 5
result = x >= 5
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<bool>());
    EXPECT_TRUE(ConstDeref<bool>(result));
}

TEST_F(RhoAdditionalTests2, IfElse_NestedConditions) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 8
result = 0
if x > 5
    if x < 10
        result = 1
    else
        result = 2
else
    result = 3
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(1, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Expression_UnaryMinus) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 7
result = -x
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(-7, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Function_NoArguments) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
answer = fun()
    42
result = answer()
result
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(RhoAdditionalTests2, Variable_Reassignment) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
x = 1
x = x + 1
x = x + 1
x
)";
    console_.Execute(code, Structure::Program);
    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));
}
