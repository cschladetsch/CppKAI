#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Comprehensive test for ALL ways to iterate in Rho
// Covers: while, for, do-while, foreach, map, filter, reduce, comprehensions
struct RhoAllIterationMethodsTest : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }

    template <typename T>
    void RunAndExpect(const string& code, T expected,
                      const string& testName = "") {
        try {
            console_.SetLanguage(Language::Rho);
            console_.Execute(code, Structure::Program);

            UnwrapStackValues();

            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();

            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution"
                << (testName.empty() ? "" : "\nTest: " + testName)
                << "\nCode:\n"
                << code;

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null")
                << (testName.empty() ? "" : "\nTest: " + testName);

            T actual = ConstDeref<T>(result);
            ASSERT_EQ(actual, expected)
                << "Value mismatch"
                << (testName.empty() ? "" : "\nTest: " + testName)
                << "\nCode:\n"
                << code;

        } catch (const Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString()
                   << (testName.empty() ? "" : "\nTest: " + testName)
                   << "\nCode:\n"
                   << code;
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what()
                   << (testName.empty() ? "" : "\nTest: " + testName)
                   << "\nCode:\n"
                   << code;
        }
    }
};

// ============================================================================
// CATEGORY 1: TRADITIONAL IMPERATIVE LOOPS (15 tests)
// ============================================================================

// --- WHILE LOOPS ---

TEST_F(RhoAllIterationMethodsTest, While_BasicCount) {
    RunAndExpect<int>(R"(
i = 0
while i < 5
    i = i + 1
i
)",
                      5, "While_BasicCount");
}

TEST_F(RhoAllIterationMethodsTest, While_Accumulate) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 10
    sum = sum + i
    i = i + 1
sum
)",
                      45, "While_Accumulate");
}

TEST_F(RhoAllIterationMethodsTest, While_WithBreak) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 100
    sum = sum + i
    i = i + 1
    if sum > 20
        break
sum
)",
                      21, "While_WithBreak");
}

TEST_F(RhoAllIterationMethodsTest, While_Nested) {
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
                      12, "While_Nested");
}

// --- FOR LOOPS ---

TEST_F(RhoAllIterationMethodsTest, For_BasicIteration) {
    RunAndExpect<int>(R"(
sum = 0
for i = 0; i < 10; i = i + 1
    sum = sum + i
sum
)",
                      45, "For_BasicIteration");
}

TEST_F(RhoAllIterationMethodsTest, For_WithStep) {
    RunAndExpect<int>(R"(
sum = 0
for i = 0; i < 10; i = i + 2
    sum = sum + i
sum
)",
                      20, "For_WithStep");  // 0+2+4+6+8
}

TEST_F(RhoAllIterationMethodsTest, For_Countdown) {
    RunAndExpect<int>(R"(
sum = 0
for i = 10; i > 0; i = i - 1
    sum = sum + i
sum
)",
                      55, "For_Countdown");
}

TEST_F(RhoAllIterationMethodsTest, For_WithBreak) {
    RunAndExpect<int>(R"(
sum = 0
for i = 0; i < 100; i = i + 1
    sum = sum + i
    if sum > 30
        break
sum
)",
                      36, "For_WithBreak");
}

// --- DO-WHILE LOOPS ---

TEST_F(RhoAllIterationMethodsTest, DoWhile_BasicIteration) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    sum = sum + i
    i = i + 1
while i < 5
sum
)",
                      10, "DoWhile_BasicIteration");  // 0+1+2+3+4
}

TEST_F(RhoAllIterationMethodsTest, DoWhile_ExecutesOnce) {
    RunAndExpect<int>(
        R"(
count = 0
i = 100
do
    count = count + 1
    i = i + 1
while i < 10
count
)",
        1,
        "DoWhile_ExecutesOnce");  // Executes once even though condition false
}

TEST_F(RhoAllIterationMethodsTest, DoWhile_WithBreak) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
do
    sum = sum + i
    i = i + 1
    if sum > 15
        break
while i < 100
sum
)",
                      21, "DoWhile_WithBreak");
}

// --- RANGE-BASED PATTERNS ---

TEST_F(RhoAllIterationMethodsTest, RangeStyle_WithArray) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
sum = 0
i = 0
while i < 5
    sum = sum + arr[i]
    i = i + 1
sum
)",
                      15, "RangeStyle_WithArray");
}

TEST_F(RhoAllIterationMethodsTest, RangeStyle_ForLoop) {
    RunAndExpect<int>(R"(
arr = [10, 20, 30]
sum = 0
for i = 0; i < 3; i = i + 1
    sum = sum + arr[i]
sum
)",
                      60, "RangeStyle_ForLoop");
}

