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

    if (isalpha(current) || current == '_') {
        Add(LexAlpha());
        return true;  // parser will deal with keywords in wrong places
    }

    if (isdigit(current)) {
        // Implement number lexing directly here
        int start = offset;

        // Consume the number
        while (isdigit(Current())) {
            Next();
        }

        // Handle decimal point and fractional part
        if (Current() == '.') {
            Next();  // consume the dot
            while (isdigit(Current())) {
                Next();
            }
        }

        // Handle exponent notation
        if (Current() == 'e' || Current() == 'E') {
            Next();  // consume 'e' or 'E'

            // Handle optional sign
            if (Current() == '+' || Current() == '-') {
                Next();
            }

            // Parse exponent digits
            while (isdigit(Current())) {
                Next();
            }
        }

        return Add(Enum::Number, Slice(start, offset));
    }

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
        case '[':
            return Add(Enum::Array);  // Use Array token for array syntax
        case ']':
            return Add(Enum::ArrayProxy);  // Reuse ArrayProxy as close bracket
        case ':':
            // Handle visibility modifiers - simplified to avoid using Previous()
            // Just treat any colon as a semicolon equivalent
            return Add(Enum::Semi);  // Reuse semi token for colons in general
        case ',':
            return Add(Enum::Comma);
        case '=':
            return Add(Enum::Assign);
        case ' ':
            return Add(Enum::Whitespace, Gather(IsSpaceChar));
        case '"':
            return LexString();
        case '\t':
            return Add(Enum::Tab);
        case '\n':
            return Add(Enum::NewLine);
        case '/':
            if (Peek() == '/') {
                Next();
                int start = offset;
                while (Next() != '\n' && Current() != 0);
                return Add(Enum::Comment, offset - start);
            }

            return Fail("Expected comment start");
        case '<':
            // Handle template/generic syntax with better token handling
            {
                // Simplified approach for handling template parameters
                // Just treat the entire template syntax as an identifier
                int start = offset;
                int depth = 1;
                Next();  // consume '<'
                
                // Simple implementation that just captures the entire template parameter
                // and treats it as an identifier - we're not doing full template parsing yet
                while (depth > 0 && Current() != 0) {
                    if (Current() == '<') depth++;
                    if (Current() == '>') depth--;
                    if (depth > 0) Next();
                    else break;  // Don't consume the final '>'
                }
                
                // Consume the final '>'
                if (Current() == '>') Next();
                
                return Add(Enum::Ident, Slice(start, offset));
            }
        case '>':
            // This should only happen in isolation if there's a mistake in the
            // input
            return Add(Enum::Ident);
    }

    return LexError("Unrecognised %c");
}

void TauLexer::Terminate() { Add(Enum::None, 0); }

TAU_END
