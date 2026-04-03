#include "KAI/Language/Common/LexerBase.h"

KAI_BEGIN

int IsSpaceChar(int ch) { return ch == ' '; }

LexerBase::LexerBase(const char *in, Registry &r)
    : ProcessCommon(r), input(in), offset(0), lineNumber(0) {}

void LexerBase::CreateLines() {
    if (input.back() != '\n') input.push_back('\n');

    size_t lineStart = 0;
    for (size_t n = 0; n < input.size(); ++n) {
        if (input[n] == '\n') {
            lines.push_back(input.substr(lineStart, n - lineStart + 1));
            lineStart = n + 1;
        }
    }
}

char LexerBase::Current() const {
    if (lineNumber == (int)lines.size()) return 0;

    return Line()[offset];
}

const std::string &LexerBase::Line() const { return GetLine(lineNumber); }

bool LexerBase::EndOfLine() const {
    auto len = (int)Line().size();
    return len == 0 || offset == (int)Line().size() - 1;
}

char LexerBase::Peek() const {
    if (!Current()) return 0;

    if (EndOfLine()) return 0;

    return Line()[offset + 1];
}

Slice LexerBase::Gather(int (*filt)(int)) {
    int start = offset;
    while (filt(Next()));

    return Slice(start, offset);
}

char LexerBase::Next() {
    if (EndOfLine()) {
        offset = 0;
        ++lineNumber;
    } else
        ++offset;

    if (lineNumber == (int)lines.size()) return 0;

    return Line()[offset];
}

bool LexerBase::LexString() {
    int start = offset;
    Next();
    while (!Failed && Current() != '"') {
        if (Current() == '\\') {
            switch (Next()) {
                case '"':
                case 'n':
                case 't':
                    break;

                default:
                    LexErrorBase("Bad escape sequence %c");
                    return false;
            }
        }

        if (Peek() == 0) {
            Fail("Bad string literal");
            return false;
        }

        Next();
    }

    Next();

    AddStringToken(lineNumber, Slice(start + 1, offset - 1));

    return true;
}

bool LexerBase::LexShellCommand() {
#ifdef ENABLE_SHELL_SYNTAX
    int start = offset;
    Next();

    while (!Failed && Current() != '`') {
        if (Current() == 0) {
            Fail("Unterminated shell command");
            return false;
        }

        if (Current() == '\\') {
            char nextChar = Peek();
            if (nextChar == '`' || nextChar == '\\') {
                Next();
                if (Current() != 0) {
                    Next();
                }
            } else {
                Next();
            }
        } else {
            Next();
        }
    }

    if (Current() == '`') {
        Next();
    }

    AddShellCommandToken(lineNumber, Slice(start + 1, offset - 1));

    return true;
#else
    Fail(
        "Shell syntax (backtick operations) is disabled for security. Enable "
        "with -DENABLE_SHELL_SYNTAX=ON");
    return false;
#endif
}

KAI_END
