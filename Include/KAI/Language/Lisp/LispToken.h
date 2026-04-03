#pragma once

#include <KAI/Language/Common/TokenBase.h>

KAI_BEGIN

// A token in the Lisp language
struct LispTokenEnumType {
    enum Enum {
        None = 0,
        Whitespace = 1,
        Semi = 2,
        Int = 3,
        Float = 4,
        String = 5,
        True = 6,
        False = 7,
        Return = 8,
        Ident = 9,
        Dot = 10,
        Comma = 11,
        If = 12,
        Else = 13,
        For = 14,
        While = 15,
        OpenBrace = 16,
        CloseBrace = 17,
        OpenParan = 18,
        CloseParan = 19,
        Plus = 20,
        Minus = 21,
        Mul = 22,
        Divide = 23,
        Assign = 24,
        Less = 25,
        Equiv = 26,
        NotEquiv = 27,
        Greater = 28,
        LessEquiv = 29,
        GreaterEquiv = 30,
        Not = 31,
        And = 32,
        Or = 33,
        Xor = 34,
        OpenSquareBracket = 35,
        CloseSquareBracket = 36,
        Increment = 37,
        Decrement = 38,
        BitAnd = 39,
        BitOr = 40,
        BitXor = 41,
        Self = 42,
        Lookup = 43,
        Tab = 44,
        NewLine = 45,
        Fun = 46,
        Comment = 47,
        Yield = 48,
        Suspend = 49,
        Replace = 50,
        Resume = 51,
        PlusAssign = 60,
        MinusAssign = 61,
        MulAssign = 62,
        DivAssign = 63,
        In = 64,
        Assert = 65,
        ToPi = 66,
        PiSequence = 67,
        Pathname = 68,
        Debug = 69,
        Quote = 70,
        Sep = 71,
    };

    struct Type : TokenBase<LispTokenEnumType> {
        Type() {}

        Type(Enum val, const LexerBase &lexer, int ln, Slice slice)
            : TokenBase<LispTokenEnumType>(val, lexer, ln, slice) {}
    };

    static const char *ToString(Enum val);
};

typedef LispTokenEnumType::Type LispToken;
typedef LispTokenEnumType LispTokens;

KAI_END
