#pragma once

#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Tau/TauAstNode.h>
#include <KAI/Language/Tau/TauLexer.h>

TAU_BEGIN

// parser specific to the IDL Tau language
class TauParser : public ParserCommon<TauLexer, TauAstEnumType> {
   public:
    typedef ParserCommon<TauLexer, TauAstEnumType> Parent;
    using Parent::AstNode;
    using Parent::AstNodePtr;
    using Parent::Lexer;
    using Parent::TokenEnum;
    using Parent::TokenNode;

    typedef TauAstEnumType::Enum AstType;
    typedef TauAstEnumType NodeType;
    typedef TauTokenEnumType TokenType;

    TauParser(Registry &r) : Parent(r) {}

    void StripTokens();
    bool Process(std::shared_ptr<Lexer> lex, Structure st) override;

   protected:
    bool Run(AstNodePtr, Structure);
    bool Namespace(AstNodePtr root);
    bool Module(AstNodePtr root);
    bool Class(AstNodePtr root);
    bool Interface(AstNodePtr root);
    bool Enum(AstNodePtr root);
    bool Struct(AstNodePtr root);
    bool Event(AstNodePtr root);
    bool Method(AstNodePtr klass, TokenNode const &returnType,
                TokenNode const &name);
    bool Field(AstNodePtr klass, TokenNode const &ty, TokenNode const &id);

    void AddArg(AstNodePtr list);
    // void OptionalSemi();
};

TAU_END

// EOF
