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

bool TauParser::Module(AstNodePtr root) { 
    auto module = NewNode(TauAstEnumType::Module);
    
    while (!Empty()) {
        switch (Current().type) {
            case TokenEnum::Namespace:
                Consume();
                if (!Namespace(module))
                    return false;
                break;
                
            default: {
                auto const &cur = Current();
                return Fail(Lexer::CreateErrorMessage(
                    cur, "Unexpected token %s in module scope, expected 'namespace'",
                    TokenEnumType::ToString(cur.type)));
            }
        }
    }
    
    root->Add(module);
    return true;
}

bool TauParser::Namespace(AstNodePtr root) {
    auto ns = NewNode(TauAstEnumType::Namespace, Consume());
    Expect(TokenEnum::OpenBrace);

    while (!Empty() && !CurrentIs(TokenEnum::CloseBrace)) {
        switch (Current().type) {
            case TokenEnum::Class:
                Consume();
                if (!Class(ns))
                    return false;
                break;

            case TokenEnum::Namespace:
                Consume();
                if (!Namespace(ns))
                    return false;
                break;

            default: {
                auto const &cur = Current();
                return Fail(Lexer::CreateErrorMessage(
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
    // The class keyword has already been consumed by Now()
    // Next token should be the class name
    const auto className = Consume(); // Class name
    const auto klass = NewNode(TauAstEnumType::Class, className);
    
    Expect(TokenEnum::OpenBrace);
    if (Failed) return false;

    while (!Failed && !CurrentIs(TokenEnum::CloseBrace)) {
        auto ty = Expect(TokenEnum::Ident);
        if (Failed) return false;
        
        auto name = Expect(TokenEnum::Ident);
        if (Failed) return false;

        if (CurrentIs(TokenType::OpenParan)) {
            Consume();
            if (!Method(klass, ty->GetToken(), name->GetToken())) return false;
        } else {
            if (!Field(klass, ty->GetToken(), name->GetToken())) return false;
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
    
    // Check for assignment (initialization)
    if (CurrentIs(TauTokenEnumType::Assign)) {
        Consume(); // consume the '=' token
        
        // Add the assigned value
        if (CurrentIs(TauTokenEnumType::Number)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            field->Add(valueNode);
        } else if (CurrentIs(TauTokenEnumType::String)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            field->Add(valueNode);
        } else if (CurrentIs(TauTokenEnumType::Ident)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            field->Add(valueNode);
        } else {
            return Fail("Expected value after '='");
        }
    }
    
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
    arg->Add(Consume());  // name
    
    // Check for default parameter value
    if (CurrentIs(TauTokenEnumType::Assign)) {
        Consume(); // consume the '=' token
        
        // Add the default value
        if (CurrentIs(TauTokenEnumType::Number)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            arg->Add(valueNode);
        } else if (CurrentIs(TauTokenEnumType::String)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            arg->Add(valueNode);
        } else if (CurrentIs(TauTokenEnumType::Ident)) {
            auto valueNode = NewNode(AstEnum::Value, Consume());
            arg->Add(valueNode);
        } else {
            Fail("Expected value after '='");
            return; // Error will be handled by the calling method
        }
    }
    
    parent->Add(arg);
}

TAU_END
