#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Comprehensive test suite for Rho iteration constructs
// 50 tests covering for, while, do-while, forEach patterns
// Purpose: Ensure iteration stability before any refactoring
struct RhoIterationComprehensiveTests : TestLangCommon {
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
                << "No result on stack after execution\nCode:\n"
                << code;

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

// ============================================================================
// WHILE LOOP TESTS (10 tests)
// ============================================================================

TEST_F(RhoIterationComprehensiveTests, WhileLoop_BasicCount) {
    RunAndExpect<int>(R"(
i = 0
while i < 10
    i = i + 1
i
)",
                      10);
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_Sum) {
    RunAndExpect<int>(R"(
i = 1
sum = 0
while i <= 5
    sum = sum + i
    i = i + 1
sum
)",
                      15);  // 1+2+3+4+5
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_NestedAccumulation) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 3
    j = 0
    while j < 4
        sum = sum + 1
        j = j + 1
    i = i + 1
sum
)",
                      12);  // 3 * 4
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_WithMultipleConditions) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 20
    i = i + 1
    if i % 2 == 0
        sum = sum + i
sum
)",
                      110);  // 2+4+6+8+10+12+14+16+18+20
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_CountDown) {
    RunAndExpect<int>(R"(
i = 10
sum = 0
while i > 0
    sum = sum + i
    i = i - 1
sum
)",
                      55);  // 10+9+8+...+1
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_StepByTwo) {
    RunAndExpect<int>(R"(
i = 0
count = 0
while i < 20
    count = count + 1
    i = i + 2
count
)",
                      10);
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_WithConditionalBreak) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 100
    i = i + 1
    sum = sum + i
    if sum > 50
        break
sum
)",
                      55);
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_WithContinue) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 10
    i = i + 1
    if i % 3 == 0
        continue
    sum = sum + i
sum
)",
                      37);  // 1+2+4+5+7+8+10 (skips 3,6,9)
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_DoubleNested) {
    RunAndExpect<int>(
        R"(
i = 0
product = 0
while i < 3
    i = i + 1
    j = 0
    while j < 4
        j = j + 1
        product = product + i * j
product
)",
        60);  // (1*1+1*2+1*3+1*4) + (2*1+2*2+2*3+2*4) + (3*1+3*2+3*3+3*4)
}

TEST_F(RhoIterationComprehensiveTests, WhileLoop_ComplexCondition) {
    RunAndExpect<int>(R"(
i = 0
j = 20
sum = 0
while i < 10
    sum = sum + i + j
    i = i + 1
    j = j - 1
sum
)",
                      200);  // Sum of (0+20, 1+19, ..., 9+11) = 10 * 20 = 200
}

// ============================================================================
// FOR LOOP TESTS (15 tests)
// ============================================================================

TEST_F(RhoIterationComprehensiveTests, ForLoop_BasicIncrement) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 1)
    sum = sum + i
sum
)",
                      45);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_StepByTwo) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 2)
    sum = sum + i
sum
)",
                      20);  // 0+2+4+6+8
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_Countdown) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 10; i > 0; i = i - 1)
    sum = sum + i
sum
)",
                      55);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_Nested) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 3; i = i + 1)
    for (j = 0; j < 4; j = j + 1)
        sum = sum + 1
sum
)",
                      12);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_NestedWithProduct) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 1; i <= 3; i = i + 1)
    for (j = 1; j <= 4; j = j + 1)
        sum = sum + i * j
sum
)",
                      60);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_WithConditional) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 20; i = i + 1)
    if i % 2 == 0
        sum = sum + i
sum
)",
                      90);  // 0+2+4+6+8+10+12+14+16+18
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_WithBreak) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 100; i = i + 1)
    sum = sum + i
    if sum > 50
        break
sum
)",
                      55);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_WithContinue) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 1)
    if i % 3 == 0
        continue
    sum = sum + i
