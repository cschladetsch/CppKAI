#include "TestLangCommon.h"

using namespace kai;
using namespace std;

class RhoBacktickShellTest : public TestLangCommon {
   protected:
    void RunRhoScript(const char* script) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        console_.Execute(script);
        UnwrapStackValues();
    }

    template <class T>
    T GetTop() {
        EXPECT_FALSE(data_->Empty());
        return AtData<T>(0);
    }
};

// Basic arithmetic with shell commands
TEST_F(RhoBacktickShellTest, SimpleAddition) {
    RunRhoScript("result = 1 + `echo 2`; result");
    EXPECT_EQ(GetTop<int>(), 3);
}

TEST_F(RhoBacktickShellTest, MultipleShellCommands) {
    const char* code = R"(
        sum = `echo 10` + `echo 20`;
        sum
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 30);
}

TEST_F(RhoBacktickShellTest, SubtractionWithShell) {
    const char* code = R"(
        diff = `echo 100` - `echo 25`;
        diff
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 75);
}

TEST_F(RhoBacktickShellTest, MultiplicationWithShell) {
    const char* code = R"(
        product = 5 * `echo 3`;
        product
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);
}

TEST_F(RhoBacktickShellTest, DivisionWithShell) {
    const char* code = R"(
        quotient = `echo 20` / `echo 4`;
        quotient
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 5);
}

// String operations with shell commands
TEST_F(RhoBacktickShellTest, StringConcatenation) {
    const char* code = R"(
        greeting = "Hello " + `echo World`;
        greeting
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "Hello World");
}

TEST_F(RhoBacktickShellTest, StringFromShellCommand) {
    const char* code = R"(
        text = `echo "test string"`;
        text
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "test string");
}

// Complex expressions
TEST_F(RhoBacktickShellTest, NestedArithmetic) {
    const char* code = R"(
        result = (`echo 5` + `echo 3`) * `echo 2`;
        result
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 16);  // (5+3)*2
}

TEST_F(RhoBacktickShellTest, ModuloWithShell) {
    const char* code = R"(
        remainder = `echo 17` % `echo 5`;
        remainder
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 2);
}

// Boolean operations
TEST_F(RhoBacktickShellTest, BooleanComparison) {
    const char* code = R"(
        equal = `echo 10` == `echo 10`;
        equal
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

TEST_F(RhoBacktickShellTest, LessThanComparison) {
    const char* code = R"(
        less = `echo 5` < `echo 10`;
        less
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

TEST_F(RhoBacktickShellTest, GreaterThanComparison) {
    const char* code = R"(
        greater = `echo 20` > `echo 15`;
        greater
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

// Control structures with shell commands
TEST_F(RhoBacktickShellTest, IfWithShellCondition) {
    const char* code = R"(
        result = "";
        if (`echo 5` > `echo 3`) {
            result = "yes"
        } else {
            result = "no"
        };
        result
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "yes");
}

TEST_F(RhoBacktickShellTest, ForLoopWithShellLimit) {
    const char* code = R"(
        sum = 0;
        for (i = 1; i <= `echo 5`; i = i + 1) {
            sum = sum + i
        };
        sum
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);  // 1+2+3+4+5
}

// Complex shell command outputs
TEST_F(RhoBacktickShellTest, PipelineCommand) {
    const char* code = R"(
        word_count = `echo "one two three" | wc -w`;
        word_count
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 3);
}

TEST_F(RhoBacktickShellTest, FunctionWithShellParams) {
    const char* code = R"(
        fun add_shell(x) {
            x + `echo 10`
        };
        result = add_shell(`echo 5`);
        result
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);
}

// Math expressions
TEST_F(RhoBacktickShellTest, MathExpressionEval) {
    const char* code = R"(
        calc = `echo $((2+3*4))`;
        calc
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 14);
}

TEST_F(RhoBacktickShellTest, ComplexCalculation) {
    const char* code = R"(
        result = (`echo 2` + `echo 3`) * `echo 4` + `echo 5`;
        result
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 25);  // ((2+3)*4)+5
}

// Array operations with shell commands
TEST_F(RhoBacktickShellTest, ArrayWithShellElements) {
    const char* code = R"(
        arr = [`echo 1`, `echo 2`, `echo 3`];
        sum = 0;
        i = 0;
        while (i < 3) {
            sum = sum + arr[i];
            i = i + 1
        };
        sum
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 6);  // 1+2+3
}

// Mixed operations
TEST_F(RhoBacktickShellTest, MixedStringAndNumber) {
    const char* code = R"(
        count = `echo 5` + `echo 3`;
        message = to_str(count) + " items";
        message
    )";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "8 items");
}