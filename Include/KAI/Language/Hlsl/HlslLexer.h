#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Hlsl/HlslToken.h>

KAI_BEGIN

struct HlslLexer : LexerCommon<HlslTokenEnumType> {
    typedef LexerCommon<HlslTokenEnumType> Parent;
    typedef TokenBase<HlslTokenEnumType> TokenNode;

    HlslLexer(const char *text, Registry &r) : Parent(text, r) {}

    void AddKeyWords() override;
    bool NextToken() override;
    void Terminate() override;

   protected:
    bool LexIdent();
    using Parent::reg_;
};

KAI_END
