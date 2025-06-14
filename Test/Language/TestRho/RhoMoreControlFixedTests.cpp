#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// More fixed control structure tests for Rho
struct RhoMoreControlFixedTests : TestLangCommon {
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

// Test break and continue in loops (from RhoControlStructuresTests.cpp)
TEST_F(RhoMoreControlFixedTests, BreakStatement) {
    RunAndExpect<int>(R"(
sum = 0
i = 1
while true
    if i > 5
        break
    sum = sum + i
    i = i + 1
sum
)",
                      15);

    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 10
    i = i + 1
    if i == 5
        break
    sum = sum + i
sum
)",
                      10);  // 1 + 2 + 3 + 4
}

TEST_F(RhoMoreControlFixedTests,
       ContinueStatement) {  // Continue is not a keyword in Rho
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 10
    i = i + 1
    if i % 2 == 1
        continue
    sum = sum + i
sum
)",
                      30);  // 2 + 4 + 6 + 8 + 10

    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 5
    i = i + 1
    if i == 3
        continue
    sum = sum + i
sum
)",
                      12);  // 1 + 2 + 4 + 5
}

// Test nested loops with break/continue
TEST_F(RhoMoreControlFixedTests,
       NestedLoopsWithBreakContinue) {  // Continue is not a keyword in Rho
    RunAndExpect<int>(
        R"(
sum = 0
i = 1
while i <= 3
    j = 1
    while j <= 3
        if j == 2
            j = j + 1
            continue
        sum = sum + (i * j)
        j = j + 1
    i = i + 1
sum
)",
        24);  // (1*1 + 1*3) + (2*1 + 2*3) + (3*1 + 3*3) = 4 + 8 + 12 = 24

    RunAndExpect<int>(R"(
count = 0
i = 0
while i < 5
    j = 0
    while j < 5
        if i + j > 5
            break
        count = count + 1
        j = j + 1
    i = i + 1
count
)",
                      19);  // Count pairs where i + j <= 5
}

// Test complex nested conditions
TEST_F(RhoMoreControlFixedTests, ComplexNestedConditions) {
    RunAndExpect<int>(R"(
result = 0
x = 10
y = 20
z = 30

if x < y
    if y < z
        if x < z
            result = 1
        else
            result = 2
    else
        result = 3
else
    result = 4
result
)",
                      1);

    RunAndExpect<int>(R"(
result = 0
a = true
b = false
c = true

if a
    if b || c
        if a && c
            result = 100
        else
            result = 50
    else
        result = 25
else
    result = 0
result
)",
                      100);
}

// Test multiple variable updates in loops
TEST_F(RhoMoreControlFixedTests, MultipleVariableLoops) {
    RunAndExpect<int>(R"(
sum = 0
product = 1
i = 1
while i <= 4
    sum = sum + i
    product = product * i
    i = i + 1
sum + product
)",
                      34);  // sum=10, product=24, total=34

    RunAndExpect<int>(R"(
a = 0
b = 1
count = 0
while count < 6
    temp = a + b
    a = b
    b = temp
    count = count + 1
b
)",
                      13);  // 6th Fibonacci number
}

// Test early returns with conditions
TEST_F(RhoMoreControlFixedTests, EarlyReturnPatterns) {
    RunAndExpect<int>(R"(
result = 0
x = 5
if x < 10
    result = x * 2
else
    result = x * 3
result
)",
                      10);  // Should return x * 2 = 10

    RunAndExpect<int>(R"(
found = false
i = 1
result = -1
while i <= 10
    if i * i == 25
        result = i
        found = true
        break
    i = i + 1
result
)",
                      5);
}

// Test conditional assignments
TEST_F(RhoMoreControlFixedTests, ConditionalAssignments) {
    RunAndExpect<int>(R"(
x = 10
y = 20
max = 0
if x > y
    max = x
else
    max = y
max
)",
                      20);

    RunAndExpect<int>(R"(
score = 85
grade = 0
if score >= 90
    grade = 4
else
    if score >= 80
        grade = 3
    else
        if score >= 70
            grade = 2
        else
            grade = 1
grade
)",
                      3);
}

// Test loop with multiple exit conditions
TEST_F(RhoMoreControlFixedTests, MultipleExitConditions) {
    RunAndExpect<int>(R"(
count = 0
sum = 0
while true
    count = count + 1
    sum = sum + count
    if count >= 10
        break
    if sum > 30
        break
count
)",
                      8);  // Loop exits when sum > 30

    RunAndExpect<int>(R"(
i = 0
j = 10
steps = 0
while i < j
    i = i + 1
    j = j - 1
    steps = steps + 1
    if i + j == 10
        break
steps
)",
                      1);
}