#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Tau/TauToken.h>

TAU_BEGIN

struct TauLexer : LexerCommon<TauTokenEnumType> {
    typedef LexerCommon<TauTokenEnumType> Parent;
    typedef TokenBase<TauTokenEnumType> TokenNode;
    typedef TauTokenEnumType TokenEnumType;

    TauLexer(const char *text, Registry &r) : Parent(text, r) {}

    virtual void AddKeyWords();
    virtual bool NextToken();
    virtual void Terminate();
};

TAU_END
