#pragma once

#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Lisp/LispAstNode.h>
#include <KAI/Language/Lisp/LispLexer.h>

KAI_BEGIN

// Parser for s-expression Lisp.
//
// The whole grammar is: a program is a sequence of forms; a form is an atom, a
// quoted form, or a parenthesised list of forms. Special forms are *not*
// recognised here -- `(if a b c)` parses as a plain 4-element list and the
// translator gives it meaning. That is what lets new forms be added without
// touching the parser.
class LispParser : public ParserCommon<LispLexer, LispAstNodeEnumType> {
   public:
    typedef ParserCommon<LispLexer, LispAstNodeEnumType> Parent;
    using typename Parent::AstNode;
    using typename Parent::AstNodePtr;
    using typename Parent::Lexer;
    using typename Parent::TokenEnum;
    using typename Parent::TokenNode;

    typedef LispAstNodeEnumType NodeType;
    typedef LispTokenEnumType TokenType;

    LispParser(Registry &r) : Parent(r) {}

    virtual bool Process(std::shared_ptr<Lexer> lex, Structure st) override;
    String Print() const;

   private:
    bool Run(Structure st);

    // Parses one form. Returns null and sets the error state on failure.
    AstNodePtr ParseForm();
    AstNodePtr ParseList();

    // Whitespace, tabs, newlines and comments carry no meaning in Lisp.
    void SkipTrivia();

    bool CreateError(const char *);
};

KAI_END
