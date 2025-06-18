#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Complex backtick tests for Rho language
struct TestRhoBacktickComplex : TestLangCommon {
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

// Test 1: Shell commands in arithmetic
TEST_F(TestRhoBacktickComplex, ShellArithmetic) {
    RunRhoScript("a = `echo 10` + `echo 20`; a");
    EXPECT_EQ(GetTop<int>(), 30);
}

// Test 2: Shell in conditional
TEST_F(TestRhoBacktickComplex, ShellConditional) {
    const char* script = R"(
if `echo 5` > 3
    result = `echo 100`
else
    result = `echo 0`
result
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 100);
}

// Test 3: Shell in loops
TEST_F(TestRhoBacktickComplex, ShellInLoop) {
    const char* script = R"(
sum = 0
i = 0
while i < `echo 3`
    sum = sum + `echo 10`
    i = i + 1
sum
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 30);
}

// Test 4: Shell with functions
TEST_F(TestRhoBacktickComplex, ShellWithFunction) {
    const char* script = R"(
fun add(x)
    x + `echo 50`
    
add(`echo 10`)
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 60);
}

// Test 5: Complex shell expressions
TEST_F(TestRhoBacktickComplex, ComplexShellExpr) {
    RunRhoScript("`echo 2` * (`echo 3` + `echo 4`)");
    EXPECT_EQ(GetTop<int>(), 14);
}

// Test 6: Shell with string operations
TEST_F(TestRhoBacktickComplex, ShellStringOps) {
    RunRhoScript("\"Value: \" + to_str(`echo 42`)");
    EXPECT_EQ(GetTop<String>(), "Value: 42");
}

// Test 7: Nested shell in functions
TEST_F(TestRhoBacktickComplex, NestedShellFunction) {
    const char* script = R"(
fun calc(x, y)
    x * `echo 2` + y * `echo 3`
    
calc(`echo 5`, `echo 4`)
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 22);  // 5*2 + 4*3
}

// Test 8: Shell in array operations
TEST_F(TestRhoBacktickComplex, ShellArrayOps) {
    const char* script = R"(
arr = [`echo 1`, `echo 2`, `echo 3`]
arr[`echo 1`]
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 2);
}

// Test 9: Shell with Pi blocks
TEST_F(TestRhoBacktickComplex, ShellWithPiBlock) {
    RunRhoScript("{ `echo 10` `echo 20` + }");
    EXPECT_EQ(GetTop<int>(), 30);
}

// Test 10: Complex control flow with shell
TEST_F(TestRhoBacktickComplex, ComplexControlFlow) {
    const char* script = R"(
x = `echo 15`
y = 0
if x > 10
    y = x * `echo 2`
else
    y = x + `echo 100`
y
)";
    RunRhoScript(script);
    EXPECT_EQ(GetTop<int>(), 30);
}