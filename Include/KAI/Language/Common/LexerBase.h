#pragma once

#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Common/Slice.h>

KAI_BEGIN

int IsSpaceChar(int ch);

class LexerBase : public ProcessCommon {
   public:
    typedef std::vector<std::string> Lines;

    LexerBase(const char *, Registry &r);

    const std::string &GetLine(size_t n) const {
        if (lines.empty() || n >= lines.size()) KAI_THROW_2(OutOfBounds, n, 0);
        return lines[n];
    }

    const Lines &GetLines() const { return lines; }
    const std::string &GetInput() const { return input; }
    int GetOffset() const { return offset; }
    int GetLineNumber() const { return offset; }
    const std::string &Line() const;
    std::string GetString(Slice const &slice) const {
        int length = slice.Length();
        return length == 0 ? "" : input.substr(slice.Start, length);
    }

   protected:
    Lines lines;
    std::string input;
    int offset, lineNumber;
    using ProcessCommon::reg_;

    void CreateLines();
    bool LexString();
    bool LexShellCommand();
    char Current() const;
    char Next();
    bool EndOfLine() const;
    char Peek() const;
    char PeekBase() const;

    virtual void LexErrorBase(const char *msg) = 0;
    virtual void AddStringToken(int lineNumber, Slice slice) = 0;
    virtual void AddShellCommandToken(int lineNumber, Slice slice) = 0;

    Slice Gather(int (*filter)(int ch));
};

KAI_END
