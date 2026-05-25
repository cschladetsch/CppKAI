#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct ExpressionProgramParityTest : TestLangCommon {};

TEST_F(ExpressionProgramParityTest, CompactArithmeticMatchesBetweenEntryPoints) {
    console_.SetLanguage(Language::Rho);

    data_->Clear();
    console_.Execute("1+2", Structure::Expression);
    ASSERT_FALSE(data_->Empty()) << "Expression entrypoint produced no result";
    Object expr_result = data_->Top();
    ASSERT_TRUE(expr_result.IsType<int>())
        << "Expression result type mismatch: "
        << expr_result.GetClass()->GetName();
    EXPECT_EQ(3, ConstDeref<int>(expr_result));

    data_->Clear();
    console_.Execute("1+2", Structure::Program);
    ASSERT_FALSE(data_->Empty()) << "Program entrypoint produced no result";
    Object prog_result = data_->Top();
    ASSERT_TRUE(prog_result.IsType<int>())
        << "Program result type mismatch: " << prog_result.GetClass()->GetName();
    EXPECT_EQ(3, ConstDeref<int>(prog_result));
}

TEST_F(ExpressionProgramParityTest, ConsoleProcessProducesValueForCompactArithmetic) {
    console_.SetLanguage(Language::Rho);

    data_->Clear();
    String output = console_.Process("1+2");
    EXPECT_TRUE(output.Empty()) << output.c_str();

    ASSERT_FALSE(data_->Empty()) << "Console::Process produced no stack value";
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>())
        << "Process result type mismatch: " << result.GetClass()->GetName();
    EXPECT_EQ(3, ConstDeref<int>(result));
}
