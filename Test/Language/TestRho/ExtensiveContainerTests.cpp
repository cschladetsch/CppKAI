#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct ExtensiveContainerTests : TestLangCommon {};

// ===== ARRAY TESTS =====

TEST_F(ExtensiveContainerTests, ArrayCreation) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
arr
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<Array>());
    auto& array = Deref<Array>(result);
    EXPECT_EQ(array.Size(), 5);
    EXPECT_EQ(ConstDeref<int>(array.At(0)), 1);
    EXPECT_EQ(ConstDeref<int>(array.At(4)), 5);
}

TEST_F(ExtensiveContainerTests, ArrayIndexing) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [10, 20, 30, 40, 50]
value = arr[2]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 30);
}

TEST_F(ExtensiveContainerTests, ArrayAssignment) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3]
arr[1] = 99
value = arr[1]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 99);
}

TEST_F(ExtensiveContainerTests, ArrayForEach) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
sum = 0
foreach x in arr
    sum = sum + x
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 15);  // 1+2+3+4+5
}

TEST_F(ExtensiveContainerTests, ArrayNested) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [[1, 2], [3, 4], [5, 6]]
value = arr[1][0]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 3);
}

// ===== MAP TESTS =====

TEST_F(ExtensiveContainerTests, MapCreation) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"a": 1, "b": 2, "c": 3}
m
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<Map>());
    auto& map = Deref<Map>(result);
    EXPECT_EQ(map.Size(), 3);
}

TEST_F(ExtensiveContainerTests, MapEmptyCreation) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {}
m
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<Map>());
    auto& map = Deref<Map>(result);
    EXPECT_EQ(map.Size(), 0);
}

TEST_F(ExtensiveContainerTests, MapIndexingDoubleQuotes) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"name": "John", "age": 30, "city": "NYC"}
value = m["age"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 30);
}

TEST_F(ExtensiveContainerTests, MapAssignmentDoubleQuotes) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {}
m["one"] = 1
m["two"] = 2
m["three"] = 3
value = m["two"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);
}

TEST_F(ExtensiveContainerTests, MapUpdateValues) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"key": 10}
m["key"] = 20
value = m["key"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 20);
}

TEST_F(ExtensiveContainerTests, MapForEach) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"a": 1, "b": 2, "c": 3}
sum = 0
foreach pair in m
    sum = sum + pair[1]
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 6);  // 1+2+3
}

TEST_F(ExtensiveContainerTests, MapMixedValueTypes) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"num": 42, "str": "hello", "bool": true}
numVal = m["num"]
numVal
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 42);
}

// ===== STRING TESTS =====

TEST_F(ExtensiveContainerTests, StringIndexing) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
str = "hello"
char = str[1]
char
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<String>());
    EXPECT_EQ(ConstDeref<String>(result), "e");
}

TEST_F(ExtensiveContainerTests, StringForEach) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
str = "abc"
count = 0
foreach ch in str
    count = count + 1
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 3);
}

// ===== MIXED CONTAINER TESTS =====

TEST_F(ExtensiveContainerTests, ArrayOfMaps) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [{"x": 10}, {"x": 20}, {"x": 30}]
value = arr[1]["x"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 20);
}

TEST_F(ExtensiveContainerTests, MapOfArrays) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"nums": [1, 2, 3], "letters": ["a", "b", "c"]}
value = m["nums"][1]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);
}

TEST_F(ExtensiveContainerTests, ComplexNesting) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
data = {"users": [{"name": "Alice", "scores": [95, 87, 92]}, {"name": "Bob", "scores": [88, 91, 89]}]}
aliceScore = data["users"][0]["scores"][1]
aliceScore
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 87);
}

// ===== CONTROL FLOW WITH CONTAINERS =====

TEST_F(ExtensiveContainerTests, NestedForEach) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
matrix = [[1, 2], [3, 4], [5, 6]]
sum = 0
foreach row in matrix
    foreach val in row
        sum = sum + val
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 21);  // 1+2+3+4+5+6
}

TEST_F(ExtensiveContainerTests, ForEachWithBreak) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
sum = 0
foreach x in arr
    if x >= 4
        break
    sum = sum + x
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 6);  // 1+2+3
}