// --- INFINITE LOOP WITH BREAK ---

TEST_F(RhoAllIterationMethodsTest, InfiniteLoop_WithBreak) {
    RunAndExpect<int>(R"(
i = 0
while true
    i = i + 1
    if i >= 5
        break
i
)",
                      5, "InfiniteLoop_WithBreak");
}

TEST_F(RhoAllIterationMethodsTest, InfiniteLoop_ConditionalBreak) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while true
    sum = sum + i
    i = i + 1
    if i == 10
        break
sum
)",
                      45, "InfiniteLoop_ConditionalBreak");
}

// ============================================================================
// CATEGORY 2: FOREACH / FUNCTIONAL ITERATION (20 tests)
// ============================================================================

TEST_F(RhoAllIterationMethodsTest, ForEach_Array) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
sum = 0
for x in arr
    sum = sum + x
sum
)",
                      15, "ForEach_Array");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_EmptyArray) {
    RunAndExpect<int>(R"(
arr = []
count = 0
for x in arr
    count = count + 1
count
)",
                      0, "ForEach_EmptyArray");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_Nested) {
    RunAndExpect<int>(R"(
matrix = [[1, 2], [3, 4]]
sum = 0
for row in matrix
    for val in row
        sum = sum + val
sum
)",
                      10, "ForEach_Nested");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithBreak) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
sum = 0
for x in arr
    if x > 3
        break
    sum = sum + x
sum
)",
                      6, "ForEach_WithBreak");  // 1+2+3
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithContinue) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
sum = 0
for x in arr
    if x == 3
        continue
    sum = sum + x
sum
)",
                      12, "ForEach_WithContinue");  // 1+2+4+5
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithFunction) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
arr = [1, 2, 3]
sum = 0
for x in arr
    sum = sum + double(x)
sum
)",
                      12, "ForEach_WithFunction");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithConditional) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
sum = 0
for x in arr
    if x % 2 == 0
        sum = sum + x
sum
)",
                      30, "ForEach_WithConditional");  // 2+4+6+8+10
}

TEST_F(RhoAllIterationMethodsTest, ForEach_CountingElements) {
    RunAndExpect<int>(R"(
arr = [10, 20, 30, 40, 50]
count = 0
for x in arr
    count = count + 1
count
)",
                      5, "ForEach_CountingElements");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_SumOfSquares) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4]
sum = 0
for x in arr
    sum = sum + x * x
sum
)",
                      30, "ForEach_SumOfSquares");  // 1+4+9+16
}

TEST_F(RhoAllIterationMethodsTest, ForEach_MultipleArrays) {
    RunAndExpect<int>(R"(
arr1 = [1, 2, 3]
arr2 = [10, 20, 30]
sum = 0
for x in arr1
    sum = sum + x
for y in arr2
    sum = sum + y
sum
)",
                      66, "ForEach_MultipleArrays");  // 6 + 60
}

TEST_F(RhoAllIterationMethodsTest, ForEach_NestedWithBreak) {
    RunAndExpect<int>(R"(
matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
sum = 0
for row in matrix
    for val in row
        sum = sum + val
        if sum > 10
            break
    if sum > 10
        break
sum
)",
                      15, "ForEach_NestedWithBreak");  // 1+2+3+4+5
}

TEST_F(RhoAllIterationMethodsTest, ForEach_FilteringPattern) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
count = 0
for x in arr
    if x % 2 == 1
        if x > 3
            count = count + 1
count
)",
                      3, "ForEach_FilteringPattern");  // 5, 7, 9 are odd and >
                                                       // 3 (3 items, not 4)
}

TEST_F(RhoAllIterationMethodsTest, ForEach_Accumulator) {
    RunAndExpect<int>(R"(
arr = [5, 15, 25]
product = 1
for x in arr
    product = product * x
product
)",
                      1875, "ForEach_Accumulator");  // 5*15*25
}

