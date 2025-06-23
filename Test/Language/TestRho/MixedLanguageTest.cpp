#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct MixedLanguageTest : TestLangCommon {};

TEST_F(MixedLanguageTest, PiToRhoTransition) {
    // Start with Pi - create some values
    console_.SetLanguage(Language::Pi);
    console_.Execute("42 'answer Store", Structure::Statement);
    console_.Execute("\"Hello from Pi\" 'message Store", Structure::Statement);

    // Switch to Rho and access Pi-created values
    console_.SetLanguage(Language::Rho);
    console_.Execute("result = answer + 8", Structure::Statement);
    console_.Execute("result", Structure::Expression);  // Put result on stack

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(50, ConstDeref<int>(result));

    // Access the string created in Pi
    data_->Clear();
    console_.Execute("message", Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    result = data_->Top();
    ASSERT_TRUE(result.IsType<String>());
    EXPECT_EQ("Hello from Pi", ConstDeref<String>(result));
}

TEST_F(MixedLanguageTest, RhoToPiTransition) {
    // Start with Rho - create a function
    console_.SetLanguage(Language::Rho);
    console_.Execute("square = fun(x) { x * x }", Structure::Statement);
    console_.Execute("myArray = [1, 2, 3, 4, 5]", Structure::Statement);

    // Switch to Pi and use Rho-created function
    console_.SetLanguage(Language::Pi);
    console_.Execute("3 square Suspend", Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(9, ConstDeref<int>(result));

    // Access the array created in Rho
    data_->Clear();
    console_.Execute("myArray 2 Index", Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(3, ConstDeref<int>(result));
}

TEST_F(MixedLanguageTest, SharedDataStructures) {
    // Create a map in Pi
    console_.SetLanguage(Language::Pi);
    console_.Execute("0 ToMap 'data Store", Structure::Statement);
    console_.Execute("data \"name\" \"Alice\" SetChild 'data Store",
                     Structure::Statement);
    console_.Execute("data \"age\" 25 SetChild 'data Store",
                     Structure::Statement);

    // Access and modify in Rho
    console_.SetLanguage(Language::Rho);
    console_.Execute("data.city = \"New York\"", Structure::Statement);
    console_.Execute("data.age = data.age + 1", Structure::Statement);

    // Verify changes in Pi
    console_.SetLanguage(Language::Pi);
    console_.Execute("data \"age\" GetChild", Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(26, ConstDeref<int>(result));

    data_->Clear();
    console_.Execute("data \"city\" GetChild", Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    result = data_->Top();
    ASSERT_TRUE(result.IsType<String>());
    EXPECT_EQ("New York", ConstDeref<String>(result));
}

TEST_F(MixedLanguageTest, FunctionInterop) {
    // Create functions in both languages
    console_.SetLanguage(Language::Rho);
    console_.Execute("add = fun(a, b) { a + b }", Structure::Statement);

    console_.SetLanguage(Language::Pi);
    console_.Execute("{ Mul } 'multiply Store", Structure::Statement);

    // Use both functions from Rho
    console_.SetLanguage(Language::Rho);
    console_.Execute("result1 = add(5, 3)", Structure::Statement);
    console_.Execute("result1", Structure::Expression);  // Put result on stack

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(8, ConstDeref<int>(result));

    // Use Pi function from Rho (via Pi block)
    data_->Clear();
    console_.Execute("result2 = { 4 6 multiply Suspend }",
                     Structure::Statement);

    ASSERT_FALSE(data_->Empty());
    result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(24, ConstDeref<int>(result));
}

TEST_F(MixedLanguageTest, LoopWithLanguageSwitch) {
    // Create a counter in Rho
    console_.SetLanguage(Language::Rho);
    console_.Execute("counter = 0", Structure::Statement);

    // Create an increment function in Pi
    console_.SetLanguage(Language::Pi);
    console_.Execute("{ 1 Plus } 'inc Store", Structure::Statement);

    // Use Pi function in Rho loop
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
while counter < 5
    counter = { counter inc Suspend }
counter
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(5, ConstDeref<int>(result));
}