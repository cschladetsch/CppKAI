#include <KAI/Language/Pi/PiLexer.h>

#include <iostream>
#include <map>

using namespace std;

KAI_BEGIN

namespace {
void PopulateKeywords(std::map<std::string, PiTokenEnumType::Enum> &keywords) {
    keywords["if"] = PiTokenEnumType::If;
    keywords["ife"] = PiTokenEnumType::IfElse;
    keywords["for"] = PiTokenEnumType::For;
    keywords["foreach"] = PiTokenEnumType::ForEach;
    keywords["break"] = PiTokenEnumType::Break;
    keywords["continue"] = PiTokenEnumType::Continue;
    keywords["true"] = PiTokenEnumType::True;
    keywords["false"] = PiTokenEnumType::False;
    keywords["self"] = PiTokenEnumType::Self;
    keywords["while"] = PiTokenEnumType::While;
    keywords["assert"] = PiTokenEnumType::Assert;
    keywords["div"] = PiTokenEnumType::Divide;
    keywords["rho"] = PiTokenEnumType::ToRho;
    keywords["rho{"] = PiTokenEnumType::ToRhoSequence;
    keywords["to_str"] = PiTokenEnumType::ToStr;

    keywords["not"] = PiTokenEnumType::Not;
    keywords["and"] = PiTokenEnumType::And;
    keywords["or"] = PiTokenEnumType::Or;
    keywords["xor"] = PiTokenEnumType::Xor;
    keywords["exists"] = PiTokenEnumType::Exists;

    keywords["drop"] = PiTokenEnumType::Drop;
    keywords["dup"] = PiTokenEnumType::Dup;
    keywords["dup2"] = PiTokenEnumType::Dup2;
    keywords["drop2"] = PiTokenEnumType::Drop2;
    keywords["pick"] = PiTokenEnumType::PickN;
    keywords["over"] = PiTokenEnumType::Over;
    keywords["swap"] = PiTokenEnumType::Swap;
    keywords["rot"] = PiTokenEnumType::Rot;
    keywords["rotn"] = PiTokenEnumType::RotN;
    keywords["roll"] = PiTokenEnumType::Roll;
    keywords["min"] = PiTokenEnumType::Min;
    keywords["max"] = PiTokenEnumType::Max;
    keywords["toarray"] = PiTokenEnumType::ToArray;
    keywords["gc"] = PiTokenEnumType::GarbageCollect;
    keywords["clear"] = PiTokenEnumType::Clear;
    keywords["expand"] = PiTokenEnumType::Expand;
    keywords["cd"] = PiTokenEnumType::ChangeFolder;
    keywords["pwd"] = PiTokenEnumType::PrintFolder;
    keywords["type"] = PiTokenEnumType::GetType;
    keywords["size"] = PiTokenEnumType::Size;
    keywords["depth"] = PiTokenEnumType::Depth;
    keywords["new"] = PiTokenEnumType::New;
    keywords["print"] = PiTokenEnumType::Print;
    keywords["dropn"] = PiTokenEnumType::DropN;
    keywords["setchild"] = PiTokenEnumType::SetChild;

    keywords["toarray"] = PiTokenEnumType::ToArray;
    keywords["tolist"] = PiTokenEnumType::ToList;
    keywords["tomap"] = PiTokenEnumType::ToMap;
    keywords["toset"] = PiTokenEnumType::ToSet;

    keywords["div"] = PiTokenEnumType::Divide;
    keywords["mod"] = PiTokenEnumType::Modulo;

    keywords["expand"] = PiTokenEnumType::Expand;
    keywords["noteq"] = PiTokenEnumType::NotEquiv;
    keywords["lls"] = PiTokenEnumType::Contents;
    keywords["ls"] = PiTokenEnumType::GetContents;
    keywords["freeze"] = PiTokenEnumType::Freeze;
    keywords["thaw"] = PiTokenEnumType::Thaw;
    keywords["send"] = PiTokenEnumType::Send;
    keywords["call"] = PiTokenEnumType::Suspend;
    keywords["at"] = PiTokenEnumType::GetChild;
}
}  // namespace

void PiLexer::AddKeyWords() { PopulateKeywords(keyWords); }

bool PiLexer::TryGetKeyword(const std::string &text,
                            PiTokenEnumType::Enum &out) {
    static const std::map<std::string, PiTokenEnumType::Enum> keywords = [] {
        std::map<std::string, PiTokenEnumType::Enum> map;
        PopulateKeywords(map);
        return map;
    }();

    auto it = keywords.find(text);
    if (it == keywords.end()) return false;

    out = it->second;
    return true;
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
                // Save the start position (first dot)
                int start = offset;
                Next();  // Move past second dot
                if (Peek() == '.') {
                    // Three dots - create Resume token from saved start
                    Next();  // Move past third dot
                    return Add(Enum::Resume, Slice(start, offset + 1));
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

            KAI_TRACE() << "[PiLexer] Adding keyword token: type="
                        << result.type;
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

    auto pathText = Slice(start, offset);
    std::string pathStr(input.begin() + start, input.begin() + offset);
    KAI_TRACE() << "[PiLexer] Adding Pathname token: '" << pathStr << "'";
    Add(Enum::Pathname, pathText);

    return true;
}

KAI_END
