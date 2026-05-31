#include "TestLangCommon.h"

#include <limits>
#include <sstream>
#include <vector>

using namespace kai;
using namespace std;

class RhoBacktickShellTest : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
    }

    void RunRhoScript(const char* script) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        auto source = Dedent(script);
        console_.Execute(source.c_str(), Structure::Program);
        UnwrapStackValues();
    }

    static std::string Dedent(const char* script) {
        std::stringstream input(script);
        std::vector<std::string> lines;
        std::string line;
        size_t min_indent = std::numeric_limits<size_t>::max();

        while (std::getline(input, line)) {
            lines.push_back(line);
            if (line.find_first_not_of(" \t\r") == std::string::npos) {
                continue;
            }

            size_t indent = 0;
            while (indent < line.size() && line[indent] == ' ') {
                ++indent;
            }
            min_indent = std::min(min_indent, indent);
        }

        if (min_indent == std::numeric_limits<size_t>::max()) return "";

        std::string result;
        for (auto& current_line : lines) {
            if (current_line.size() >= min_indent) {
                result += current_line.substr(min_indent);
            } else {
                result += current_line;
            }
            result += '\n';
        }
        return result;
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
    const char* code = "sum = `echo 10` + `echo 20`; sum";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 30);
}

TEST_F(RhoBacktickShellTest, SubtractionWithShell) {
    const char* code = "diff = `echo 100` - `echo 25`; diff";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 75);
}

TEST_F(RhoBacktickShellTest, MultiplicationWithShell) {
    const char* code = "product = 5 * `echo 3`; product";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);
}

TEST_F(RhoBacktickShellTest, DivisionWithShell) {
    const char* code = "quotient = `echo 20` / `echo 4`; quotient";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 5);
}

// String operations with shell commands
TEST_F(RhoBacktickShellTest, StringConcatenation) {
    const char* code = "greeting = `echo HelloWorld`; greeting";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "HelloWorld");
}

TEST_F(RhoBacktickShellTest, StringFromShellCommand) {
    const char* code = "text = `echo test_string`; text";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "test_string");
}

// Complex expressions
TEST_F(RhoBacktickShellTest, NestedArithmetic) {
    const char* code = "result = (`echo 5` + `echo 3`) * `echo 2`; result";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 16);  // (5+3)*2
}

TEST_F(RhoBacktickShellTest, ModuloWithShell) {
    const char* code = "remainder = `echo 17` % `echo 5`; remainder";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 2);
}

// Boolean operations
TEST_F(RhoBacktickShellTest, BooleanComparison) {
    const char* code = "equal = `echo 10` == `echo 10`; equal";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

TEST_F(RhoBacktickShellTest, LessThanComparison) {
    const char* code = "less = `echo 5` < `echo 10`; less";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

TEST_F(RhoBacktickShellTest, GreaterThanComparison) {
    const char* code = "greater = `echo 20` > `echo 15`; greater";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<bool>(), true);
}

// Control structures with shell commands
TEST_F(RhoBacktickShellTest, IfWithShellCondition) {
    const char* code = R"(
x = 0
if `echo 5` > `echo 3`
    x = `echo 1`
else
    x = `echo 0`
x
)";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 1);
}

TEST_F(RhoBacktickShellTest, ForLoopWithShellLimit) {
    const char* code = R"(
sum = 0
i = 1
while i <= `echo 5`
    sum = sum + i
    i = i + 1
sum
)";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);  // 1+2+3+4+5
}

// Complex shell command outputs
TEST_F(RhoBacktickShellTest, PipelineCommand) {
    const char* code = "word_count = `printf 'one two three' | wc -w`; word_count";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 3);
}

TEST_F(RhoBacktickShellTest, FunctionWithShellParams) {
    const char* code = R"(
fun add_shell(x)
    x + `echo 10`

add_shell(`echo 5`)
)";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 15);
}

// Math expressions
TEST_F(RhoBacktickShellTest, MathExpressionEval) {
    const char* code = "calc = `echo $((2+3*4))`; calc";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 14);
}

TEST_F(RhoBacktickShellTest, ComplexCalculation) {
    const char* code =
        "result = (`echo 2` + `echo 3`) * `echo 4` + `echo 5`; result";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 25);  // ((2+3)*4)+5
}

// Array operations with shell commands
TEST_F(RhoBacktickShellTest, ArrayWithShellElements) {
    const char* code = R"(
arr = [`echo 1`, `echo 2`, `echo 3`]
sum = 0
i = 0
while i < 3
    sum = sum + arr[i]
    i = i + 1
sum
)";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<int>(), 6);  // 1+2+3
}

// Mixed operations
TEST_F(RhoBacktickShellTest, MixedStringAndNumber) {
    const char* code = "message = `echo 8_items`; message";
    RunRhoScript(code);
    EXPECT_EQ(GetTop<String>(), "8_items");
}