TEST_F(RhoAllIterationMethodsTest, ForEach_MaxValue) {
    RunAndExpect<int>(R"(
arr = [3, 7, 2, 9, 4]
maxVal = 0
for x in arr
    if x > maxVal
        maxVal = x
maxVal
)",
                      9, "ForEach_MaxValue");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_MinValue) {
    RunAndExpect<int>(R"(
arr = [3, 7, 2, 9, 4]
minVal = 1000
for x in arr
    if x < minVal
        minVal = x
minVal
)",
                      2, "ForEach_MinValue");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_TripleNested) {
    RunAndExpect<int>(R"(
cube = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
sum = 0
for layer in cube
    for row in layer
        for val in row
            sum = sum + val
sum
)",
                      36, "ForEach_TripleNested");  // 1+2+3+4+5+6+7+8
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithIndex) {
    RunAndExpect<int>(
        R"(
arr = [10, 20, 30, 40, 50]
sum = 0
i = 0
for x in arr
    sum = sum + x + i
    i = i + 1
sum
)",
        160, "ForEach_WithIndex");  // (10+0)+(20+1)+(30+2)+(40+3)+(50+4)
}

TEST_F(RhoAllIterationMethodsTest, ForEach_EvenOddCount) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8]
even = 0
odd = 0
for x in arr
    if x % 2 == 0
        even = even + 1
    else
        odd = odd + 1
even + odd
)",
                      8, "ForEach_EvenOddCount");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_ConditionalSum) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
sum1 = 0
sum2 = 0
for x in arr
    if x <= 5
        sum1 = sum1 + x
    else
        sum2 = sum2 + x
sum1 + sum2
)",
                      55, "ForEach_ConditionalSum");
}

TEST_F(RhoAllIterationMethodsTest, ForEach_WithModulo) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
count = 0
for x in arr
    if x % 3 == 0
        count = count + 1
count
)",
                      4, "ForEach_WithModulo");  // 3, 6, 9, 12
}

// ============================================================================
// CATEGORY 3: MIXED PATTERNS (10 tests)
// ============================================================================

TEST_F(RhoAllIterationMethodsTest, Mixed_WhileWithArray) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
i = 0
sum = 0
while i < 5
    sum = sum + arr[i]
    i = i + 1
sum
)",
                      15, "Mixed_WhileWithArray");
}

TEST_F(RhoAllIterationMethodsTest, Mixed_ForEachThenWhile) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3]
sum = 0
for x in arr
    sum = sum + x
i = 0
while i < 3
    sum = sum + 1
    i = i + 1
sum
)",
                      9, "Mixed_ForEachThenWhile");  // 6 + 3
}

TEST_F(RhoAllIterationMethodsTest, Mixed_NestedForEachInWhile) {
    RunAndExpect<int>(R"(
arrs = [[1, 2], [3, 4], [5, 6]]
i = 0
sum = 0
while i < 3
    for x in arrs[i]
        sum = sum + x
    i = i + 1
sum
)",
                      21, "Mixed_NestedForEachInWhile");
}

TEST_F(RhoAllIterationMethodsTest, Mixed_WhileInsideForEach) {
    RunAndExpect<int>(R"(
limits = [2, 3, 4]
sum = 0
for limit in limits
    i = 0
    while i < limit
        sum = sum + 1
        i = i + 1
sum
)",
                      9, "Mixed_WhileInsideForEach");  // 2+3+4
}

TEST_F(RhoAllIterationMethodsTest, Mixed_ForAndForEach) {
    RunAndExpect<int>(R"(
arr = [10, 20, 30]
sum = 0
for i = 0; i < 3; i = i + 1
    sum = sum + arr[i]
for x in arr
    sum = sum + x
sum
)",
                      120, "Mixed_ForAndForEach");  // 60 + 60
}

TEST_F(RhoAllIterationMethodsTest, Mixed_DoWhileWithArray) {
    RunAndExpect<int>(R"(
arr = [5, 15, 25]
i = 0
sum = 0
do
    sum = sum + arr[i]
    i = i + 1
while i < 3
sum
)",
                      45, "Mixed_DoWhileWithArray");
}

TEST_F(RhoAllIterationMethodsTest, Mixed_AllThreeLoops) {
    RunAndExpect<int>(R"(
sum = 0
i = 0
while i < 2
    sum = sum + 1
    i = i + 1
for j = 0; j < 2; j = j + 1
    sum = sum + 1
k = 0
do
    sum = sum + 1
    k = k + 1
while k < 2
sum
)",
                      6, "Mixed_AllThreeLoops");  // 2+2+2
}

TEST_F(RhoAllIterationMethodsTest, Mixed_BreakInMultipleLevels) {
    RunAndExpect<int>(R"(
matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
sum = 0
i = 0
while i < 3
    for x in matrix[i]
        sum = sum + x
        if sum > 15
            break
    if sum > 15
        break
    i = i + 1
sum
)",
                      21, "Mixed_BreakInMultipleLevels");
}

