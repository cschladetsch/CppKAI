#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test to find a workaround for else-if chains
struct RhoElseIfWorkaround : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }

    template <typename T>
    void RunAndExpect(const string& code, T expected) {
        try {
            // Clear the data stack
            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();
            dataStack->Clear();

            // Execute as a complete program
            console_.Execute(code, Structure::Program);

            // Check result
            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution";

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null");

            T actual = ConstDeref<T>(result);
            ASSERT_EQ(actual, expected) << "Value mismatch";

        } catch (const Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what();
        }
    }
};

// Test nested if as workaround for else-if
TEST_F(RhoElseIfWorkaround, NestedIfInsteadOfElseIf) {
    RunAndExpect<int>(R"(
x = 2
if x == 1
    result = 10
else
    if x == 2
        result = 20
    else
        if x == 3
            result = 30
        else
            result = 40
result
)",
                      20);

    RunAndExpect<int>(R"(
x = 3
if x == 1
    result = 10
else
    if x == 2
        result = 20
    else
        if x == 3
            result = 30
        else
            result = 40
result
)",
                      30);
}

// Test single line if-else chains
// DISABLED: Single-line if-else with braces is not supported
TEST_F(RhoElseIfWorkaround, SingleLineIfElse) {
    // Try without line breaks - this syntax is not supported
    RunAndExpect<int>(
        "x = 2; if x == 1 { result = 10 } else { if x == 2 { result = 20 } "
        "else { result = 30 } }; result",
        20);
}

// Test using assignment in condition
TEST_F(RhoElseIfWorkaround, ConditionalAssignment) {
    RunAndExpect<int>(R"(
x = 2
result = 0
if x == 1
    result = 10
if x == 2
    result = 20
if x == 3
    result = 30
if result == 0
    result = 40
result
)",
                      20);
}