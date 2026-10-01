#pragma once

#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Sigma/SigmaAstNode.h>
#include <KAI/Language/Sigma/SigmaLexer.h>

#include <initializer_list>

KAI_BEGIN

/// Parser for Sigma. Statements follow Rho; every function parameter must be
/// typed, and a declaration may give a type (`x: T = e`) or infer it from its
/// first assignment (`x = e`). Errors stop at the first problem and are
/// reported as "line:column: message".
class SigmaParser : public ParserCommon<SigmaLexer, SigmaAstNodeEnumType> {
   public:
    using Parent = ParserCommon<SigmaLexer, SigmaAstNodeEnumType>;
    using typename Parent::AstNode;
    using typename Parent::AstNodePtr;
    using typename Parent::Lexer;
    using typename Parent::TokenEnum;
    using typename Parent::TokenNode;
    using Tok = SigmaTokenEnumType;
    using Ast = SigmaAstNodeEnumType;

    explicit SigmaParser(Registry &r) : Parent(r) {}

    bool Process(std::shared_ptr<Lexer> lex, Structure st) override;

   private:
    int functionDepth_ = 0;

    // statements
    bool Program();
    AstNodePtr Statement();
    AstNodePtr Block();
    AstNodePtr FunctionDefinition();
    AstNodePtr IfStatement();
    AstNodePtr WhileStatement();
    AstNodePtr DoWhileStatement();
    AstNodePtr ForStatement();
    AstNodePtr ReturnStatement();
    AstNodePtr SimpleStatement();
    bool EndOfStatement();

    // types
    AstNodePtr Type();

    // expressions, lowest precedence first
    AstNodePtr Expression();
    AstNodePtr Or();
    AstNodePtr And();
    AstNodePtr BitOr();
    AstNodePtr BitXor();
    AstNodePtr BitAnd();
    AstNodePtr Equality();
    AstNodePtr Relational();
    AstNodePtr Shift();
    AstNodePtr Additive();
    AstNodePtr Term();
    AstNodePtr Unary();
    AstNodePtr Postfix();
    AstNodePtr Primary();
    AstNodePtr ListLiteral();
    AstNodePtr MapLiteral();

    using Level = AstNodePtr (SigmaParser::*)();
    AstNodePtr BinaryLevel(Level next, std::initializer_list<Tok::Enum> ops);

    // helpers
    bool Is(Tok::Enum type) const;
    TokenNode const &Peek(size_t ahead = 0) const;
    TokenNode Take();
    bool Accept(Tok::Enum type);
    bool Require(Tok::Enum type, const char *what);
    bool CheckName(TokenNode const &tok, const char *what);
    AstNodePtr Error(const char *message);
    AstNodePtr Error(TokenNode const &at, const std::string &message);
};

KAI_END
