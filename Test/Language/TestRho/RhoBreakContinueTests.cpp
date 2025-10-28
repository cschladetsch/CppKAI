#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Comprehensive tests for Rho break and continue statements
struct RhoBreakContinueTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }

    template <typename T>
    void RunAndExpect(const string& code, T expected) {
        try {
            console_.SetLanguage(Language::Rho);
            console_.Execute(code, Structure::Program);

            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();

            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution\nCode:\n" << code;

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null");

            T actual = ConstDeref<T>(result);
            ASSERT_EQ(actual, expected) << "Value mismatch\nCode:\n" << code;

        } catch (const Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString() << "\nCode:\n" << code;
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what() << "\nCode:\n" << code;
        }
    }
};

// Test 1: Simple break in while loop
TEST_F(RhoBreakContinueTests, SimpleBreakInWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 10
    if i == 5
        break
    sum = sum + i
    i = i + 1
sum
)", 10);  // 0 + 1 + 2 + 3 + 4 = 10
}

// Test 2: Simple continue in while loop
TEST_F(RhoBreakContinueTests, SimpleContinueInWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 10
    i = i + 1
    if i % 2 == 0
        continue
    sum = sum + i
sum
)", 25);  // 1 + 3 + 5 + 7 + 9 = 25
}

// Test 3: Break in for loop
TEST_F(RhoBreakContinueTests, BreakInForLoop) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 20; i = i + 1)
    if i == 7
        break
    sum = sum + i
sum
)", 21);  // 0 + 1 + 2 + 3 + 4 + 5 + 6 = 21
}

// Test 4: Continue in for loop
TEST_F(RhoBreakContinueTests, ContinueInForLoop) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 1)
    if i < 5
        continue
    sum = sum + i
sum
)", 35);  // 5 + 6 + 7 + 8 + 9 = 35
}

// Test 5: Break immediately exits loop
TEST_F(RhoBreakContinueTests, BreakExitsImmediately) {
    RunAndExpect<int>(R"(
count = 0
i = 0
while true
    count = count + 1
    if count == 3
        break
    i = i + 1
count
)", 3);
}

// Test 6: Continue skips rest of iteration
TEST_F(RhoBreakContinueTests, ContinueSkipsRestOfIteration) {
    RunAndExpect<int>(R"(
count = 0
i = 0
while i < 5
    i = i + 1
    if i == 3
        continue
    count = count + 1
count
)", 4);  // Increments for i=1,2,4,5 but not i=3
}

// Test 7: Multiple breaks in nested conditions
TEST_F(RhoBreakContinueTests, MultipleBreaksInConditions) {
    RunAndExpect<int>(R"(
i = 0
result = 0
while i < 100
    i = i + 1
    if i > 10
        break
    if i % 2 == 0
        result = result + 10
        if result >= 30
            break
result
)", 30);
}

// Test 8: Multiple continues with different conditions
TEST_F(RhoBreakContinueTests, MultipleContinuesWithConditions) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 20
    i = i + 1
    if i % 2 == 0
        continue
    if i % 3 == 0
        continue
    sum = sum + i
sum
)", 55);  // Sum of numbers 1-19 that are neither divisible by 2 nor 3
}

// Test 9: Break with value accumulation before break
TEST_F(RhoBreakContinueTests, BreakWithAccumulation) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 100
    sum = sum + i
    i = i + 1
    if sum > 50
        break
sum
)", 55);  // 0+1+2+...+10 = 55 (first sum > 50)
}

// Test 10: Continue in loop with multiple increments
TEST_F(RhoBreakContinueTests, ContinueWithMultipleIncrements) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 10
    i = i + 1
    if i <= 5
        continue
    sum = sum + i
    i = i + 1
sum
)", 24);  // 6 + 8 + 10 = 24 (skips 1-5, processes 6,8,10)
}

// Test 11: Break in do-while loop
TEST_F(RhoBreakContinueTests, BreakInDoWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    sum = sum + i
    i = i + 1
    if i == 5
        break
while i < 10
sum
)", 10);  // 0 + 1 + 2 + 3 + 4 = 10
}

// Test 12: Continue in do-while loop
TEST_F(RhoBreakContinueTests, ContinueInDoWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
count = 0
do
    count = count + 1
    i = i + 1
    if i % 2 == 0
        continue
    sum = sum + i
while i < 7
sum
)", 16);  // 1 + 3 + 5 + 7 = 16
}
