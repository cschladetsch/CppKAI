#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Lisp/LispToken.h>

KAI_BEGIN

struct LispLexer : LexerCommon<LispTokenEnumType> {
    typedef LexerCommon<LispTokenEnumType> Parent;
    typedef TokenBase<LispTokenEnumType> TokenNode;

    LispLexer(const char *text, Registry &r) : Parent(text, r) {}

    void AddKeyWords() override;
    bool NextToken() override;
    void Terminate() override;
    String ToString() const { return Print(); }

   protected:
    bool LexPathname();
    using Parent::reg_;
};

KAI_END
