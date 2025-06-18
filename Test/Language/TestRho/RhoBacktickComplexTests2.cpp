#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// More complex backtick tests for Rho
struct TestRhoBacktickComplex2 : TestLangCommon {
    void RunRhoScript(const char* script) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        console_.Execute(script, Structure::Program);
        UnwrapStackValues();
    }

    template <class T>
    T GetTop() {
        EXPECT_FALSE(data_->Empty());
        return AtData<T>(0);
    }
};

// Test 11: Do-while with shell
TEST_F(TestRhoBacktickComplex2, DoWhileShell) {
    const char* script = R"(
sum = 0
i = 0
do
    sum = sum + `echo 5`
    i = i + 1
while i < `echo 4`
sum
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 20);
}

// Test 12: Shell in recursive functions
TEST_F(TestRhoBacktickComplex2, RecursiveShellFunction) {
    const char* script = R"(
fun fact(n)
    if n <= 1
        `echo 1`
    else
        n * fact(n - 1)
        
fact(`echo 5`)
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 120);
}

// Test 13: Complex nested loops with shell
TEST_F(TestRhoBacktickComplex2, NestedLoopsShell) {
    const char* script = R"(
sum = 0
i = 0
while i < `echo 2`
    j = 0
    while j < `echo 3`
        sum = sum + `echo 4`
        j = j + 1
    i = i + 1
sum
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 24);  // 2 * 3 * 4
}

// Test 14: Shell with string concatenation
TEST_F(TestRhoBacktickComplex2, ShellStringConcat) {
    const char* script = R"(
s1 = "`echo Hello`"
s2 = "`echo World`"
num = `echo 2024`
s1 + " " + s2 + " " + to_str(num)
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<String>(), "Hello World 2024");
}

// Test 15: Shell in switch-like pattern
TEST_F(TestRhoBacktickComplex2, ShellSwitchPattern) {
    const char* script = R"(
val = `echo 2`
result = 0
if val == 1
    result = `echo 10`
else if val == 2
    result = `echo 20`
else
    result = `echo 30`
result
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 20);
}

// Test 16: Shell with array map operations
TEST_F(TestRhoBacktickComplex2, ShellArrayMap) {
    const char* script = R"(
arr = [`echo 1`, `echo 2`, `echo 3`, `echo 4`]
sum = 0
i = 0
while i < 4
    sum = sum + arr[i] * `echo 2`
    i = i + 1
sum
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 20);  // (1+2+3+4) * 2
}

// Test 17: Complex expression with shell
TEST_F(TestRhoBacktickComplex2, ComplexExprShell) {
    const char* script = R"(
a = `echo 10`
b = `echo 5`
c = `echo 2`
((a + b) * c) - `echo 10`
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 20);  // (10+5)*2 - 10
}

// Test 18: Shell with function composition
TEST_F(TestRhoBacktickComplex2, FunctionCompositionShell) {
    const char* script = R"(
fun double(x)
    x * 2
    
fun add5(x)
    x + 5
    
fun shellAdd(x)
    x + `echo 10`
    
shellAdd(add5(double(`echo 3`)))
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 21);  // ((3*2)+5)+10
}

// Test 19: Shell with error handling
TEST_F(TestRhoBacktickComplex2, ShellErrorHandling) {
    const char* script = R"(
divisor = `echo 5`
dividend = `echo 20`

if divisor != 0
    dividend / divisor
else
    -1
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 4);
}

// Test 20: Ultimate shell test - Fibonacci
TEST_F(TestRhoBacktickComplex2, FibonacciShell) {
    const char* script = R"(
fun fib(n)
    if n <= 1
        n
    else
        fib(n - 1) + fib(n - 2)
        
fib(`echo 10`)
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 55);  // 10th Fibonacci number
}