#include <KAI/Language/Pi/PiLexer.h>

using namespace std;

KAI_BEGIN

void PiLexer::AddKeyWords() {
    keyWords["if"] = Enum::If;
    keyWords["ife"] = Enum::IfElse;
    keyWords["for"] = Enum::For;
    keyWords["true"] = Enum::True;
    keyWords["false"] = Enum::False;
    keyWords["self"] = Enum::Self;
    keyWords["while"] = Enum::While;
    keyWords["assert"] = Enum::Assert;
    keyWords["div"] = Enum::Divide;
    keyWords["rho"] = Enum::ToRho;
    keyWords["rho{"] = Enum::ToRhoSequence;
    keyWords["to_str"] = Enum::ToStr;

    keyWords["not"] = Enum::Not;
    keyWords["and"] = Enum::And;
    keyWords["or"] = Enum::Or;
    keyWords["xor"] = Enum::Xor;
    keyWords["exists"] = Enum::Exists;

    keyWords["drop"] = Enum::Drop;
    keyWords["dup"] = Enum::Dup;
    keyWords["dup2"] = Enum::Dup2;
    keyWords["drop2"] = Enum::Drop2;
    keyWords["pick"] = Enum::PickN;
    keyWords["over"] = Enum::Over;
    keyWords["swap"] = Enum::Swap;
    keyWords["rot"] = Enum::Rot;
    keyWords["rotn"] = Enum::RotN;
    keyWords["roll"] = Enum::Roll;
    keyWords["min"] = Enum::Min;
    keyWords["max"] = Enum::Max;
    keyWords["toarray"] = Enum::ToArray;
    keyWords["gc"] = Enum::GarbageCollect;
    keyWords["clear"] = Enum::Clear;
    keyWords["expand"] = Enum::Expand;
    keyWords["cd"] = Enum::ChangeFolder;
    keyWords["pwd"] = Enum::PrintFolder;
    keyWords["type"] = Enum::GetType;
    keyWords["size"] = Enum::Size;
    keyWords["depth"] = Enum::Depth;
    keyWords["new"] = Enum::New;
    keyWords["print"] = Enum::Print;
    keyWords["dropn"] = Enum::DropN;

    keyWords["toarray"] = Enum::ToArray;
    keyWords["tolist"] = Enum::ToList;
    keyWords["tomap"] = Enum::ToMap;
    keyWords["toset"] = Enum::ToSet;

    keyWords["div"] = Enum::Divide;
    keyWords["mul"] = Enum::Mul;
    keyWords["mod"] = Enum::Modulo;

    keyWords["expand"] = Enum::Expand;
    keyWords["noteq"] = Enum::NotEquiv;
    keyWords["lls"] = Enum::Contents;
    keyWords["ls"] = Enum::GetContents;
    keyWords["freeze"] = Enum::Freeze;
    keyWords["thaw"] = Enum::Thaw;
}

bool PiLexer::NextToken() {
    char current = Current();
    if (current == 0) return false;

    if (isalpha(current)) return PathnameOrKeyword();

    if (isdigit(current)) {
        // Parse number - could be int or float
        int start = offset;
        Gather(isdigit);  // Collect initial digits

        // Check for decimal point followed by digits
        if (Current() == '.' && isdigit(Peek())) {
            Next();           // Skip '.'
            Gather(isdigit);  // Collect fractional digits
            return Add(Enum::Float, Slice(start, offset));
        }

        return Add(Enum::Int, Slice(start, offset));
    }

    switch (current) {
        case '\'':
            return PathnameOrKeyword();
        case '`':
#ifdef ENABLE_SHELL_SYNTAX
            return LexShellCommand();
#else
            Fail(
                "Shell syntax (backtick operations) is disabled for security. "
                "Enable with -DENABLE_SHELL_SYNTAX=ON");
            return false;
#endif
        case '{':
            return Add(Enum::OpenBrace);
        case '}':
            return Add(Enum::CloseBrace);
        case '(':
            return Add(Enum::OpenParan);
        case ')':
            return Add(Enum::CloseParan);
        case ':':
            return Add(Enum::Colon);
        case ' ':
            return Add(Enum::Whitespace, Gather(IsSpaceChar));
        case '@':
            return Add(Enum::Lookup);
        case ',':
            return Add(Enum::Comma);
        case '#':
            return Add(Enum::Store);
        case '*':
            return Add(Enum::Mul);
        case '[':
            return Add(Enum::OpenSquareBracket);
        case ']':
            return Add(Enum::CloseSquareBracket);
        case '=':
            return AddIfNext('=', Enum::Equiv, Enum::Assign);
        case '!':
            return AddIfNext('=', Enum::NotEquiv, Enum::Replace);
        case '&':
            return AddIfNext('&', Enum::And, Enum::Suspend);
        case '|':
            return AddIfNext('|', Enum::Or, Enum::BitOr);
        case '<':
            return AddIfNext('=', Enum::LessEquiv, Enum::Less);
        case '>':
            return AddIfNext('=', Enum::GreaterEquiv, Enum::Greater);
        case '"':
            return LexString();  // "comment to unfuck Visual Studio Code's
                                 // syntax hilighter
        case '\t':
            return Add(Enum::Tab);
        case '\n':
            return Add(Enum::NewLine);
        case '-':
            if (Peek() == '-') return AddTwoCharOp(Enum::Decrement);
            if (Peek() == '=') return AddTwoCharOp(Enum::MinusAssign);

            // Check if this is a negative number literal
            if (isdigit(Peek())) {
                // This is a negative number, parse it as such
                int start = offset;
                Next();           // Skip the minus sign
                Gather(isdigit);  // Collect digits

                // Check for decimal point followed by digits
                if (Current() == '.' && isdigit(Peek())) {
                    Next();           // Skip '.'
                    Gather(isdigit);  // Collect fractional digits
                    return Add(Enum::Float, Slice(start, offset));
                }

                return Add(Enum::Int, Slice(start, offset));
            }

            return Add(Enum::Minus);

        case '.':
            if (Peek() == '.') {
                Next();
                if (Peek() == '.') {
                    Next();
                    return Add(Enum::Resume, 3);
                }
                return Fail("Two dots doesn't work");
            }
            return Add(Enum::Self);

        case '+':
            if (Peek() == '+') return AddTwoCharOp(Enum::Increment);
            if (Peek() == '=') return AddTwoCharOp(Enum::PlusAssign);
            return Add(Enum::Plus);

        case '%':
            return Add(Enum::Modulo);

        case '/':
            if (Peek() == '/') {
                Next();
                const int start = offset;
                while (Next() != '\n');

                Add(Token(Enum::Comment, *this, lineNumber,
                          Slice(start, offset)));
                Next();
                return true;
            }
            return Add(Enum::Divide);
    }

    LexError("Unrecognised %c");

    return false;
}

void PiLexer::Terminate() { Add(Enum::None, 0); }

bool Contains(const char *allowed, char current) {
    for (const char *a = allowed; *a; ++a) {
        if (current == *a) return true;
    }

    return false;
}

// TODO: this isn't a full pathname . See Pathname.cpp in Core
bool PiLexer::PathnameOrKeyword() {
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

        if (isspace(Current())) {
            break;
        }
    } while (true);

    Add(Enum::Pathname, Slice(start, offset));

    return true;
}

KAI_END
