#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixed version of RhoControlTests that uses the base class console properly
struct RhoControlTestsProperlyFixed : TestLangCommon {
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

// Test nested if statements
TEST_F(RhoControlTestsProperlyFixed, NestedIfStatements) {
    RunAndExpect<int>(R"(
result = 0
if true
    if true
        result = 42
    else
        result = 24
else
    result = 10
result
)",
                      42);

    RunAndExpect<int>(R"(
result = 0
if true
    if false
        result = 42
    else
        result = 24
else
    result = 10
result
)",
                      24);

    RunAndExpect<int>(R"(
result = 0
if false
    if true
        result = 42
    else
        result = 24
else
    result = 10
result
)",
                      10);
}

// Test complex conditions (without parentheses)
TEST_F(RhoControlTestsProperlyFixed, ComplexConditions) {
    // Test logical AND
    RunAndExpect<int>(R"(
result = 0
a = 5 > 3
b = 10 < 20
if a && b
    result = 42
else
    result = 24
result
)",
                      42);

    // Test logical OR
    RunAndExpect<int>(R"(
result = 0
a = 5 < 3
b = 10 > 20
if a || b
    result = 42
else
    result = 24
result
)",
                      24);
}

// Test basic for loops
// DISABLED: Parser issue with for loops - only sees first statement
TEST_F(RhoControlTestsProperlyFixed, BasicForLoops) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 1; i <= 5; i = i + 1)
    sum = sum + i
sum
)",
                      15);

    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 2)
    sum = sum + i
sum
)",
                      20);
}

// Test basic while loops
TEST_F(RhoControlTestsProperlyFixed, BasicWhileLoops) {
    RunAndExpect<int>(R"(
sum = 0
i = 1
while i <= 5
    sum = sum + i
    i = i + 1
sum
)",
                      15);

    RunAndExpect<int>(R"(
sum = 0
i = 10
while i > 0
    sum = sum + i
    i = i - 2
sum
)",
                      30);
}

// Test nested for loops
// DISABLED: Parser issue with for loops
TEST_F(RhoControlTestsProperlyFixed, NestedForLoops) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 1; i <= 3; i = i + 1)
    for (j = 1; j <= 3; j = j + 1)
        sum = sum + (i * j)
sum
)",
                      36);
}

// Test nested while loops
TEST_F(RhoControlTestsProperlyFixed, NestedWhileLoops) {
    RunAndExpect<int>(R"(
sum = 0
i = 1
while i <= 3
    j = 1
    while j <= 3
        sum = sum + (i * j)
        j = j + 1
    i = i + 1
sum
)",
                      36);
}

// Test switch-like if-else-if chains
// DISABLED: Parser issue with else-if chains - "Statement expected"
TEST_F(RhoControlTestsProperlyFixed, SwitchLikeStatements) {
    RunAndExpect<int>(R"(
value = 2
result = 0
if value == 1
    result = 10
else if value == 2
    result = 20
else if value == 3
    result = 30
else
    result = 0
result
)",
                      20);

    RunAndExpect<int>(R"(
value = 5
result = 0
if value == 1
    result = 10
else if value == 2
    result = 20
else if value == 3
    result = 30
else
    result = 99
result
)",
                      99);
}

// Test do-while loops
TEST_F(RhoControlTestsProperlyFixed, DoWhileLoops) {
    RunAndExpect<int>(R"(
sum = 0
i = 1
do
    sum = sum + i
    i = i + 1
while i <= 5
sum
)",
                      15);

    // Test do-while that executes only once
    RunAndExpect<int>(R"(
sum = 0
i = 1
do
    sum = sum + i
    i = i + 1
while false
sum
)",
                      1);
}

// Test combined control structures
// DISABLED: Uses for loops which have parser issues
TEST_F(RhoControlTestsProperlyFixed, CombinedControlStructures) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 1; i <= 10; i = i + 1)
    if i % 2 == 0
        sum = sum + i
sum
)",
                      30);  // 2 + 4 + 6 + 8 + 10
}