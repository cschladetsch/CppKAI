#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Lisp/LispParser.h>

KAI_BEGIN

bool LispParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    lexer = lex;
    if (lex->Failed) return Fail(lex->Error);

    // Keep only the tokens that mean something. Dropping trivia up front means
    // the rest of the parser never has to think about layout.
    tokens.clear();
    for (auto const &tok : lex->GetTokens()) {
        switch (tok.type) {
            case TokenType::Whitespace:
            case TokenType::Tab:
            case TokenType::NewLine:
            case TokenType::Comment:
                continue;
            default:
                tokens.push_back(tok);
        }
    }

    if (tokens.empty()) return Fail("No input");

    current = 0;
    root = NewNode(NodeType::Program);

    return Run(st);
}

bool LispParser::Run(Structure) {
    // Every structure -- statement, expression, program -- is just a sequence
    // of forms in Lisp, so there is nothing to switch on.
    while (!Failed && !Try(TokenType::None)) {
        auto form = ParseForm();
        if (!form) return false;
        root->Add(form);
    }

    return !Failed;
}

void LispParser::SkipTrivia() {
    // Trivia is filtered in Process(); this remains as a guard for safety if
    // the token stream is ever fed in directly.
    while (!Empty() && (Try(TokenType::Whitespace) || Try(TokenType::Tab) ||
                        Try(TokenType::NewLine) || Try(TokenType::Comment))) {
        Consume();
    }
}

LispParser::AstNodePtr LispParser::ParseForm() {
    SkipTrivia();

    if (Empty() || Try(TokenType::None)) {
        CreateError("Unexpected end of input");
        return nullptr;
    }

    if (Try(TokenType::CloseParan)) {
        CreateError("Unexpected ')'");
        return nullptr;
    }

    if (Try(TokenType::OpenParan)) return ParseList();

    if (Try(TokenType::Quote)) {
        Consume();
        auto quoted = ParseForm();
        if (!quoted) return nullptr;

        auto node = NewNode(NodeType::Quote);
        node->Add(quoted);
        return node;
    }

    // Anything else is an atom: number, string, boolean, nil or symbol.
    return NewNode(Consume());
}

LispParser::AstNodePtr LispParser::ParseList() {
    Consume();  // '('

    auto list = NewNode(NodeType::List);

    while (true) {
        SkipTrivia();

        if (Empty() || Try(TokenType::None)) {
            CreateError("Unterminated list: expected ')'");
            return nullptr;
        }

        if (Try(TokenType::CloseParan)) {
            Consume();
            return list;
        }

        auto form = ParseForm();
        if (!form) return nullptr;
        list->Add(form);
    }
}

bool LispParser::CreateError(const char *message) {
    if (!Failed) Fail(message);
    return false;
}

String LispParser::Print() const { return String(PrintTree().c_str()); }

KAI_END
