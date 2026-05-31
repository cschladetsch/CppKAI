#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "KAI/Core/Registry.h"
#include "KAI/Language/Rho/RhoLexer.h"
#include "KAI/Language/Rho/RhoParser.h"

using namespace kai;

namespace {
bool ParsesAsProgram(const char* source) {
    Registry registry;
    auto lexer = std::make_shared<RhoLexer>(source, registry);
    if (!lexer->Process()) return false;

    auto parser = std::make_shared<RhoParser>(registry);
    return parser->Process(lexer, Structure::Program);
}
}  // namespace

TEST(RhoBraceBlockSyntax, AcceptsIndentedIfBlock) {
    EXPECT_TRUE(ParsesAsProgram("if true\n    return 1\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedIfBlock) {
    EXPECT_FALSE(ParsesAsProgram("if true { return 1 }\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedElseBlock) {
    EXPECT_FALSE(ParsesAsProgram("if true\n    return 1\nelse { return 2 }\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedWhileBlock) {
    EXPECT_FALSE(ParsesAsProgram("while true { break }\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedForBlock) {
    EXPECT_FALSE(ParsesAsProgram("for i = 0; i < 3; i = i + 1 { break }\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedNamedFunctionBody) {
    EXPECT_FALSE(ParsesAsProgram("fun add_one(x) { return x + 1 }\n"));
}

TEST(RhoBraceBlockSyntax, RejectsBraceDelimitedAnonymousFunctionBody) {
    EXPECT_FALSE(ParsesAsProgram("f = fun(x) { return x + 1 }\n"));
}

TEST(RhoBraceBlockSyntax, StillAcceptsMapLiterals) {
    EXPECT_TRUE(ParsesAsProgram("config = {'name': 'node', 'count': 2}\n"));
}

TEST(RhoBraceBlockSyntax, StillAcceptsPiBlocks) {
    EXPECT_TRUE(ParsesAsProgram("value = pi { 1 2 + }\n"));
}