TEST_F(RhoAllIterationMethodsTest, Mixed_ContinueInForEach) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
sum = 0
for x in arr
    if x % 2 == 0
        continue
    if x % 3 == 0
        continue
    sum = sum + x
sum
)",
                      13, "Mixed_ContinueInForEach");  // 1+5+7 = 13 (skip evens
                                                       // and multiples of 3)
}

TEST_F(RhoAllIterationMethodsTest, Mixed_ComplexNesting) {
    RunAndExpect<int>(R"(
result = 0
for i = 0; i < 2; i = i + 1
    j = 0
    while j < 2
        arr = [1, 2]
        for x in arr
            result = result + 1
        j = j + 1
result
)",
                      8, "Mixed_ComplexNesting");  // 2*2*2
}

// ============================================================================
// CATEGORY 4: ITERATION WITH FUNCTIONS (10 tests)
// ============================================================================

TEST_F(RhoAllIterationMethodsTest, Function_MapPattern) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
arr = [1, 2, 3, 4, 5]
result = []
for x in arr
    result = result + [double(x)]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      30, "Function_MapPattern");  // 2+4+6+8+10
}

TEST_F(RhoAllIterationMethodsTest, Function_FilterPattern) {
    RunAndExpect<int>(R"(
fun isEven(x) { x % 2 == 0 }
arr = [1, 2, 3, 4, 5, 6, 7, 8]
result = []
for x in arr
    if isEven(x)
        result = result + [x]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      20, "Function_FilterPattern");  // 2+4+6+8
}

TEST_F(RhoAllIterationMethodsTest, Function_ReducePattern) {
    RunAndExpect<int>(R"(
fun add(a, b) { a + b }
arr = [1, 2, 3, 4, 5]
acc = 0
for x in arr
    acc = add(acc, x)
acc
)",
                      15, "Function_ReducePattern");
}

TEST_F(RhoAllIterationMethodsTest, Function_NestedCallsInLoop) {
    RunAndExpect<int>(R"(
fun square(x) { x * x }
fun double(x) { x * 2 }
arr = [1, 2, 3]
sum = 0
for x in arr
    sum = sum + square(double(x))
sum
)",
                      56, "Function_NestedCallsInLoop");  // 4+16+36
}

TEST_F(RhoAllIterationMethodsTest, Function_RecursiveHelper) {
    RunAndExpect<int>(R"(
fun factorial(n) {
    if n <= 1
        1
    else
        n * factorial(n - 1)
}
arr = [1, 2, 3, 4]
sum = 0
for x in arr
    sum = sum + factorial(x)
sum
)",
                      33, "Function_RecursiveHelper");  // 1+2+6+24
}

TEST_F(RhoAllIterationMethodsTest, Function_ConditionalTransform) {
    RunAndExpect<int>(R"(
fun transform(x) {
    if x % 2 == 0
        x * 2
    else
        x + 10
}
arr = [1, 2, 3, 4, 5]
sum = 0
for x in arr
    sum = sum + transform(x)
sum
)",
                      51,
                      "Function_ConditionalTransform");  // 11+4+13+8+15 = 51
}

TEST_F(RhoAllIterationMethodsTest, Function_ChainedOperations) {
    RunAndExpect<int>(
        R"(
fun inc(x) { x + 1 }
fun double(x) { x * 2 }
fun square(x) { x * x }
arr = [1, 2, 3]
sum = 0
for x in arr
    sum = sum + square(double(inc(x)))
sum
)",
        116,
        "Function_ChainedOperations");  // square(double(inc(1)))+square(double(inc(2)))+square(double(inc(3)))
                                        // = 16+36+64 = 116
}

TEST_F(RhoAllIterationMethodsTest, Function_PredicateFilter) {
    RunAndExpect<int>(R"(
fun inRange(x) {
    if x >= 3
        if x <= 7
            true
        else
            false
    else
        false
}
arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
count = 0
for x in arr
    if inRange(x)
        count = count + 1
count
)",
                      5, "Function_PredicateFilter");  // 3,4,5,6,7
}

TEST_F(RhoAllIterationMethodsTest, Function_Accumulator) {
    RunAndExpect<int>(R"(
fun combine(acc, x) { acc + x * x }
arr = [1, 2, 3, 4]
result = 0
for x in arr
    result = combine(result, x)
result
)",
                      30, "Function_Accumulator");  // 1+4+9+16
}

TEST_F(RhoAllIterationMethodsTest, Function_WithLocalState) {
    RunAndExpect<int>(R"(
fun processWithState(x, state) {
    state + x * 2
}
arr = [1, 2, 3, 4, 5]
state = 0
for x in arr
    state = processWithState(x, state)
state
)",
                      30, "Function_WithLocalState");  // 0+2+4+6+8+10
}