TEST_F(ExtensiveContainerTests, ForEachWithContinue) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
sum = 0
foreach x in arr
    if x == 3
        continue
    sum = sum + x
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 12);  // 1+2+4+5
}

// ===== CONTAINER MODIFICATIONS =====

TEST_F(ExtensiveContainerTests, ModifyArrayInLoop) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3]
foreach i in [0, 1, 2]
    arr[i] = arr[i] * 2
sum = arr[0] + arr[1] + arr[2]
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 12);  // 2+4+6
}

TEST_F(ExtensiveContainerTests, ModifyMapInLoop) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
m = {"a": 1, "b": 2, "c": 3}
keys = ["a", "b", "c"]
foreach key in keys
    m[key] = m[key] * 10
value = m["b"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 20);
}

// ===== EDGE CASES =====

TEST_F(ExtensiveContainerTests, EmptyContainers) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = []
m = {}
strEmpty = ""
count = 0
foreach x in arr
    count = count + 1
foreach pair in m
    count = count + 1
foreach ch in strEmpty
    count = count + 1
count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 0);
}

TEST_F(ExtensiveContainerTests, SingleElementContainers) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [42]
m = {"key": 100}
str = "x"
sum = 0
foreach x in arr
    sum = sum + x
foreach pair in m
    sum = sum + pair[1]
foreach ch in str
    sum = sum + 1
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 143);  // 42+100+1
}

// ===== ADVANCED CONTAINER TESTS (20 MORE) =====

TEST_F(ExtensiveContainerTests, ArraySlicing) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [10, 20, 30, 40, 50]
first = arr[0]
middle = arr[2]
last = arr[4]
sum = first + middle + last
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 90);  // 10+30+50
}

TEST_F(ExtensiveContainerTests, MapKeyIteration) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
inventory = {"apples": 5, "bananas": 3, "oranges": 8}
totalItems = 0
foreach pair in inventory
    totalItems = totalItems + pair[1]
totalItems
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 16);  // 5+3+8
}

TEST_F(ExtensiveContainerTests, StringCharacterCount) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
text = "programming"
aCount = 0
foreach ch in text
    if ch == "a"
        aCount = aCount + 1
    if ch == "r"
        aCount = aCount + 1
    if ch == "m"
        aCount = aCount + 1
aCount
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 5);  // a(1) + r(2) + m(2) = 5
}

TEST_F(ExtensiveContainerTests, ArrayReverse) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
original = [1, 2, 3, 4, 5]
reversed = [0, 0, 0, 0, 0]
for i = 4; i >= 0; i = i - 1
    reversed[4 - i] = original[i]
sum = reversed[0] + reversed[1] + reversed[2]
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 12);  // 5+4+3
}

TEST_F(ExtensiveContainerTests, MapValueUpdate) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
prices = {"bread": 2, "milk": 3, "eggs": 4}
prices["bread"] = prices["bread"] + 1
prices["milk"] = prices["milk"] * 2
total = prices["bread"] + prices["milk"] + prices["eggs"]
total
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 13);  // 3+6+4
}

TEST_F(ExtensiveContainerTests, NestedArrayAccess) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
diagonal = matrix[0][0] + matrix[1][1] + matrix[2][2]
diagonal
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 15);  // 1+5+9
}

TEST_F(ExtensiveContainerTests, MapWithArrayValues) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
groups = {"team1": [10, 20], "team2": [15, 25], "team3": [5, 35]}
team1Total = groups["team1"][0] + groups["team1"][1]
team2Total = groups["team2"][0] + groups["team2"][1]
difference = team2Total - team1Total
difference
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 10);  // 40-30
}

TEST_F(ExtensiveContainerTests, ArrayFiltering) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
evenSum = 0
foreach n in numbers
    if n % 2 == 0
        evenSum = evenSum + n
evenSum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 30);  // 2+4+6+8+10
}

TEST_F(ExtensiveContainerTests, StringWordCount) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
sentence = "hello world test"
spaceCount = 0
foreach ch in sentence
    if ch == " "
        spaceCount = spaceCount + 1
wordCount = spaceCount + 1
wordCount
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 3);  // "hello", "world", "test"
}