sum
)",
                      27);  // 1+2+4+5+7+8 = 27 (skips 0,3,6,9 where i%3==0)
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_MultipleVariables) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 5; i = i + 1)
    for (j = 0; j < 5; j = j + 1)
        if i == j
            sum = sum + i
sum
)",
                      10);  // 0+1+2+3+4
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_LargeRange) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 100; i = i + 1)
    sum = sum + 1
sum
)",
                      100);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_StepByThree) {
    RunAndExpect<int>(R"(
count = 0
for (i = 0; i < 30; i = i + 3)
    count = count + 1
count
)",
                      10);
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_TripleNested) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 2; i = i + 1)
    for (j = 0; j < 3; j = j + 1)
        for (k = 0; k < 4; k = k + 1)
            sum = sum + 1
sum
)",
                      24);  // 2*3*4
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_WithAccumulator) {
    RunAndExpect<int>(R"(
product = 1
for (i = 1; i <= 5; i = i + 1)
    product = product * i
product
)",
                      120);  // 5!
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_StartNonZero) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 5; i < 15; i = i + 1)
    sum = sum + i
sum
)",
                      95);  // 5+6+7+...+14
}

TEST_F(RhoIterationComprehensiveTests, ForLoop_NegativeNumbers) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0 - 5; i < 5; i = i + 1)
    sum = sum + i
sum
)",
                      -5);  // -5+(-4)+(-3)+(-2)+(-1)+0+1+2+3+4 = -5
}

// ============================================================================
// DO-WHILE LOOP TESTS (10 tests)
// ============================================================================

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_BasicCount) {
    RunAndExpect<int>(R"(
i = 0
do
    i = i + 1
while i < 10
i
)",
                      10);
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_Sum) {
    RunAndExpect<int>(R"(
i = 1
sum = 0
do
    sum = sum + i
    i = i + 1
while i <= 5
sum
)",
                      15);  // 1+2+3+4+5
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_RunsAtLeastOnce) {
    RunAndExpect<int>(R"(
i = 100
count = 0
do
    count = count + 1
    i = i + 1
while i < 50
count
)",
                      1);  // Executes once even though condition is false
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_WithBreak) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    sum = sum + i
    i = i + 1
    if sum > 20
        break
while i < 100
sum
)",
                      21);
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_WithContinue) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    i = i + 1
    if i % 2 == 0
        continue
    sum = sum + i
while i < 10
sum
)",
                      25);  // 1+3+5+7+9
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_Nested) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    j = 0
    do
        sum = sum + 1
        j = j + 1
    while j < 3
    i = i + 1
while i < 4
sum
)",
                      12);  // 4 * 3
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_Countdown) {
    RunAndExpect<int>(R"(
i = 10
sum = 0
do
    sum = sum + i
    i = i - 1
while i > 0
sum
)",
                      55);
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_ComplexCondition) {
    RunAndExpect<int>(R"(
i = 0
j = 10
sum = 0
do
    sum = sum + i + j
    i = i + 1
    j = j - 1
while i < 5
sum
)",
                      50);  // (0+10) + (1+9) + (2+8) + (3+7) + (4+6)
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_WithModulo) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    i = i + 1
    if i % 3 != 0
        sum = sum + i
while i < 15
sum
)",
                      75);  // Sum of 1-15 except multiples of 3
}

TEST_F(RhoIterationComprehensiveTests, DoWhileLoop_MultipleBreakConditions) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    i = i + 1
    sum = sum + i
    if i > 10
        break
    if sum > 30
        break
while i < 100
sum
)",
                      36);  // 1+2+3+4+5+6+7+8 = 36
}

// ============================================================================
// MIXED LOOP PATTERNS (15 tests)
// ============================================================================

TEST_F(RhoIterationComprehensiveTests, Mixed_ForInsideWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 3
    for (j = 0; j < 4; j = j + 1)
        sum = sum + 1
    i = i + 1
sum
)",
                      12);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_WhileInsideFor) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 3; i = i + 1)
    j = 0
    while j < 4
        sum = sum + 1
        j = j + 1
