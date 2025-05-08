#include <KAI/Language/Common/Structure.h>
#include <KAI/Language/Pi/PiParser.h>

using namespace std;
using namespace boost;

KAI_BEGIN

bool PiParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    current = 0;
    indent = 0;
    lexer = lex;

    if (lexer->Failed) return Fail(lexer->Error);

    for (auto const &tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace &&
            tok.type != TokenEnum::NewLine && tok.type != TokenEnum::Tab &&
            tok.type != TokenEnum::Comment)
            tokens.push_back(tok);
    }

    return Run(st);
}

bool PiParser::Run(Structure st) {
    KAI_UNUSED_1(
        st);  // no real structure to Pi - it's mostly a sequence of tokens
    root = NewNode(AstEnum::Continuation);
    while (!Failed && NextSingle(root));

    return !Failed;
}

bool PiParser::NextSingle(AstNodePtr root) {
    if (Empty()) return false;

    const auto tok = Current();
    switch (tok.type) {
        case PiTokens::OpenSquareBracket:
            return ParseCompound(root, AstEnum::Array,
                                 TokenEnum::CloseSquareBracket);

        case PiTokens::OpenBrace:
            return ParseCompound(root, AstEnum::Continuation,
                                 TokenType::CloseBrace);

        case PiTokens::CloseSquareBracket:
            // Fallthrough.
        case PiTokens::CloseBrace:
            Fail(Lexer::CreateErrorMessage(Current(), "%s",
                                           "Unopened compound"));
            return false;

        case PiTokens::None:
            return false;

        default:
            root->Add(Consume());
            return true;
    }
}

bool PiParser::ParseCompound(AstNodePtr root, AstEnum nodeType, TokenEnum end) {
    Consume();
    auto node = NewNode(nodeType);
    while (!Empty() && !Failed && !Try(end)) {
        if (!NextSingle(node))
            return Fail(Lexer::CreateErrorMessage(
                Current(), "Malformed compound %s",
                PiAstNodeEnumType::ToString(nodeType)));
    }

    if (Empty()) return Fail("Malformed compound");

    if (Failed) return false;

    Consume();
    root->Add(node);
    return true;
}

KAI_END
