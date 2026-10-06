#include <KAI/Language/Sigma/SigmaLexer.h>

#include <cctype>
#include <format>

KAI_BEGIN

void SigmaLexer::AddKeyWords() {
    keyWords_["fun"] = Enum::Fun;
    keyWords_["return"] = Enum::Return;
    keyWords_["yield"] = Enum::Yield;
    keyWords_["if"] = Enum::If;
    keyWords_["else"] = Enum::Else;
    keyWords_["while"] = Enum::While;
    keyWords_["do"] = Enum::Do;
    keyWords_["for"] = Enum::For;
    keyWords_["in"] = Enum::In;
    keyWords_["break"] = Enum::Break;
    keyWords_["continue"] = Enum::Continue;
    keyWords_["assert"] = Enum::Assert;
    keyWords_["true"] = Enum::True;
    keyWords_["false"] = Enum::False;
    keyWords_["self"] = Enum::Self;
}

bool SigmaLexer::Error(const char *message) {
    Fail(std::format("{}:{}: {}", lineNumber_ + 1, offset_ + 1, message));
    return false;
}

bool SigmaLexer::LastIs(Enum type) const { return !tokens_.empty() && tokens_.back().type == type; }

bool SigmaLexer::SkipToEndOfLine() {
    // Position on the line's '\n' so the next call decides about NewLine.
    offset_ = static_cast<int>(Line().size()) - 1;
    return true;
}

bool SigmaLexer::LexIndentation() {
    const std::string &line = Line();
    int level = 0;
    int spaces = 0;
    size_t i = 0;
    for (; i < line.size(); ++i) {
        const char ch = line[i];
        if (ch == '\t') {
            ++level;
        } else if (ch == ' ') {
            if (++spaces == 4) {
                ++level;
                spaces = 0;
            }
        } else if (ch != '\r') {
            break;
        }
    }

    // Blank and comment-only lines do not affect indentation.
    const bool blank = i >= line.size() || line[i] == '\n' ||
                       (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/');
    if (blank) {
        offset_ = static_cast<int>(line.size()) - 1;
        Next();  // to the next line, without emitting NewLine
        return true;
    }

    offset_ = static_cast<int>(i);
    if (depth_ > 0) return true;  // continuation line inside brackets

    if (spaces != 0) return Error("indentation must be a tab or a multiple of four spaces");

    const Slice here(offset_, offset_);
    if (level > indents_.back()) {
        if (level != indents_.back() + 1) return Error("unexpected indent");
        indents_.push_back(level);
        return Add(Token(Enum::Indent, *this, lineNumber_, here));
    }

    while (level < indents_.back()) {
        indents_.pop_back();
        Add(Token(Enum::Dedent, *this, lineNumber_, here));
    }
    if (level != indents_.back()) return Error("dedent does not match any outer indentation level");
    return true;
}

bool SigmaLexer::LexWord() {
    const int start = offset_;
    while (std::isalnum(static_cast<unsigned char>(Current())) || Current() == '_') Next();
    const Slice slice(start, offset_);
    const std::string word = Line().substr(start, offset_ - start);

    if (word == "pi") return LexPiBlock(start);

    auto kw = keyWords_.find(word);
    return Add(kw != keyWords_.end() ? kw->second : Enum::Name, slice);
}

bool SigmaLexer::LexPiBlock(int start) {
    const std::string &line = Line();
    size_t i = static_cast<size_t>(offset_);
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    if (i >= line.size() || line[i] != '{') return Error("expected '{' after 'pi'");

    int braces = 0;
    for (; i < line.size() && line[i] != '\n'; ++i) {
        if (line[i] == '{') ++braces;
        if (line[i] == '}' && --braces == 0) break;
    }
    if (braces != 0 || i >= line.size() || line[i] != '}') return Error("a pi { ... } block must close on the same line");

    offset_ = static_cast<int>(i) + 1;
    return Add(Token(Enum::PiBlock, *this, lineNumber_, Slice(start, offset_)));
}

bool SigmaLexer::LexNumber() {
    const int start = offset_;
    while (std::isdigit(static_cast<unsigned char>(Current()))) Next();
    if (Current() == '.' && std::isdigit(static_cast<unsigned char>(Peek()))) {
        Next();
        while (std::isdigit(static_cast<unsigned char>(Current()))) Next();
        return Add(Enum::Float, Slice(start, offset_));
    }
    return Add(Enum::Int, Slice(start, offset_));
}

bool SigmaLexer::LexSingleQuoted() {
    const std::string &line = Line();
    const int start = offset_;
    bool escaped = false;
    for (size_t i = static_cast<size_t>(start) + 1; i < line.size() && line[i] != '\n'; ++i) {
        if (escaped) {
            escaped = false;
        } else if (line[i] == '\\') {
            escaped = true;
        } else if (line[i] == '\'') {
            offset_ = static_cast<int>(i) + 1;
            AddStringToken(lineNumber_, Slice(start + 1, static_cast<int>(i)));
            return true;
        }
    }
    return Error("unterminated string");
}

bool SigmaLexer::NextToken() {
    if (failed || lineNumber_ >= static_cast<int>(lines_.size())) return false;

    if (offset_ == 0 && indentedLine_ != lineNumber_) {
        indentedLine_ = lineNumber_;
        return LexIndentation();
    }

    const char c = Current();
    if (c == 0) return false;

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') return LexWord();
    if (std::isdigit(static_cast<unsigned char>(c))) return LexNumber();

    const char n = Peek();
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
            Next();
            return true;

        case '\n':
            if (depth_ == 0 && !tokens_.empty() && !LastIs(Enum::NewLine) && !LastIs(Enum::Indent) &&
                !LastIs(Enum::Dedent))
                return Add(Enum::NewLine);
            Next();
            return true;

        case '"': return LexString();
        case '\'': return LexSingleQuoted();

        case '(': ++depth_; return Add(Enum::OpenParen);
        case '[': ++depth_; return Add(Enum::OpenSquare);
        case '{': ++depth_; return Add(Enum::OpenBrace);
        case ')':
        case ']':
        case '}':
            if (depth_ == 0) return Error("unbalanced closing bracket");
            --depth_;
            return Add(c == ')' ? Enum::CloseParen : c == ']' ? Enum::CloseSquare : Enum::CloseBrace);

        case ',': return Add(Enum::Comma);
        case '.':
            if (n == '.')
                return Error("'...' (resume) is not supported in Sigma: in Rho it leaves for the top level without calling the function");
            return Add(Enum::Dot);
        case ';': return Add(Enum::Semi);
        case '?': return Add(Enum::Question);
        case ':':
            if (n == ':') return Error("'::' is not supported in Sigma");
            return Add(Enum::Colon);

        case '+':
            if (n == '+') return Error("'++' is not supported in Sigma; use '+= 1'");
            return n == '=' ? Add(Enum::PlusAssign, 2) : Add(Enum::Plus);
        case '-':
            if (n == '-') return Error("'--' is not supported in Sigma; use '-= 1'");
            if (n == '>') return Add(Enum::Arrow, 2);
            return n == '=' ? Add(Enum::MinusAssign, 2) : Add(Enum::Minus);
        case '*': return n == '=' ? Add(Enum::MulAssign, 2) : Add(Enum::Mul);
        case '/':
            if (n == '/') return SkipToEndOfLine();
            return n == '=' ? Add(Enum::DivAssign, 2) : Add(Enum::Divide);
        case '%': return n == '=' ? Add(Enum::ModAssign, 2) : Add(Enum::Mod);
        case '=': return n == '=' ? Add(Enum::Equiv, 2) : Add(Enum::Assign);
        case '!': return n == '=' ? Add(Enum::NotEquiv, 2) : Add(Enum::Not);
        case '<':
            if (n == '<') return Add(Enum::LeftShift, 2);
            return n == '=' ? Add(Enum::LessEquiv, 2) : Add(Enum::Less);
        case '>':
            if (n == '>') return Add(Enum::RightShift, 2);
            return n == '=' ? Add(Enum::GreaterEquiv, 2) : Add(Enum::Greater);
        case '&': return n == '&' ? Add(Enum::And, 2) : Add(Enum::BitAnd);
        case '|': return n == '|' ? Add(Enum::Or, 2) : Add(Enum::BitOr);
        case '^': return Add(Enum::BitXor);
        case '~': return Add(Enum::BitNot);

        case '`': return Error("shell commands are not supported in Sigma");
        case '@': return Error("'@' is not supported in Sigma");
        default: break;
    }

    return Error("unrecognised character");
}