sum
)",
                      12);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_DoWhileInsideFor) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 3; i = i + 1)
    j = 0
    do
        sum = sum + 1
        j = j + 1
    while j < 4
sum
)",
                      12);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_ForInsideDoWhile) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    for (j = 0; j < 4; j = j + 1)
        sum = sum + 1
    i = i + 1
while i < 3
sum
)",
                      12);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_AllThreeLoopsNested) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 2
    for (j = 0; j < 2; j = j + 1)
        k = 0
        do
            sum = sum + 1
            k = k + 1
        while k < 2
    i = i + 1
sum
)",
                      8);  // 2 * 2 * 2
}

TEST_F(RhoIterationComprehensiveTests, Mixed_SequentialLoops) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 5; i = i + 1)
    sum = sum + i
i = 0
while i < 5
    sum = sum + i
    i = i + 1
i = 0
do
    sum = sum + i
    i = i + 1
while i < 5
sum
)",
                      30);  // 10 + 10 + 10
}

TEST_F(RhoIterationComprehensiveTests, Mixed_BreakInNestedLoop) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 1)
    j = 0
    while j < 10
        sum = sum + 1
        j = j + 1
        if sum > 25
            break
    if sum > 25
        break
sum
)",
                      26);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_ContinueInNestedLoop) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 5; i = i + 1)
    j = 0
    while j < 5
        j = j + 1
        if j % 2 == 0
            continue
        sum = sum + 1
sum
)",
                      15);  // 5 iterations * 3 odd numbers
}

TEST_F(RhoIterationComprehensiveTests, Mixed_ConditionalInsideLoops) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 10; i = i + 1)
    if i % 2 == 0
        j = 0
        while j < i
            sum = sum + 1
            j = j + 1
sum
)",
                      20);  // 0+2+4+6+8
}

TEST_F(RhoIterationComprehensiveTests, Mixed_LoopWithMultipleAccumulators) {
    RunAndExpect<int>(R"(
sum1 = 0
sum2 = 0
for (i = 0; i < 10; i = i + 1)
    if i % 2 == 0
        sum1 = sum1 + i
    else
        sum2 = sum2 + i
sum1 + sum2
)",
                      45);
}

TEST_F(RhoIterationComprehensiveTests, Mixed_ComplexControlFlow) {
    RunAndExpect<int>(R"(
result = 0
for (i = 0; i < 5; i = i + 1)
    j = 0
    while j < 5
        if i == j
            result = result + i
        j = j + 1
result
)",
                      10);  // 0+1+2+3+4
}

TEST_F(RhoIterationComprehensiveTests, Mixed_NestedBreaksAndContinues) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 10
    i = i + 1
    if i % 2 == 0
        continue
    for (j = 0; j < i; j = j + 1)
        if j > 3
            break
        sum = sum + 1
sum
)",
                      16);  // Odd i values: 1,3,5,7,9. Each adds min(i,4)
                            // elements: 1+3+4+4+4=16
}

TEST_F(RhoIterationComprehensiveTests, Mixed_DeepNesting) {
    RunAndExpect<int>(R"(
sum = 0
for (i = 0; i < 2; i = i + 1)
    j = 0
    while j < 2
        k = 0
        do
            for (m = 0; m < 2; m = m + 1)
                sum = sum + 1
            k = k + 1
        while k < 2
        j = j + 1
sum
)",
                      16);  // 2*2*2*2
}

TEST_F(RhoIterationComprehensiveTests, Mixed_LoopsWithFunctions) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
sum = 0
for (i = 0; i < 5; i = i + 1)
    sum = sum + double(i)
sum
)",
                      20);  // 0+2+4+6+8
}

TEST_F(RhoIterationComprehensiveTests, Mixed_EarlyReturnPattern) {
    RunAndExpect<int>(R"(
fun findSum(limit) {
    sum = 0
    for (i = 0; i < 100; i = i + 1)
        sum = sum + i
        if sum > limit
            break
    sum
}
findSum(50)
)",
                      55);
}
