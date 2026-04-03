#include <KAI/Language/Rho/RhoLexer.h>

using namespace std;

KAI_BEGIN

void RhoLexer::AddKeyWords() {
    keyWords["if"] = Enum::If;
    keyWords["else"] = Enum::Else;
    keyWords["for"] = Enum::For;
    keyWords["true"] = Enum::True;
    keyWords["false"] = Enum::False;
    keyWords["return"] = Enum::Return;
    keyWords["self"] = Enum::Self;
    keyWords["fun"] = Enum::Fun;
    keyWords["yield"] = Enum::Yield;
    keyWords["in"] = Enum::In;
    keyWords["while"] = Enum::While;
    keyWords["assert"] = Enum::Assert;
    keyWords["pi"] = Enum::ToPi;
    keyWords["pi{"] = Enum::PiSequence;
}

bool RhoLexer::NextToken() {
    char current = Current();
    if (current == 0) return false;

    if (isalpha(current)) return LexPathname();

    if (isdigit(current)) return Add(Enum::Int, Gather(isdigit));

    switch (current) {
        case '\'':
            return LexPathname();
        case ';':
            return Add(Enum::Semi);
        case '{':
            return Add(Enum::OpenBrace);
        case '}':
            return Add(Enum::CloseBrace);
        case '(':
            return Add(Enum::OpenParan);
        case ')':
            return Add(Enum::CloseParan);
        case ' ':
            return Add(Enum::Whitespace, Gather(IsSpaceChar));
        case '@':
            return Add(Enum::Lookup);
        case ',':
            return Add(Enum::Comma);
        case '*':
            return Add(Enum::Mul);
        case '[':
            return Add(Enum::OpenSquareBracket);
        case ']':
            return Add(Enum::CloseSquareBracket);
        case '=':
            return AddIfNext('=', Enum::Equiv, Enum::Assign);
        case '!':
            return AddIfNext('=', Enum::NotEquiv, Enum::Not);
        case '&':
            return AddIfNext('&', Enum::And, Enum::BitAnd);
        case '|':
            return AddIfNext('|', Enum::Or, Enum::BitOr);
        case '<':
            return AddIfNext('=', Enum::LessEquiv, Enum::Less);
        case '>':
            return AddIfNext('=', Enum::GreaterEquiv, Enum::Greater);
        case '"':
            return LexString();  // "
        case '\t':
            return Add(Enum::Tab);
        case '\n':
            return Add(Enum::NewLine);
        case '/':
            if (Peek() == '/') {
                Next();
                int start = offset;
                while (Next() != '\n');

                Token comment(Enum::Comment, *this, lineNumber,
                              Slice(start, offset));
                Add(comment);
                Next();
                return true;
            }
            return Add(Enum::Divide);

        case '-':
            if (Peek() == '-') return AddTwoCharOp(Enum::Decrement);
            if (Peek() == '=') return AddTwoCharOp(Enum::MinusAssign);
            return Add(Enum::Minus);

        case '.':
            if (Peek() == '.') {
                Next();
                if (Peek() == '.') {
                    Next();
                    return Add(Enum::Replace, 3);
                }
                return Fail("Two dots doesn't work");
            }
            return Add(Enum::Dot);

        case '+':
            if (Peek() == '+') return AddTwoCharOp(Enum::Increment);
            if (Peek() == '=') return AddTwoCharOp(Enum::PlusAssign);
            return Add(Enum::Plus);
    }

    LexError("Unrecognised %c");

    return false;
}

bool Contains(const char *allowed, char current);

// TODO: this is the same as PiLexer::PathnameOrKeyword(!)
bool RhoLexer::LexPathname() {
    int start = offset;
    bool quoted = Current() == '\'';
    if (quoted) Next();

    bool rooted = Current() == '/';
    if (rooted) Next();

    bool prevIdent = false;
    do {
        Token result = LexAlpha();

        if (result.type != TokenEnumType::Ident) {
            // this is actually a keyword
            if (quoted || rooted) {
                return false;
            }

            // keywords cannot be part of a path
            if (prevIdent) {
                return false;
            }

            Add(result);
            return true;
        }

        prevIdent = true;

        auto isSeparator = Contains(Pathname::Literals::AllButQuote, Current());
        if (isSeparator) {
            Next();
            continue;
        }

        if (!isalpha(Current())) {
            break;
        }
    } while (true);

    Add(Enum::Pathname, Slice(start, offset));

    return true;
}

void RhoLexer::Terminate() { Add(Enum::None, 0); }

KAI_END