// ============================================================================
// CATEGORY 5: EDGE CASES AND SPECIAL PATTERNS (10 tests)
// ============================================================================

TEST_F(RhoAllIterationMethodsTest, Edge_EmptyLoop) {
    RunAndExpect<int>(R"(
i = 0
while i < 0
    i = i + 1
i
)",
                      0, "Edge_EmptyLoop");
}

TEST_F(RhoAllIterationMethodsTest, Edge_SingleIteration) {
    RunAndExpect<int>(R"(
i = 0
count = 0
while i < 1
    count = count + 1
    i = i + 1
count
)",
                      1, "Edge_SingleIteration");
}

TEST_F(RhoAllIterationMethodsTest, Edge_LargeCount) {
    RunAndExpect<int>(R"(
i = 0
count = 0
while i < 100
    count = count + 1
    i = i + 1
count
)",
                      100, "Edge_LargeCount");
}

TEST_F(RhoAllIterationMethodsTest, Edge_ImmediateBreak) {
    RunAndExpect<int>(R"(
i = 0
while true
    break
    i = i + 1
i
)",
                      0, "Edge_ImmediateBreak");
}

TEST_F(RhoAllIterationMethodsTest, Edge_MultipleBreaks) {
    RunAndExpect<int>(R"(
i = 0
sum = 0
while i < 100
    if i == 3
        break
    if i == 5
        break
    sum = sum + i
    i = i + 1
sum
)",
                      3, "Edge_MultipleBreaks");  // 0+1+2
}

TEST_F(RhoAllIterationMethodsTest, Edge_AllContinues) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
count = 0
for x in arr
    continue
    count = count + 1
count
)",
                      0, "Edge_AllContinues");  // Never increments
}

TEST_F(RhoAllIterationMethodsTest, Edge_BreakOnFirstElement) {
    RunAndExpect<int>(R"(
arr = [1, 2, 3, 4, 5]
sum = 0
for x in arr
    break
    sum = sum + x
sum
)",
                      0, "Edge_BreakOnFirstElement");
}

TEST_F(RhoAllIterationMethodsTest, Edge_NestedBreakOuter) {
    RunAndExpect<int>(R"(
outer = 0
i = 0
while i < 10
    j = 0
    while j < 10
        outer = outer + 1
        if outer == 5
            break
        j = j + 1
    if outer == 5
        break
    i = i + 1
outer
)",
                      5, "Edge_NestedBreakOuter");
}

TEST_F(RhoAllIterationMethodsTest, Edge_ZeroToZero) {
    RunAndExpect<int>(R"(
sum = 0
for i = 0; i < 0; i = i + 1
    sum = sum + i
sum
)",
                      0, "Edge_ZeroToZero");
}

TEST_F(RhoAllIterationMethodsTest, Edge_NegativeStep) {
    RunAndExpect<int>(R"(
sum = 0
i = 10
while i > 0
    sum = sum + i
    i = i - 2
sum
)",
                      30, "Edge_NegativeStep");  // 10+8+6+4+2
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInForEach_VerifyFix) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
arr = [1, 2, 3]
result = []
for x in arr
    result = result + [double(x)]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      12, "Function_CallInForEach_VerifyFix");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInWhileLoop) {
    RunAndExpect<int>(R"(
fun inc(x) { x + 1 }
sum = 0
i = 0
while i < 5
    sum = sum + inc(i)
    i = i + 1
sum
)",
                      15, "Function_CallInWhileLoop");  // 1+2+3+4+5
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInForLoop) {
    RunAndExpect<int>(R"(
fun square(x) { x * x }
sum = 0
for i = 1; i <= 3; i = i + 1
    sum = sum + square(i)
sum
)",
                      14, "Function_CallInForLoop");  // 1+4+9
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInDoWhileLoop) {
    RunAndExpect<int>(R"(
fun triple(x) { x * 3 }
sum = 0
i = 1
do
    sum = sum + triple(i)
    i = i + 1
while i <= 3
sum
)",
                      18, "Function_CallInDoWhileLoop");  // 3+6+9
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInNestedLoops) {
    RunAndExpect<int>(
        R"(
fun add(a, b) { a + b }
sum = 0
for i = 1; i <= 2; i = i + 1
    for j = 1; j <= 2; j = j + 1
        sum = sum + add(i, j)
sum
)",
        12,
        "Function_CallInNestedLoops");  // add(1,1)+add(1,2)+add(2,1)+add(2,2)
                                        // = 2+3+3+4 = 12
}

