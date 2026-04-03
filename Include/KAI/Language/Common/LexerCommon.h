#pragma once

#include <KAI/Language/Common/LexerBase.h>
#include <KAI/Language/Common/Process.h>
#include <KAI/Language/Common/Slice.h>
#include <stdarg.h>

#include <algorithm>
#include <sstream>

#ifdef KAI_USE_MONOTONIC_ALLOCATOR
#include <boost/monotonic/monotonic.hpp>
#endif

KAI_BEGIN

template <class EnumType>
class LexerCommon : public LexerBase {
   public:
    typedef typename EnumType::Type Token;
    typedef typename EnumType::Enum Enum;
    typedef EnumType TokenEnumType;

#ifdef KAI_USE_MONOTONIC_ALLOCATOR
    typedef boost::monotonic::vector<Token> Tokens;
    typedef boost::monotonic::vector<std::string> Lines;
    typedef boost::monotonic::map<std::string, Token::Type> Keywords;
#else
    typedef std::vector<Token> Tokens;
    typedef std::map<std::string, Enum> Keywords;
#endif

    LexerCommon(const char *input, Registry &r) : LexerBase(input, r) {}

    bool Process() {
        AddKeyWords();
        CreateLines();
        return Run();
    }

    virtual void AddKeyWords() = 0;
    virtual bool NextToken() = 0;
    virtual void Terminate() = 0;

    const Tokens &GetTokens() const { return tokens; }

   protected:
    Tokens tokens;
    Keywords keyWords;
    using LexerBase::reg_;

    bool Run() {
        offset = 0;
        lineNumber = 0;

        while (!Failed && NextToken());

        Terminate();

        return !Failed;
    }

    Token LexAlpha() {
        auto isIdentChar = [](int ch) -> int {
            return isalnum(ch) || ch == '_';
        };

        Token tok(Enum::Ident, *this, lineNumber, Gather(isIdentChar));
        auto kw = keyWords.find(tok.Text());
        auto keyword = kw != keyWords.end();
        if (keyword) tok.type = kw->second;

        return tok;
    }

    void AddStringToken(int lineNumber, Slice slice) override {
        tokens.push_back(Token(Enum::String, *this, lineNumber, slice));
    }

    void AddShellCommandToken(int lineNumber, Slice slice) override {
        tokens.push_back(Token(Enum::ShellCommand, *this, lineNumber, slice));
    }

    void LexErrorBase(const char *msg) override { LexError(msg); }

    bool Add(Token const &tok) {
        tokens.push_back(tok);
        return true;
    }

    bool Add(Enum type, Slice slice) {
        tokens.push_back(Token(type, *this, lineNumber, slice));
        return true;
    }

    bool Add(Enum type, int len = 1) {
        Add(type, Slice(offset, offset + len));
        while (len--) Next();

        return true;
    }

    bool AddIfNext(char ch, Enum thentype, Enum elseType) {
        if (Peek() == ch) {
            Next();
            return Add(thentype, 2);
        }

        return Add(elseType, 1);
    }

    bool AddTwoCharOp(Enum ty) {
        Add(ty, 2);
        Next();

        return true;
    }

    bool AddThreeCharOp(Enum ty) {
        Add(ty, 3);
        Next();
        Next();

        return true;
    }

    bool LexError(const char *text) {
        return Fail(CreateErrorMessage(Token(static_cast<Enum>(0), *this,
                                             lineNumber, Slice(offset, offset)),
                                       text, Current()));
    }

   public:
    static std::string CreateErrorMessage(Token tok, const char *fmt, ...) {
        char buff0[4096];
        va_list ap;
        va_start(ap, fmt);
#ifdef WIN32
        vsprintf_s(buff0, sizeof(buff0), fmt, ap);
#else
        vsnprintf(buff0, sizeof(buff0), fmt, ap);
#endif

        const char *fmt1 = "%s(%d):[%d]: %s\n";
        char buff[8192];
#ifdef WIN32
        sprintf_s(buff, sizeof(buff), fmt1, "", tok.lineNumber, tok.slice.Start,
                  buff0);
#else
        snprintf(buff, sizeof(buff), fmt1, "", tok.lineNumber, tok.slice.Start,
                 buff0);
#endif
        int beforeContext = 2;
        int afterContext = 2;

        const LexerBase &lex = *tok.lexer;
        int start = std::max(0, tok.lineNumber - beforeContext);
        int end = std::min((int)lex.GetLines().size() - 1,
                           tok.lineNumber + afterContext);

        std::stringstream err;
        err << buff << std::endl;
        for (int n = start; n <= end; ++n) {
            for (auto ch : lex.GetLine(n)) {
                if (ch == '\t')
                    err << "    ";
                else
                    err << ch;
            }

            if (n == tok.lineNumber) {
                for (int ch = 0; ch < (int)lex.GetLine(n).size(); ++ch) {
                    if (ch == tok.slice.Start) {
                        err << '^';
                        break;
                    }

                    auto c = lex.GetLine(tok.lineNumber)[ch];
                    if (c == '\t')
                        err << "    ";
                    else
                        err << ' ';
                }

                err << std::endl;
            }
        }

        return err.str();
    }

    std::string Print() const {
        std::stringstream str;
        for (const auto &tok : tokens) {
            str << tok << ", ";
        }
        return str.str();
    }
};

KAI_END
