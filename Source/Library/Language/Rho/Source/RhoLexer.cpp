#include <KAI/Language/Rho/RhoLexer.h>

#include <iostream>

using namespace std;

KAI_BEGIN

void RhoLexer::AddKeyWords() {
    // Basic keywords
    keyWords["if"] = Enum::If;
    keyWords["else"] = Enum::Else;
    keyWords["true"] = Enum::True;
    keyWords["false"] = Enum::False;
    keyWords["return"] = Enum::Return;
    keyWords["self"] = Enum::Self;
    keyWords["fun"] = Enum::Fun;
    keyWords["yield"] = Enum::Yield;
    keyWords["assert"] = Enum::Assert;

    // Pi language transitions
    keyWords["pi"] = Enum::ToPi;
    keyWords["pi{"] = Enum::PiSequence;

    // Iteration constructs
    keyWords["while"] = Enum::While;
    keyWords["for"] = Enum::For;
    keyWords["do"] = Enum::DoWhile;  // 'do' is recognized as DoWhile token
    keyWords["foreach"] = Enum::ForEach;
    keyWords["break"] = Enum::Break;
    keyWords["continue"] = Enum::Continue;
}

bool RhoLexer::NextToken() {
    char current = Current();
    if (current == 0) {
        return false;
    }

    // Debug output removed

    // Allow identifiers to start with either a letter or an underscore
    if (isalpha(current) || current == '_') {
        return LexPathname();
    }

    if (isdigit(current)) {
        // Collect the digits
        int start = offset;
        while (isdigit(Current())) {
            Next();
        }

        // Check for decimal point (floating point number)
        if (Current() == '.' && isdigit(Peek())) {
            Next();  // consume the '.'
            while (isdigit(Current())) {
                Next();
            }
            return Add(Enum::Float, Slice(start, offset));
        }

        return Add(Enum::Int, Slice(start, offset));
    }

    switch (current) {
        case '\'':
            return LexPathname();
        case '`':
#ifdef ENABLE_SHELL_SYNTAX
            return LexShellCommand();
#else
            Fail(
                "Shell syntax (backtick operations) is disabled for security. "
                "Enable with -DENABLE_SHELL_SYNTAX=ON");
            return false;
#endif
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
            if (Peek() == '=') return AddTwoCharOp(Enum::Equiv);
            return Add(Enum::Assign);
        case '!':
            if (Peek() == '=') return AddTwoCharOp(Enum::NotEquiv);
            return Add(Enum::Not);
        case '&':
            return AddIfNext('&', Enum::And, Enum::BitAnd);
        case '|':
            return AddIfNext('|', Enum::Or, Enum::BitOr);
        case '<':
            if (Peek() == '<') return AddTwoCharOp(Enum::LeftShift);
            if (Peek() == '=') return AddTwoCharOp(Enum::LessEquiv);
            return Add(Enum::Less);
        case '>':
            if (Peek() == '>') return AddTwoCharOp(Enum::RightShift);
            if (Peek() == '=') return AddTwoCharOp(Enum::GreaterEquiv);
            return Add(Enum::Greater);
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

        case '%':
            if (Peek() == '=') return AddTwoCharOp(Enum::ModAssign);
            return Add(Enum::Mod);

        case '^':
            return Add(Enum::BitXor);

        case '~':
            return Add(Enum::BitNot);

        case ':':
            if (Peek() == ':') return AddTwoCharOp(Enum::DoubleColon);
            return Add(Enum::Colon);
    }

    LexError("Unrecognised character");

    return false;
}

bool Contains(const char* allowed, char current);

// TODO: this is the same as PiLexer::PathnameOrKeyword(!)
bool RhoLexer::LexPathname() {
    // Store the current position for later use
    int start = offset;

    // Check if this is a quoted pathname
    bool quoted = Current() == '\'';
    if (quoted) Next();

    // Check if this is a rooted pathname
    bool rooted = Current() == '/';
    if (rooted) Next();

    // If this is not quoted or rooted, it might be a keyword or identifier
    if (!quoted && !rooted && (isalpha(Current()) || Current() == '_')) {
        // Save the start position of the word
        int wordStart = offset;
        std::string word;

        // Collect the entire word
        while (isalnum(Current()) || Current() == '_') {
            word += Current();
            Next();
        }

        // Check if it's a keyword - simplified logic to just use the keyWords
        // map
        auto it = keyWords.find(word);
        if (it != keyWords.end()) {
            // Add the token with the appropriate enum type
            return Add(it->second, Slice(wordStart, offset));
        }

        // Not a keyword, it's a regular identifier
        return Add(Enum::Label, Slice(wordStart, offset));
    }

    // If it's a quoted or rooted path, process it as a pathname
    if (quoted || rooted) {
        bool prevIdent = false;

        do {
            if (isalpha(Current()) || Current() == '_') {
                // Gather the identifier part
                while (isalnum(Current()) || Current() == '_') {
                    Next();
                }
                prevIdent = true;
            }

            if (Contains(Pathname::Literals::AllButQuote, Current())) {
                Next();
                continue;
            }

            break;
        } while (true);

        return Add(Enum::Pathname, Slice(start, offset));
    }

    // If we get here, it's probably an error or an empty identifier
    return Add(Enum::Label, Slice(start, offset));
}

void RhoLexer::Terminate() { Add(Enum::None, 0); }

KAI_END
