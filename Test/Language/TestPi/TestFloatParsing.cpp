#include <KAI/Language/Pi/PiLexer.h>
#include <KAI/Language/Pi/PiTranslator.h>

#include "TestLangCommon.h"

struct PiFloatParsingTest : kai::TestLangCommon {};

TEST_F(PiFloatParsingTest, LexerFloatParsing) {
    // Test that the lexer correctly parses float literals
    kai::PiLexer lexer("3.14", *reg_);
    ASSERT_TRUE(lexer.Process());

    auto tokens = lexer.GetTokens();
    std::cout << "Tokens generated for '3.14':" << std::endl;
    for (const auto& tok : tokens) {
        std::cout << "  Token type: "
                  << kai::PiTokenEnumType::ToString(tok.type) << ", text: '"
                  << tok.Text() << "'" << std::endl;
    }

    // Should have 2 tokens: Float and None (terminator)
    ASSERT_GE(tokens.size(), 2);
    ASSERT_EQ(tokens[0].type, kai::PiTokenEnumType::Float)
        << "First token should be Float";
    ASSERT_EQ(tokens[0].Text(), "3.14")
        << "Float token should have text '3.14'";
}

TEST_F(PiFloatParsingTest, TranslatorFloatHandling) {
    // Test that the translator correctly handles float tokens
    kai::PiTranslator trans(*reg_);
    auto cont = trans.Translate("3.14", kai::Structure::Sequence);

    ASSERT_TRUE(cont.Exists()) << "Translation should produce a continuation";

    // Execute and check stack
    console_.GetExecutor()->Continue(cont);
    auto stack = console_.GetExecutor()->GetDataStack();

    ASSERT_EQ(stack->Size(), 1) << "Stack should have 1 element";
    ASSERT_TRUE(stack->Top().IsType<float>()) << "Top should be a float";
    ASSERT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.14f)
        << "Value should be 3.14";
}