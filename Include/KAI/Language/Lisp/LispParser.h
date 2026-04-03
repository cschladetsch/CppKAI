#pragma once

#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Lisp/LispAstNode.h>
#include <KAI/Language/Lisp/LispLexer.h>

KAI_BEGIN

// parser specific to the in-fix Lisp language
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

   protected:
    void Process(Structure);

   private:
    bool Run(Structure st);
    bool Program();
    bool Statement(AstNodePtr);
    bool Expression();
    bool Logical();
    bool Relational();
    bool Additive();
    bool Term();
    bool Factor();
    void ConsumeNewLines();
    void Block(AstNodePtr block);
    bool ParsePathname();
    bool ParsePathname(AstNodePtr path);
    bool ParseFactorIdent();
    void ParseGetMember();
    void ParseMethodCall();
    void Function(AstNodePtr);
    void AddBlock(AstNodePtr fun);
    void IfCondition(AstNodePtr);
    void ParseIndexOp();
    void Assignment(AstNodePtr);
    void For(AstNodePtr);
    void While(AstNodePtr);

    bool CreateError(const char *);
};

KAI_END
