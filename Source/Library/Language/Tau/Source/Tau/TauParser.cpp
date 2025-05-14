#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Tau/TauParser.h>
#include <assert.h>

using namespace std;

TAU_BEGIN

bool TauParser::Process(shared_ptr<Lexer> lex, Structure st) {
    current = 0;
    indent = 0;
    lexer = lex;

    if (lexer->Failed) return Fail("Lexer error: " + lexer->Error);

    StripTokens();

    root = NewNode(AstEnum::None);
    return Run(root, st);
}

void TauParser::StripTokens() {
    for (auto const &tok : lexer->GetTokens()) {
        switch (tok.type) {
            case TokenEnum::Whitespace:
            case TokenEnum::Tab:
            case TokenEnum::Comment:
            case TokenEnum::NewLine:
                continue;
        }

        tokens.push_back(tok);
    }
}

bool TauParser::Run(AstNodePtr root, Structure st) {
    switch (st) {
        case Structure::Module:
            return Module(root);
        case Structure::Namespace:
            return Namespace(root);
        case Structure::Class:
            return Class(root);
        default:
            return Fail("Cannot parse top-level structure");
    }
}

bool TauParser::Module(AstNodePtr root) { KAI_NOT_IMPLEMENTED(); }

bool TauParser::Namespace(AstNodePtr root) {
    auto ns = NewNode(TauAstEnumType::Namespace, Consume());
    Expect(TokenEnum::OpenBrace);

    while (!Empty() && !CurrentIs(TokenEnum::CloseBrace)) {
        switch (Current().type) {
            case TokenEnum::Class:
                Consume();
                return Class(ns);

            case TokenEnum::Namespace:
                Consume();
                return Namespace(ns);

            default: {
                auto const &cur = Current();
                Fail(Lexer::CreateErrorMessage(
                    cur, "Unexpected token %s",
                    TokenEnumType::ToString(cur.type)));
            }
        }
    }

    Expect(TokenEnumType::CloseBrace);
    if (Failed) return false;
    root->Add(ns);
    return true;
}

bool TauParser::Class(AstNodePtr root) {
    Next();
    const auto klass = NewNode(TauAstEnumType::Class, Consume());
    Expect(TokenEnum::OpenBrace);

    while (!Failed && !CurrentIs(TokenEnum::CloseBrace)) {
        auto ty = Expect(TokenEnum::Ident)->GetToken();
        auto name = Expect(TokenEnum::Ident)->GetToken();

        if (CurrentIs(TokenType::OpenParan)) {
            Consume();
            if (!Method(klass, ty, name)) return false;
        } else {
            if (!Field(klass, ty, name)) return false;
        }

        if (Empty()) return Fail("Incomplete Class");
    }

    if (Failed) return false;

    Expect(TokenEnum::CloseBrace);
    root->Add(klass);
    return !Failed;
}

bool TauParser::Method(AstNodePtr klass, TokenNode const &returnType,
                       TokenNode const &name) {
    auto method = NewNode(AstEnum::Method, name);
    auto args = NewNode(AstEnum::Arglist);

    method->Add(returnType);
    method->Add(args);

    while (!CurrentIs(TokenType::CloseParan)) {
        AddArg(args);
        if (!CurrentIs(TokenType::Comma)) break;

        Consume();
    }

    Expect(TokenType::CloseParan);
    Expect(TokenType::Semi);
    if (Failed) return false;
    klass->Add(method);
    return !Failed;
}

bool TauParser::Field(AstNodePtr klass, TokenNode const &ty,
                      TokenNode const &id) {
    auto field = NewNode(AstEnum::Property);
    field->Add(ty);
    field->Add(id);
    Expect(TauTokenEnumType::Semi);
    klass->Add(field);
    return !Failed;
}

// void TauParser::OptionalSemi()
//{
//     if (CurrentIs(TokenType::Semi) || PeekIs(TokenType::Semi))
//         Consume();
// }

void TauParser::AddArg(AstNodePtr parent) {
    auto arg = NewNode(AstEnum::Argument);
    arg->Add(Consume());  // type
    arg->Add(Consume());  // name_
    parent->Add(arg);
}

TAU_END
