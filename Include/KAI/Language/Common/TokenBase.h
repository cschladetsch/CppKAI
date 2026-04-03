#pragma once

#include <KAI/Language/Common/LexerBase.h>

#include <sstream>

KAI_BEGIN

// common to all Tokens
template <class EEnumType>
class TokenBase {
   public:
    typedef EEnumType EnumType;
    typedef TokenBase<EnumType> Self;
    typedef typename EnumType::Enum Enum;

    Enum type;
    Slice slice;
    int lineNumber;
    const LexerBase *lexer;

    TokenBase() : lexer(0), lineNumber(0), type((Enum)0) {}

    TokenBase(Enum val, const LexerBase &lexer, int ln, Slice slice)
        : type(val), lexer(&lexer), lineNumber(ln), slice(slice) {}

    char operator[](int n) const { return lexer->GetInput()[slice.Start + n]; }

    std::string ToString() const {
        std::stringstream out;
        String ty = EEnumType::ToString(type);
        String text = Text();
        out << ty.c_str();
        if (ty != text && !text.empty())  // && !isspace(text[0]) &&
                                          // text.tolower() != ty.tolower())
        {
            out << " '" << text.c_str() << "'";
        }

        return out.str();
    }

    std::string Text() const {
        if (lexer == 0) return "";

        if (slice.Length() == 0) return "";

        return lexer->GetLine(lineNumber).substr(slice.Start, slice.Length());
    }

    friend std::ostream &operator<<(std::ostream &out, Self const &node) {
        return out << node.ToString();
    }
};

KAI_END
