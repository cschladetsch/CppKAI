#include <KAI/Language/Tau/TauLexer.h>

using namespace std;

TAU_BEGIN

void TauLexer::AddKeyWords() {
    keyWords["namespace"] = Enum::Namespace;
    keyWords["class"] = Enum::Class;
    keyWords["sync"] = Enum::Sync;
    keyWords["async"] = Enum::Async;
    keyWords["Proxy"] = Enum::Proxy;
}

bool TauLexer::NextToken() {
    char current = Current();
    if (current == 0) return false;

    if (isalpha(current)) {
        Add(LexAlpha());
        return true;  // parser will deal with keywords in wrong places
    }

    if (isdigit(current)) return Fail("Number not expected");

    switch (current) {
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
        case ',':
            return Add(Enum::Comma);
        case ' ':
            return Add(Enum::Whitespace, Gather(IsSpaceChar));
        case '"':
            return LexString();
        // case '\'': return LexAlpha();
        case '\t':
            return Add(Enum::Tab);
        case '\n':
            return Add(Enum::NewLine);
        case '/':
            if (Peek() == '/') {
                Next();
                int start = offset;
                while (Next() != '\n');
                return Add(Enum::Comment, offset - start);
            }

            return Fail("Expected comment start");
    }

    return LexError("Unrecognised %c");
}

void TauLexer::Terminate() { Add(Enum::None, 0); }

TAU_END
