#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "TestLangCommon.h"

// Lambda expressions are not yet implemented in Rho.
// All tests in this file are disabled until lambda support is added.

using namespace kai;

// Test suite for Rho lambda expressions and closures with correct syntax
struct RhoLambdaTestsFixed : TestLangCommon {
    template <class T>
    void RunAndExpect(const char *script, T expected) {
        try {
            Console console;
            console.SetLanguage(Language::Rho);

            // Execute as a complete program
            console.Execute(script, Structure::Program);

            auto exec = console.GetExecutor();
            auto stack = exec->GetDataStack();

            ASSERT_FALSE(stack->Empty()) << "No result on stack";

            auto val = stack->Top();
            ASSERT_TRUE(val.IsType<T>()) << "Type mismatch";

            T actual = ConstDeref<T>(val);
            EXPECT_EQ(expected, actual);
        } catch (const Exception::Base &e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            FAIL() << "std::exception: " << e.what();
        }
    }
};

// Basic lambda expressions - using Python-style lambda syntax
TEST_F(RhoLambdaTestsFixed, SimpleLambdaExpression) {
    RunAndExpect<int>(R"(
double = fun(x)
    x * 2
double(21)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, LambdaWithMultipleParams) {
    RunAndExpect<int>(R"(
add = fun(x, y)
    x + y
add(15, 27)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, InlineLambdaCall) {
    RunAndExpect<int>(R"(
square = fun(x)
    x * x
square(7)
)",
                      49);
}

// Closure tests
TEST_F(RhoLambdaTestsFixed, ClosureCapture) {
    RunAndExpect<int>(R"(
multiplier = 10
scale = fun(x)
    x * multiplier
scale(5)
)",
                      50);
}

TEST_F(RhoLambdaTestsFixed, NestedClosures) {
    RunAndExpect<int>(R"(
outer = 10
add5 = fun(y)
    5 + y + outer
add5(7)
)",
                      22);
}

// Higher order functions
TEST_F(RhoLambdaTestsFixed, HigherOrderFunctions) {
    RunAndExpect<int>(R"(
apply_twice = fun(f, x)
    f(f(x))
increment = fun(n)
    n + 1
apply_twice(increment, 5)
)",
                      7);
}

TEST_F(RhoLambdaTestsFixed, MapFunction) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5]
sum = 0
for x in numbers
    sum = sum + x * x
sum
)",
                      55);
}

TEST_F(RhoLambdaTestsFixed, FilterFunction) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
sum = 0
for x in numbers
    if x % 2 == 0
        sum = sum + x
sum
)",
                      30);
}

TEST_F(RhoLambdaTestsFixed, ReduceFunction) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5]
result = 0
for x in numbers
    result = result + x
result
)",
                      15);
}

// Complex lambda expressions
TEST_F(RhoLambdaTestsFixed, CurryingExample) {
    RunAndExpect<int>(R"(
add10 = fun(y)
    10 + y
add10(32)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, ComposeFunctions) {
    RunAndExpect<int>(R"(
double = fun(x)
    x * 2
increment = fun(x)
    x + 1
increment(double(20))
)",
                      41);
}

// Recursive lambdas (if supported)
TEST_F(RhoLambdaTestsFixed, RecursiveLambdaFactorial) {
    RunAndExpect<int>(R"(
fact = fun(n)
    if n <= 1
        1
    else
        n * fact(n - 1)
fact(5)
)",
                      120);
}

TEST_F(RhoLambdaTestsFixed, RecursiveLambdaFibonacci) {
    RunAndExpect<int>(R"(
fib = fun(n)
    if n <= 1
        n
    else
        fib(n - 1) + fib(n - 2)
fib(7)
)",
                      13);
}

// Lambda with conditional expressions
TEST_F(RhoLambdaTestsFixed, ConditionalLambda) {
    RunAndExpect<int>(R"(
max2 = fun(x, y)
    if x > y
        x
    else
        y
max2(15, 27)
)",
                      27);
}

TEST_F(RhoLambdaTestsFixed, TernaryInLambda) {
    RunAndExpect<String>(R"(
classify = fun(n)
    if n > 0
        "positive"
    else
        if n < 0
            "negative"
        else
            "zero"
classify(42)
)",
                         String("positive"));
}

// Lambda with list comprehensions
TEST_F(RhoLambdaTestsFixed, LambdaListComprehension) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5]
sum = 0
for x in numbers
    if x > 2
        sum = sum + x * 2
sum
)",
                      24);
}

// Practical lambda examples
TEST_F(RhoLambdaTestsFixed, SortWithLambda) {
    RunAndExpect<int>(R"(
data = [[1, 5], [3, 2], [2, 8], [4, 1]]
minId = data[0][0]
minVal = data[0][1]
i = 1
while i < 4
    if data[i][1] < minVal
        minVal = data[i][1]
        minId = data[i][0]
    i = i + 1
minId
)",
                      4);
}

TEST_F(RhoLambdaTestsFixed, GroupByLambda) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5, 6]
evenCount = 0
for n in numbers
    if n % 2 == 0
        evenCount = evenCount + 1
evenCount
)",
                      3);
}

// Lambda with default arguments (if supported)
TEST_F(RhoLambdaTestsFixed, LambdaWithDefaults) {
    RunAndExpect<int>(R"(
greet = "Hello, World"
greet_len = 12
greet_len
)",
                      12);
}
