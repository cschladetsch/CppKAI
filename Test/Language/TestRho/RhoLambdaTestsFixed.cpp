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
double = lambda x: x * 2
double(21)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, LambdaWithMultipleParams) {
    RunAndExpect<int>(R"(
add = lambda x, y: x + y
add(15, 27)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, InlineLambdaCall) {
    RunAndExpect<int>(R"(
(lambda x: x * x)(7)
)",
                      49);
}

// Closure tests
TEST_F(RhoLambdaTestsFixed, ClosureCapture) {
    RunAndExpect<int>(R"(
multiplier = 10
scale = lambda x: x * multiplier
scale(5)
)",
                      50);
}

TEST_F(RhoLambdaTestsFixed, NestedClosures) {
    RunAndExpect<int>(R"(
outer = 10
make_adder = lambda x: lambda y: x + y + outer
add5 = make_adder(5)
add5(7)
)",
                      22);
}

// Higher order functions
TEST_F(RhoLambdaTestsFixed, HigherOrderFunctions) {
    RunAndExpect<int>(R"(
apply_twice = lambda f, x: f(f(x))
increment = lambda n: n + 1
apply_twice(increment, 5)
)",
                      7);
}

TEST_F(RhoLambdaTestsFixed, MapFunction) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5]
map = lambda f, lst: [f(x) for x in lst]
squared = map(lambda x: x * x, numbers)
sum(squared)
)",
                      55);
}

TEST_F(RhoLambdaTestsFixed, FilterFunction) {
    RunAndExpect<int>(R"(
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
filter = lambda pred, lst: [x for x in lst if pred(x)]
evens = filter(lambda x: x % 2 == 0, numbers)
sum(evens)
)",
                      30);
}

TEST_F(RhoLambdaTestsFixed, ReduceFunction) {
    RunAndExpect<int>(R"(
reduce = lambda f, lst, init: 
    result = init
    for x in lst
        result = f(result, x)
    result

numbers = [1, 2, 3, 4, 5]
reduce(lambda acc, x: acc + x, numbers, 0)
)",
                      15);
}

// Complex lambda expressions
TEST_F(RhoLambdaTestsFixed, CurryingExample) {
    RunAndExpect<int>(R"(
curry = lambda f: lambda x: lambda y: f(x, y)
add = lambda x, y: x + y
curried_add = curry(add)
add10 = curried_add(10)
add10(32)
)",
                      42);
}

TEST_F(RhoLambdaTestsFixed, ComposeFunctions) {
    RunAndExpect<int>(R"(
compose = lambda f, g: lambda x: f(g(x))
double = lambda x: x * 2
increment = lambda x: x + 1
double_then_increment = compose(increment, double)
double_then_increment(20)
)",
                      41);
}

// Recursive lambdas (if supported)
TEST_F(RhoLambdaTestsFixed, RecursiveLambdaFactorial) {
    RunAndExpect<int>(R"(
fact = lambda n: 1 if n <= 1 else n * fact(n - 1)
fact(5)
)",
                      120);
}

TEST_F(RhoLambdaTestsFixed, RecursiveLambdaFibonacci) {
    RunAndExpect<int>(R"(
fib = lambda n: n if n <= 1 else fib(n - 1) + fib(n - 2)
fib(7)
)",
                      13);
}

// Lambda with conditional expressions
TEST_F(RhoLambdaTestsFixed, ConditionalLambda) {
    RunAndExpect<int>(R"(
max = lambda x, y: x if x > y else y
max(15, 27)
)",
                      27);
}

TEST_F(RhoLambdaTestsFixed, TernaryInLambda) {
    RunAndExpect<String>(R"(
classify = lambda n: "positive" if n > 0 else ("negative" if n < 0 else "zero")
classify(42)
)",
                         String("positive"));
}

// Lambda with list comprehensions
TEST_F(RhoLambdaTestsFixed, LambdaListComprehension) {
    RunAndExpect<int>(R"(
transform = lambda lst: [x * 2 for x in lst if x > 2]
numbers = [1, 2, 3, 4, 5]
result = transform(numbers)
sum(result)
)",
                      24);
}

// Practical lambda examples
TEST_F(RhoLambdaTestsFixed, SortWithLambda) {
    RunAndExpect<int>(R"(
data = [(1, 5), (3, 2), (2, 8), (4, 1)]
sorted_data = sorted(data, key=lambda x: x[1])
sorted_data[0][0]
)",
                      4);
}

TEST_F(RhoLambdaTestsFixed, GroupByLambda) {
    RunAndExpect<int>(R"(
group_by = lambda lst, key_fn:
    groups = {}
    for item in lst
        key = key_fn(item)
        if key not in groups
            groups[key] = []
        groups[key].append(item)
    groups

numbers = [1, 2, 3, 4, 5, 6]
by_parity = group_by(numbers, lambda x: x % 2)
len(by_parity[0])
)",
                      3);
}

// Lambda with default arguments (if supported)
TEST_F(RhoLambdaTestsFixed, LambdaWithDefaults) {
    RunAndExpect<int>(R"(
greet = lambda name="World": "Hello, " + name
len(greet())
)",
                      12);
}