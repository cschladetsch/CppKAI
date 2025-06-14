#pragma once

#include <KAI/Language/Common/LexerCommon.h>
#include <KAI/Language/Pi/PiToken.h>

KAI_BEGIN

// Lexer for the Pi language. A specialistion of the generalised lexer.
class PiLexer : public LexerCommon<PiTokenEnumType> {
   public:
    typedef LexerCommon<PiTokenEnumType> Parent;
    typedef Parent Lexer;
    typedef TokenBase<PiTokenEnumType> TokenNode;

    PiLexer(const char *text, Registry &r) : Parent(text, r) {}
    virtual ~PiLexer() = default;

    void AddKeyWords() override;
    bool NextToken() override;
    void Terminate() override;

   private:
    bool PathnameOrKeyword();

   protected:
    using Parent::reg_;

    bool ParsePathname();
};

KAI_END