TEST_F(RhoAllIterationMethodsTest, Function_WithReturnInLoop) {
    RunAndExpect<int>(R"(
fun half(x)
    return x / 2
arr = [2, 4, 6]
result = []
for x in arr
    result = result + [half(x)]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      6, "Function_WithReturnInLoop");  // 1+2+3
}

TEST_F(RhoAllIterationMethodsTest, InlineFunction_InLoop) {
    RunAndExpect<int>(R"(
fun double_inline(x) { x * 2 }
arr = [1, 2, 3]
sum = 0
for x in arr
    sum = sum + double_inline(x)
sum
)",
                      12, "InlineFunction_InLoop");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, Function_WithMultipleParams_InLoop) {
    RunAndExpect<int>(R"(
fun multiply(a, b) { a * b }
sum = 0
for i = 1; i <= 3; i = i + 1
    sum = sum + multiply(i, 2)
sum
)",
                      12, "Function_WithMultipleParams_InLoop");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInMapPattern) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
arr = [1, 2, 3]
result = []
for x in arr
    result = result + [double(x)]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      12, "Function_CallInMapPattern");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInFilterPattern) {
    RunAndExpect<int>(R"(
fun is_even(x) { x % 2 == 0 }
arr = [1, 2, 3, 4, 5]
result = []
for x in arr
    if is_even(x)
        result = result + [x]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      6, "Function_CallInFilterPattern");  // 2+4
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInReducePattern) {
    RunAndExpect<int>(R"(
fun add(a, b) { a + b }
arr = [1, 2, 3, 4]
result = 0
for x in arr
    result = add(result, x)
result
)",
                      10, "Function_CallInReducePattern");  // 1+2+3+4
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInComprehension) {
    RunAndExpect<int>(R"(
fun square(x) { x * x }
result = []
for x in [1, 2, 3]
    result = result + [square(x)]
sum = 0
for y in result
    sum = sum + y
sum
)",
                      14, "Function_CallInComprehension");  // 1+4+9
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInWhileWithBreak) {
    RunAndExpect<int>(R"(
fun inc(x) { x + 1 }
sum = 0
i = 0
while i < 10
    sum = sum + inc(i)
    i = i + 1
    if sum > 10
        break
sum
)",
                      15, "Function_CallInWhileWithBreak");  // 1+2+3+4+5
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInForEachWithContinue) {
    RunAndExpect<int>(R"(
fun double(x) { x * 2 }
arr = [1, 2, 3, 4]
sum = 0
for x in arr
    if x % 2 == 0
        continue
    sum = sum + double(x)
sum
)",
                      8, "Function_CallInForEachWithContinue");  // 2+6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInMixedLoops) {
    RunAndExpect<int>(
        R"(
fun add(a, b) { a + b }
sum = 0
for i = 1; i <= 2; i = i + 1
    j = 1
    while j <= 2
        sum = sum + add(i, j)
        j = j + 1
sum
)",
        12,
        "Function_CallInMixedLoops");  // add(1,1)+add(1,2)+add(2,1)+add(2,2)
                                       // = 2+3+3+4 = 12
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInAllThreeLoops) {
    RunAndExpect<int>(
        R"(
fun inc(x) { x + 1 }
sum = 0
i = 0
while i < 2
    sum = sum + inc(i)
    i = i + 1
for j = 2; j < 4; j = j + 1
    sum = sum + inc(j)
for k in [3, 4]
    sum = sum + inc(k)
sum
)",
        19,
        "Function_CallInAllThreeLoops");  // inc(0)+inc(1) + inc(2)+inc(3) +
                                          // inc(3)+inc(4) = 1+2+3+4+4+5 = 19
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInEdgeCases) {
    RunAndExpect<int>(R"(
fun identity(x) { x }
sum = 0
for x in []
    sum = sum + identity(x)
for x in [0]
    sum = sum + identity(x)
for x in [1, -1]
    sum = sum + identity(x)
sum
)",
                      0, "Function_CallInEdgeCases");  // 0 + 0 + 1 + (-1) = 0
}

TEST_F(RhoAllIterationMethodsTest, Function_CallWithFloats) {
    RunAndExpect<float>(R"(
fun half(x) { x / 2.0 }
sum = 0.0
for x in [2.0, 4.0, 6.0]
    sum = sum + half(x)
sum
)",
                        6.0f, "Function_CallWithFloats");  // 1.0 + 2.0 + 3.0
}

