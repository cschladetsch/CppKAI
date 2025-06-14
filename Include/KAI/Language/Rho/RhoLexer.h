#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Rho/RhoToken.h>

KAI_BEGIN

struct RhoLexer : LexerCommon<RhoTokenEnumType> {
    typedef LexerCommon<RhoTokenEnumType> Parent;
    typedef TokenBase<RhoTokenEnumType> TokenNode;

    RhoLexer(const char *text, Registry &r) : Parent(text, r) {}
    virtual ~RhoLexer() = default;

    void AddKeyWords() override;
    bool NextToken() override;
    void Terminate() override;
    String ToString() const { return Print(); }

   protected:
    bool LexPathname();
    using Parent::reg_;
};

KAI_END
