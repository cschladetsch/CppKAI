#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct ForEachLoopTest : TestLangCommon {};

// Test foreach with arrays
TEST_F(ForEachLoopTest, ForEachArray) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(arr = [1, 2, 3, 4, 5]
sum = 0

foreach x in arr
    sum = sum + x

sum)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 15);  // 1+2+3+4+5 = 15
}

// Test foreach with strings
TEST_F(ForEachLoopTest, ForEachString) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
str = "hello"
count = 0

foreach ch in str
    count = count + 1

count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 5);
}

// Test foreach with lists
TEST_F(ForEachLoopTest, ForEachList) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
lst = [10, 20, 30]
sum = 0

foreach item in lst
    sum = sum + item

sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 60);
}

// Test foreach with maps
TEST_F(ForEachLoopTest, ForEachMap) {
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
    EXPECT_EQ(ConstDeref<int>(result), 6);  // 1 + 2 + 3
}

// Test nested foreach loops
TEST_F(ForEachLoopTest, ForEachNested) {
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
    EXPECT_EQ(ConstDeref<int>(result), 21);  // 1+2+3+4+5+6 = 21
}

// Test foreach with break
TEST_F(ForEachLoopTest, ForEachBreak) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
count = 0

foreach x in arr
    if x >= 3
        break
    count = count + 1

count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);  // Only 1 and 2 before break
}

// Test foreach with continue
TEST_F(ForEachLoopTest, ForEachContinue) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3, 4, 5]
count = 0

foreach x in arr
    if x == 3
        continue
    count = count + 1

count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 4);  // All except 3
}

// Test foreach with function calls
TEST_F(ForEachLoopTest, ForEachWithFunction) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
square = fun(x)
    return x * x

arr = [1, 2, 3, 4]
sum = 0

foreach n in arr
    sum = sum + square(n)

sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 30);  // 1+4+9+16 = 30
}

// Test foreach with empty collection
TEST_F(ForEachLoopTest, ForEachEmpty) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = []
count = 0

foreach x in arr
    count = count + 1

count
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 0);
}

// Test foreach with modification of collection elements
TEST_F(ForEachLoopTest, ForEachModifyElements) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
arr = [1, 2, 3]
sum = 0

foreach x in arr
    sum = sum + x + 10

sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 36);  // (1+10)+(2+10)+(3+10) = 36
}