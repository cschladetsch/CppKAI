#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Sigma/SigmaToken.h>

#include <vector>

KAI_BEGIN

/// Lexer for Sigma. Indentation is turned into Indent/Dedent tokens using the
/// same rule as Rho (a tab, or four spaces, is one level). Newlines inside
/// brackets are ignored so that list, map and argument lists may span lines.
struct SigmaLexer : LexerCommon<SigmaTokenEnumType> {
    using Parent = LexerCommon<SigmaTokenEnumType>;
    using TokenNode = SigmaToken;

    SigmaLexer(const char *text, Registry &r) : Parent(text, r) {}

    void AddKeyWords() override;
    bool NextToken() override;
    void Terminate() override;

   private:
    std::vector<int> indents_{0};
    int indentedLine_ = -1;  // last line whose indentation has been handled
    int depth_ = 0;          // bracket nesting

    bool LexIndentation();
    bool LexWord();
    bool LexNumber();
    bool LexSingleQuoted();
    bool LexPiBlock(int start);
    bool SkipToEndOfLine();
    bool LastIs(Enum type) const;
    bool Error(const char *message);
};

KAI_END