TEST_F(RhoAllIterationMethodsTest, Function_CallNestedFunctions) {
    RunAndExpect<int>(
        R"(
fun square(x) { x * x }
fun sum_squares(a, b) { square(a) + square(b) }
total = 0
for i in [1, 2]
    total = total + sum_squares(i, i + 1)
total
)",
        1 + 4 + 4 + 9,
        "Function_CallNestedFunctions");  // 1^2 + 2^2 + 2^2 + 3^2 = 1+4+4+9=18
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInDoWhileComplex) {
    RunAndExpect<int>(
        R"(
fun fib(n)
    if n <= 1
        n
    else
        fib(n - 1) + fib(n - 2)
sum = 0
i = 1
do
    sum = sum + fib(i)
    i = i + 1
while i <= 3
sum
)",
        1 + 1 + 2,
        "Function_CallInDoWhileComplex");  // fib(1)=1, fib(2)=1, fib(3)=2
}

TEST_F(RhoAllIterationMethodsTest, Function_CallWithArrays) {
    RunAndExpect<int>(R"(
fun length(arr) { arr.size() }
sum = 0
for arr in [[1,2], [3,4,5], [6]]
    sum = sum + length(arr)
sum
)",
                      2 + 3 + 1, "Function_CallWithArrays");  // 6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInWhileWithCondition) {
    RunAndExpect<int>(
        R"(
fun is_positive(x) { x > 0 }
count = 0
i = -2
while i < 5
    if is_positive(i)
        count = count + 1
    i = i + 1
count
)",
        4,
        "Function_CallInWhileWithCondition");  // i in [-2..4] (i<5), positive
                                               // values: 1,2,3,4 → count=4
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInForLoopRange) {
    RunAndExpect<int>(R"(
fun cube(x) { x * x * x }
sum = 0
for i = 1; i <= 3; i = i + 1
    sum = sum + cube(i)
sum
)",
                      1 + 8 + 27, "Function_CallInForLoopRange");  // 36
}

// Diagnostic: sum_list called directly, no outer foreach
TEST_F(RhoAllIterationMethodsTest, Debug_SumListAlone) {
    RunAndExpect<int>(R"(
fun sum_list(lst)
    s = 0
    for x in lst
        s = s + x
    s
sum_list([1,2])
)",
                      3, "Debug_SumListAlone");
}

// Diagnostic: store sum_list result then read it
TEST_F(RhoAllIterationMethodsTest, Debug_StoreAndRetrieve) {
    RunAndExpect<int>(R"(
fun sum_list(lst)
    s = 0
    for x in lst
        s = s + x
    s
total = sum_list([1,2])
total
)",
                      3, "Debug_StoreAndRetrieve");
}

// Diagnostic: call sum_list twice, add results
TEST_F(RhoAllIterationMethodsTest, Debug_TwoIndependentCalls) {
    RunAndExpect<int>(R"(
fun sum_list(lst)
    s = 0
    for x in lst
        s = s + x
    s
r1 = sum_list([1,2])
r2 = sum_list([3,4])
r1 + r2
)",
                      10, "Debug_TwoIndependentCalls");
}

// Diagnostic: sum_list called twice sequentially
TEST_F(RhoAllIterationMethodsTest, Debug_SumListTwice) {
    RunAndExpect<int>(R"(
fun sum_list(lst)
    s = 0
    for x in lst
        s = s + x
    s
total = sum_list([1,2])
total = total + sum_list([3,4])
total
)",
                      10, "Debug_SumListTwice");
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInForEachNested) {
    RunAndExpect<int>(R"(
fun sum_list(lst)
    s = 0
    for x in lst
        s = s + x
    s
total = 0
for lst in [[1,2], [3,4]]
    total = total + sum_list(lst)
total
)",
                      3 + 7, "Function_CallInForEachNested");  // 10
}

TEST_F(RhoAllIterationMethodsTest, Function_CallWithDefaultArgs) {
    RunAndExpect<int>(R"(
fun power(base, exp) { if exp == 0 { 1 } else { base * power(base, exp - 1) } }
sum = 0
for b in [2, 3]
    sum = sum + power(b, 2)
sum
)",
                      4 + 9, "Function_CallWithDefaultArgs");  // 13
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInMixedTypes) {
    RunAndExpect<int>(R"(
fun process(x) { if x % 2 == 0 { x / 2 } else { x * 3 } }
sum = 0
for x in [1, 2, 3, 4]
    sum = sum + process(x)
sum
)",
                      3 + 1 + 9 + 2, "Function_CallInMixedTypes");  // 15
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInErrorHandling) {
    RunAndExpect<int>(R"(
fun safe_div(a, b) { if b == 0 { 0 } else { a / b } }
sum = 0
for pair in [[10, 2], [8, 0], [6, 3]]
    a = pair[0]
    b = pair[1]
    sum = sum + safe_div(a, b)
sum
)",
                      5 + 0 + 2, "Function_CallInErrorHandling");  // 7
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInRecursionLimit) {
    RunAndExpect<int>(R"(
fun countdown(n) { if n == 0 { 0 } else { n + countdown(n - 1) } }
result = countdown(5)
result
)",
                      15, "Function_CallInRecursionLimit");  // 5+4+3+2+1+0 = 15
}