void SigmaLexer::Terminate() {
    if (failed) return;
    const int line = lines_.empty() ? 0 : static_cast<int>(lines_.size()) - 1;
    const Slice none(0, 0);
    if (depth_ != 0) {
        Fail(std::format("{}:1: unclosed bracket at end of input", line + 1));
        return;
    }
    if (!tokens_.empty() && !LastIs(Enum::NewLine) && !LastIs(Enum::Dedent))
        tokens_.push_back(Token(Enum::NewLine, *this, line, none));
    while (indents_.size() > 1) {
        indents_.pop_back();
        tokens_.push_back(Token(Enum::Dedent, *this, line, none));
    }
    tokens_.push_back(Token(Enum::End, *this, line, none));
}

const char *SigmaTokenEnumType::ToString(Enum val) {
    switch (val) {
#define CASE(N) \
    case N:     \
        return #N;
        CASE(None) CASE(End) CASE(NewLine) CASE(Indent) CASE(Dedent) CASE(Int) CASE(Float) CASE(String)
        CASE(True) CASE(False) CASE(Name) CASE(Fun) CASE(Return) CASE(Yield) CASE(If) CASE(Else) CASE(While)
        CASE(Do) CASE(For) CASE(In) CASE(Break) CASE(Continue) CASE(Assert) CASE(Self) CASE(PiBlock) CASE(Dot)
        CASE(Comma) CASE(Colon) CASE(Semi) CASE(Question) CASE(Arrow) CASE(OpenParen) CASE(CloseParen)
        CASE(OpenSquare) CASE(CloseSquare) CASE(OpenBrace) CASE(CloseBrace) CASE(Plus) CASE(Minus) CASE(Mul)
        CASE(Divide) CASE(Mod) CASE(Assign) CASE(PlusAssign) CASE(MinusAssign) CASE(MulAssign) CASE(DivAssign)
        CASE(ModAssign) CASE(Equiv) CASE(NotEquiv) CASE(Less) CASE(Greater) CASE(LessEquiv) CASE(GreaterEquiv)
        CASE(Not) CASE(And) CASE(Or) CASE(BitAnd) CASE(BitOr) CASE(BitXor) CASE(BitNot) CASE(LeftShift)
        CASE(RightShift) CASE(ShellCommand) CASE(Ident)
#undef CASE
    }
    return "Unknown";
}

KAI_END
