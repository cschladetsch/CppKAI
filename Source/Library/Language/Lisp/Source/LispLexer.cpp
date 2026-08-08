#include <KAI/Language/Lisp/LispLexer.h>

#include <cctype>
#include <cstdlib>
#include <string>

KAI_BEGIN

// Lisp has no keywords in the lexical sense. `if`, `define`, `lambda` and the
// rest are just symbols; the translator decides what they mean based on where
// they appear. Leaving this empty is the point, not an omission.
void LispLexer::AddKeyWords() {}

bool LispLexer::IsDelimiter(char ch) {
    return ch == 0 || ch == '(' || ch == ')' || ch == '\'' || ch == '"' ||
           ch == ';' || isspace(static_cast<unsigned char>(ch));
}

namespace {
// Does this atom's text denote a number rather than a symbol? `-` and `+` are
// symbols on their own but start a number when followed by a digit, and `1+`
// is a legal symbol name in most Lisps, so the whole text has to parse.
bool LooksNumeric(const std::string &text, bool &isFloat) {
    if (text.empty()) return false;

    size_t i = 0;
    if (text[0] == '-' || text[0] == '+') i = 1;
    if (i >= text.size()) return false;  // bare "-" or "+" is a symbol

    bool sawDigit = false;
    bool sawDot = false;
    for (; i < text.size(); ++i) {
        const char ch = text[i];
        if (isdigit(static_cast<unsigned char>(ch))) {
            sawDigit = true;
            continue;
        }
        if (ch == '.' && !sawDot) {
            sawDot = true;
            continue;
        }
        return false;  // any other character makes it a symbol
    }

    isFloat = sawDot;
    return sawDigit;
}
}  // namespace

bool LispLexer::LexAtom() {
    const int start = offset;
    while (!IsDelimiter(Current())) Next();

    if (offset == start) {
        LexError("Empty atom");
        return false;
    }

    // Slices are columns within the current line, not offsets into the whole
    // input -- LexerBase::Next() resets offset at each line break, and
    // TokenBase::Text() resolves them against GetLine(lineNumber). An atom
    // always stops before the newline (it is a delimiter), so start and offset
    // are guaranteed to be on the same line here.
    const Slice slice(start, offset);
    const std::string text = Line().substr(start, offset - start);

    bool isFloat = false;
    if (LooksNumeric(text, isFloat)) {
        return Add(isFloat ? Enum::Float : Enum::Int, slice);
    }

    // Booleans and nil are spelled several ways across Lisp dialects; accept
    // the common ones so tests can be written naturally.
    if (text == "#t" || text == "true") return Add(Enum::True, slice);
    if (text == "#f" || text == "false") return Add(Enum::False, slice);
    if (text == "nil" || text == "'()") return Add(Enum::Nil, slice);

    return Add(Enum::Ident, slice);
}

bool LispLexer::NextToken() {
    const char current = Current();
    if (current == 0) return false;

    switch (current) {
        case ' ':
            return Add(Enum::Whitespace, Gather(IsSpaceChar));
        case '\t':
            return Add(Enum::Tab);
        case '\r':
            Next();
            return true;
        case '\n':
            return Add(Enum::NewLine);
        case '(':
            return Add(Enum::OpenParan);
        case ')':
            return Add(Enum::CloseParan);
        case '\'':
            return Add(Enum::Quote);
        case '"':
            return LexString();
        case '`':
            Fail("Shell syntax is not supported in Lisp");
            return false;
        case ';': {
            // Comment runs to end of line. Don't consume the newline itself:
            // the parser treats it as ordinary trivia.
            const int start = offset;
            while (Current() != '\n' && Current() != 0) Next();
            return Add(Token(Enum::Comment, *this, lineNumber,
                             Slice(start, offset)));
        }
    }

    return LexAtom();
}

void LispLexer::Terminate() { Add(Enum::None, 0); }

KAI_END