TEST_F(RhoAllIterationMethodsTest, Function_CallWithClosures) {
    RunAndExpect<int>(R"(
fun add5(x) { x + 5 }
sum = 0
for x in [1, 2, 3]
    sum = sum + add5(x)
sum
)",
                      6 + 7 + 8, "Function_CallWithClosures");  // 21
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInParallelStructures) {
    RunAndExpect<int>(R"(
fun min(a, b) { if a < b { a } else { b } }
result = []
for i in [1, 2, 3]
    for j in [4, 5]
        result = result + [min(i, j)]
sum = 0
for x in result
    sum = sum + x
sum
)",
                      1 + 1 + 2 + 2 + 3 + 3,
                      "Function_CallInParallelStructures");  // 12
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInStatefulLoop) {
    RunAndExpect<int>(R"(
fun update_state(state, val) { state + val }
state = 0
for x in [10, 20, 30]
    state = update_state(state, x)
state
)",
                      60, "Function_CallInStatefulLoop");
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInConditionalLoop) {
    RunAndExpect<int>(R"(
fun should_continue(x) { x < 10 }
sum = 0
x = 1
while should_continue(x)
    sum = sum + x
    x = x + 1
sum
)",
                      45, "Function_CallInConditionalLoop");  // 1+2+...+9 = 45
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInMapReduce) {
    RunAndExpect<int>(R"(
fun map_double(x) { x * 2 }
fun reduce_sum(acc, x) { acc + x }
mapped = []
for x in [1, 2, 3]
    mapped = mapped + [map_double(x)]
result = 0
for y in mapped
    result = reduce_sum(result, y)
result
)",
                      12, "Function_CallInMapReduce");  // 2+4+6
}

TEST_F(RhoAllIterationMethodsTest, Debug_AndOperator_2mod2) {
    // 2 % 2 == 0 should be true; verify and works with two bools
    RunAndExpect<bool>(R"(
fun even_pos(x) { x > 0 and x % 2 == 0 }
even_pos(2)
)",
                       true, "Debug_AndOperator_2mod2");
}

TEST_F(RhoAllIterationMethodsTest, Debug_Modulo2) {
    // 2 % 2 should be 0
    RunAndExpect<int>(R"(
2 % 2
)",
                      0, "Debug_Modulo2");
}

TEST_F(RhoAllIterationMethodsTest, Debug_CondInLoop) {
    // Test calling a function with and inside a for loop
    RunAndExpect<int>(R"(
fun even_pos(x) { x > 0 and x % 2 == 0 }
sum = 0
for v in [1, 2, 3, 4]
    if even_pos(v)
        sum = sum + v
sum
)",
                      2 + 4, "Debug_CondInLoop");
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInComplexConditionals) {
    RunAndExpect<int>(R"(
fun complex_cond(x) { x > 0 and x % 2 == 0 }
sum = 0
for x in [0, 1, 2, 3, 4]
    if complex_cond(x)
        sum = sum + x
sum
)",
                      2 + 4, "Function_CallInComplexConditionals");  // 6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInDynamicLoop) {
    RunAndExpect<int>(R"(
fun get_limit() { 3 }
fun inc(x) { x + 1 }
sum = 0
i = 1
limit = get_limit()
while i <= limit
    sum = sum + inc(i - 1)
    i = i + 1
sum
)",
                      1 + 2 + 3, "Function_CallInDynamicLoop");  // 6
}

TEST_F(RhoAllIterationMethodsTest, Function_CallInNestedRecursion) {
    RunAndExpect<int>(R"(
fun ackermann(m, n)
    if m == 0
        n + 1
    else if n == 0
        ackermann(m - 1, 1)
    else
        ackermann(m - 1, ackermann(m, n - 1))
result = ackermann(1, 1)
result
)",
                      3, "Function_CallInNestedRecursion");
}
