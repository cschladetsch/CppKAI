#include "TestLangCommon.h"

using namespace kai;
using namespace std;

class RhoBacktickAdvancedTest : public TestLangCommon {
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

// File system operations
TEST_F(RhoBacktickAdvancedTest, FileCount) {
    const char* code = R"(
        file_count = `ls -1 2>/dev/null | wc -l`
        has_files = file_count > 0
        has_files
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, CurrentDirectory) {
    const char* code = R"(
        cwd = `pwd`
        is_valid = cwd != ""
        is_valid
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, DateProcessing) {
    const char* code = R"(
        year = to_int(`date +%Y`)
        is_future = year > 2000
        is_future
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Environment variables
TEST_F(RhoBacktickAdvancedTest, HomeDirectory) {
    const char* code = R"(
        home = `echo $HOME`
        has_home = home != ""
        has_home
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, PathProcessing) {
    const char* code = R"(
        path_count = `echo $PATH | grep -c ':'`
        has_paths = path_count > 0
        has_paths
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Text processing with functions
TEST_F(RhoBacktickAdvancedTest, WordCountFunction) {
    const char* code = R"(
        fun count_words(text) {
            return `echo "$text" | wc -w`
        }
        
        count = count_words("hello world from rho")
        count
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);
}

TEST_F(RhoBacktickAdvancedTest, TextTransformation) {
    const char* code = R"(
        original = "hello"
        uppercase = `echo "$original" | tr '[:lower:]' '[:upper:]'`
        uppercase
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "HELLO");
}

// Complex calculations
TEST_F(RhoBacktickAdvancedTest, ShellMathInLoop) {
    const char* code = R"(
        sum = 0
        for i = 1; i <= 3; i = i + 1
            sum = sum + `echo $((i * i))`
        sum
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 14);  // 1 + 4 + 9
}

TEST_F(RhoBacktickAdvancedTest, ConditionalWithShell) {
    const char* code = R"(
        files = `ls -1 2>/dev/null | wc -l`
        
        if (files > 10) {
            status = "many files"
        } else {
            status = "few files"
        }
        
        status != ""
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Array operations with shell
TEST_F(RhoBacktickAdvancedTest, DynamicArraySize) {
    const char* code = R"(
        size = `echo 5`
        arr = []
        
        for i = 0; i < size; i = i + 1
            arr = arr + [i * 2]
        
        arr[4]
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

// String processing
TEST_F(RhoBacktickAdvancedTest, StringSplitting) {
    const char* code = R"(
        second_field = `echo 'one,two,three' | cut -d',' -f2`
        second_field
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "two");
}

TEST_F(RhoBacktickAdvancedTest, RegexReplacement) {
    const char* code = R"(
        text = `echo 'hello world' | sed 's/world/universe/'`
        text
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "hello universe");
}

// System information
TEST_F(RhoBacktickAdvancedTest, SystemInfo) {
    const char* code = R"(
        os_name = `uname -s`
        is_valid_os = os_name != ""
        is_valid_os
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(RhoBacktickAdvancedTest, ProcessInfo) {
    const char* code = R"(
        process_count = `ps aux 2>/dev/null | wc -l`
        has_processes = process_count > 1
        has_processes
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Recursive function with shell
TEST_F(RhoBacktickAdvancedTest, RecursiveShellSum) {
    const char* code = R"(
        fun shell_sum(n) {
            if (n <= 0) {
                return 0
            }
            return `echo $n` + shell_sum(n - 1)
        }
        
        result = shell_sum(3)
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 3 + 2 + 1
}

// Complex pipeline
TEST_F(RhoBacktickAdvancedTest, ComplexPipeline) {
    const char* code = R"(
        result = `echo -e '5\n2\n8\n1\n9' | sort -n | tail -3 | head -1`
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

// While loop with shell
TEST_F(RhoBacktickAdvancedTest, WhileWithShell) {
    const char* code = R"(
        counter = 0
        limit = `echo 3`
        
        while (counter < limit) {
            counter = counter + 1
        }
        
        counter
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

// JSON-like parsing
TEST_F(RhoBacktickAdvancedTest, SimpleJSONParse) {
    const char* code = R"(
        json_value = `echo '{"count": 42}' | grep -o '[0-9]\+'`
        json_value
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

// Error handling
TEST_F(RhoBacktickAdvancedTest, CommandFailure) {
    const char* code = R"(
        output = `nonexistentcommand123 2>/dev/null`
        is_empty = output == ""
        is_empty
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Mixed operations
TEST_F(RhoBacktickAdvancedTest, MixedShellAndRho) {
    const char* code = R"(
        base = 10
        multiplier = `echo 5`
        offset = `echo $((2 + 3))`
        
        result = base * multiplier + offset
        result
    )";

    console_.Execute(code);
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 55);  // 10 * 5 + 5
}