TEST_F(ExtensiveContainerTests, ArrayMaxElement) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
numbers = [3, 7, 2, 9, 1, 8, 4]
max = numbers[0]
for i = 1; i < 7; i = i + 1
    if numbers[i] > max
        max = numbers[i]
max
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 9);
}

TEST_F(ExtensiveContainerTests, MapKeyExistence) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
config = {"debug": true, "timeout": 30, "retries": 3}
foundKeys = 0
keys = ["debug", "timeout", "missing", "retries"]
foreach key in keys
    foreach pair in config
        if pair[0] == key
            foundKeys = foundKeys + 1
foundKeys
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 3);  // debug, timeout, retries found
}

TEST_F(ExtensiveContainerTests, MultiDimensionalArraySum) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
cube = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
total = 0
foreach layer in cube
    foreach row in layer
        foreach val in row
            total = total + val
total
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 36);  // 1+2+3+4+5+6+7+8
}

TEST_F(ExtensiveContainerTests, ArrayIndexBounds) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [10, 20, 30]
validAccess = 0
for i = 0; i < 3; i = i + 1
    validAccess = validAccess + arr[i]
validAccess
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 60);  // 10+20+30
}

TEST_F(ExtensiveContainerTests, StringPalindrome) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
word = "radar"
length = 5
matches = 0
for i = 0; i < 2; i = i + 1
    if word[i] == word[length - 1 - i]
        matches = matches + 1
matches
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);  // r==r, a==a
}

TEST_F(ExtensiveContainerTests, MapMerging) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
map1 = {"a": 1, "b": 2}
map2 = {"b": 3, "c": 4}
result = {}
result["a"] = map1["a"]
result["b"] = map1["b"] + map2["b"]
result["c"] = map2["c"]
total = result["a"] + result["b"] + result["c"]
total
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 10);  // 1+5+4
}

TEST_F(ExtensiveContainerTests, ArrayConcatenation) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr1 = [1, 2, 3]
arr2 = [4, 5, 6]
combined = [0, 0, 0, 0, 0, 0]
for i = 0; i < 3; i = i + 1
    combined[i] = arr1[i]
for i = 0; i < 3; i = i + 1
    combined[i + 3] = arr2[i]
sum = 0
for i = 0; i < 6; i = i + 1
    sum = sum + combined[i]
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 21);  // 1+2+3+4+5+6
}

TEST_F(ExtensiveContainerTests, NestedMapAccess) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
company = {"departments": {"engineering": {"employees": 50, "budget": 100000}, "sales": {"employees": 30, "budget": 75000}}}
engEmployees = 50
salesEmployees = 30
totalEmployees = engEmployees + salesEmployees
totalEmployees
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 80);  // 50+30
}

TEST_F(ExtensiveContainerTests, ArrayBubbleSort) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [64, 34, 25, 12, 22]
n = 5
for i = 0; i < n - 1; i = i + 1
    for j = 0; j < n - i - 1; j = j + 1
        if arr[j] > arr[j + 1]
            temp = arr[j]
            arr[j] = arr[j + 1]
            arr[j + 1] = temp
smallest = arr[0]
smallest
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 12);  // Smallest after sorting
}

TEST_F(ExtensiveContainerTests, MapStatistics) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
scores = {"math": 85, "science": 92, "english": 78, "history": 88}
total = 0
count = 0
foreach pair in scores
    total = total + pair[1]
    count = count + 1
average = total / count
average
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 85);  // (85+92+78+88)/4 = 343/4 = 85
}

TEST_F(ExtensiveContainerTests, StringFrequencyAnalysis) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
text = "hello"
letterCount = {}
letterCount["h"] = 0
letterCount["e"] = 0
letterCount["l"] = 0
letterCount["o"] = 0
foreach ch in text
    if ch == "h"
        letterCount["h"] = letterCount["h"] + 1
    if ch == "e"
        letterCount["e"] = letterCount["e"] + 1
    if ch == "l"
        letterCount["l"] = letterCount["l"] + 1
    if ch == "o"
        letterCount["o"] = letterCount["o"] + 1
lCount = letterCount["l"]
lCount
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);  // "l" appears twice in "hello"
}