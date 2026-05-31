#include <gtest/gtest.h>

#include "TestLangCommon.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace kai;

struct AssertSyntaxTest : TestLangCommon {};

TEST_F(AssertSyntaxTest, AssertRequiresParentheses) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("assert(1 + 1 == 2)", Structure::Statement);
    console_.Execute("assert(true)", Structure::Statement);
    console_.Execute("x = 42", Structure::Statement);
    console_.Execute("assert(x == 42)", Structure::Statement);

    RhoTranslator translator(*reg_);
    auto bare_assert =
        translator.Translate("assert true", Structure::Statement);
    EXPECT_TRUE(translator.Failed);
    EXPECT_FALSE(bare_assert.Exists());

    RhoTranslator expression_translator(*reg_);
    auto bare_expression_assert = expression_translator.Translate(
        "assert 1 + 1 == 2", Structure::Statement);
    EXPECT_TRUE(expression_translator.Failed);
    EXPECT_FALSE(bare_expression_assert.Exists());

    SUCCEED();
}

TEST_F(AssertSyntaxTest, AssertFailure) {
    console_.SetLanguage(Language::Rho);

    // This should throw an exception
    EXPECT_THROW(
        { console_.Execute("assert(1 + 1 == 3)", Structure::Statement); },
        Exception::Base);
}

TEST_F(AssertSyntaxTest, AssertInFunction) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
validate = fun(x)
    assert(x > 0)
    assert(x < 100)
    x * 2

result = validate(50)
result
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(100, ConstDeref<int>(result));
}

TEST_F(AssertSyntaxTest, AssertInLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
sum = 0
for i = 0; i < 5; i = i + 1
    assert(i >= 0)
    assert(i < 5)
    sum = sum + i
sum
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));
}

TEST_F(AssertSyntaxTest, AssertWithComplexExpression) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
a = 5
b = 10
c = 15

assert(a < b && b < c)
assert(a + b == c)
assert(a * 3 == c)

true
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<bool>());
    EXPECT_TRUE(ConstDeref<bool>(result));
}

TEST_F(AssertSyntaxTest, CompactFunctionAssertLeavesStackEmpty) {
    console_.SetLanguage(Language::Rho);

    data_->Clear();

    const char* code =
        "fun foo(a,b) a+b\n\tassert(foo(1,2) == 3)";

    console_.Execute(code, Structure::Program);

    EXPECT_TRUE(data_->Empty())
        << "Successful assert should leave no value on the data stack";
}
