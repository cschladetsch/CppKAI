#pragma once

#include <KAI/Language/Common/TokenBase.h>

KAI_BEGIN

// A token in the Rho language
struct RhoTokenEnumType {
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
        Label = 9,  // Renamed from Ident for consistency
        Dot = 10,
        Comma = 11,
        If = 12,
        Else = 13,
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
        BitNot = 81,
        LeftShift = 82,
        RightShift = 83,
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
        Assert = 65,
        ToPi = 66,
        PiSequence = 67,
        Pathname = 68,
        Debug = 69,
        Quote = 70,
        Sep = 71,
        AcrossAllNodes = 72,
        Mod = 73,
        Colon = 74,
        ModAssign = 75,
        DoubleColon = 76,
        While = 77,
        For = 78,
        DoWhile = 79,
        ForEach = 80,
        Break = 84,
        Continue = 85,
        ShellCommand = 86,  // shell command wrapped in backticks `command`
    };

    struct Type : TokenBase<RhoTokenEnumType> {
        Type() {}

        Type(Enum val, const LexerBase &lexer, int ln, Slice slice)
            : TokenBase<RhoTokenEnumType>(val, lexer, ln, slice) {}
    };

    static const char *ToString(Enum val);
};

typedef RhoTokenEnumType::Type RhoToken;
typedef RhoTokenEnumType RhoTokens;

KAI_END
