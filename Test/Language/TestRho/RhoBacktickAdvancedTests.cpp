#include "TestLangCommon.h"

#include <limits>
#include <sstream>
#include <vector>

using namespace kai;
using namespace std;

class RhoBacktickAdvancedTest : public TestLangCommon {
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

// File system operations
TEST_F(RhoBacktickAdvancedTest, FileCount) {
    const char* code =
        "file_count = `ls -1 2>/dev/null | wc -l`; file_count >= 0";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, CurrentDirectory) {
    const char* code = "cwd = `pwd`; cwd != \"\"";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, DateProcessing) {
    const char* code = "year = `date +%Y`; year > 2000";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

// Environment variables
TEST_F(RhoBacktickAdvancedTest, HomeDirectory) {
    const char* code = "home = `echo $HOME`; home != \"\"";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, PathProcessing) {
    const char* code = "path_count = `echo $PATH | grep -c ':'`; path_count > 0";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

// Text processing with functions
TEST_F(RhoBacktickAdvancedTest, WordCountFunction) {
    const char* code = R"(
fun count_words()
    `printf 'hello world from rho' | wc -w`

count_words()
)";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 4);
}

TEST_F(RhoBacktickAdvancedTest, TextTransformation) {
    const char* code = "uppercase = `echo hello | tr '[:lower:]' '[:upper:]'`; uppercase";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(data_->Top()), "HELLO");
}

// Complex calculations
TEST_F(RhoBacktickAdvancedTest, ShellMathInLoop) {
    const char* code = R"(
sum = 0
i = 1
while i <= 3
    sum = sum + i * i * `echo 1`
    i = i + 1
sum
)";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 14);  // 1 + 4 + 9
}

TEST_F(RhoBacktickAdvancedTest, ConditionalWithShell) {
    const char* code = R"(
files = `ls -1 2>/dev/null | wc -l`
if files >= 0
    status = `echo 1`
else
    status = `echo 0`
status == 1
)";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

// Array operations with shell
TEST_F(RhoBacktickAdvancedTest, DynamicArraySize) {
    const char* code =
        "arr = [`echo 0`, `echo 2`, `echo 4`, `echo 6`, `echo 8`]; "
        "arr[`echo 4`]";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 8);
}

// String processing
TEST_F(RhoBacktickAdvancedTest, StringSplitting) {
    const char* code = "second_field = `echo 'one,two,three' | cut -d',' -f2`; second_field";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(data_->Top()), "two");
}

TEST_F(RhoBacktickAdvancedTest, RegexReplacement) {
    const char* code = "text = `echo 'hello world' | sed 's/world/universe/'`; text";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(data_->Top()), "hello universe");
}

// System information
TEST_F(RhoBacktickAdvancedTest, SystemInfo) {
    const char* code = "os_name = `uname -s`; os_name != \"\"";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, ProcessInfo) {
    const char* code = "process_count = `ps aux 2>/dev/null | wc -l`; process_count > 1";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

// Recursive function with shell
TEST_F(RhoBacktickAdvancedTest, RecursiveShellSum) {
    const char* code = R"(
fun shell_sum(n)
    if n <= 0
        return 0
    return n + shell_sum(n - `echo 1`)

shell_sum(3)
)";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 6);  // 3 + 2 + 1
}

// Complex pipeline
TEST_F(RhoBacktickAdvancedTest, ComplexPipeline) {
    const char* code = "result = `printf '5\\n2\\n8\\n1\\n9\\n' | sort -n | tail -3 | head -1`; result";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 5);
}

// While loop with shell
TEST_F(RhoBacktickAdvancedTest, WhileWithShell) {
    const char* code = R"(
counter = 0
limit = `echo 3`
while counter < limit
    counter = counter + 1
counter
)";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 3);
}

// JSON-like parsing
TEST_F(RhoBacktickAdvancedTest, SimpleJSONParse) {
    const char* code = "json_value = `echo '{\"count\": 42}' | grep -o '[0-9]\\+'`; json_value";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 42);
}

// Error handling
TEST_F(RhoBacktickAdvancedTest, CommandFailure) {
    const char* code = "output = `nonexistentcommand123 2>/dev/null`; output == \"\"";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(data_->Top()), true);
}

// Mixed operations
TEST_F(RhoBacktickAdvancedTest, MixedShellAndRho) {
    const char* code =
        "base = 10; multiplier = `echo 5`; offset = `echo $((2 + 3))`; "
        "base * multiplier + offset";

    RunRhoScript(code);
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 55);  // 10 * 5 + 5
}
