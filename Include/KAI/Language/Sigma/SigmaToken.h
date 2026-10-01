#pragma once

#include <KAI/Language/Common/TokenBase.h>

KAI_BEGIN

/// Tokens of Sigma, the statically typed KAI language. Sigma uses Rho's
/// indentation-based syntax plus type annotations, and compiles to Rho.
struct SigmaTokenEnumType {
    enum Enum {
        None = 0,
        End,      // end of input
        NewLine,  // end of a logical line (suppressed inside (), [] and {})
        Indent,
        Dedent,

        Int,
        Float,
        String,
        True,
        False,
        Name,

        // keywords
        Fun,
        Return,
        Yield,
        If,
        Else,
        While,
        Do,
        For,
        In,
        Break,
        Continue,
        Assert,
        Self,
        PiBlock,  // `pi { ... }` on one line, passed through to Rho verbatim

        // punctuation
        Dot,
        Comma,
        Colon,
        Semi,
        Question,
        Arrow,
        OpenParen,
        CloseParen,
        OpenSquare,
        CloseSquare,
        OpenBrace,
        CloseBrace,

        // operators
        Plus,
        Minus,
        Mul,
        Divide,
        Mod,
        Assign,
        PlusAssign,
        MinusAssign,
        MulAssign,
        DivAssign,
        ModAssign,
        Equiv,
        NotEquiv,
        Less,
        Greater,
        LessEquiv,
        GreaterEquiv,
        Not,
        And,
        Or,
        BitAnd,
        BitOr,
        BitXor,
        BitNot,
        LeftShift,
        RightShift,

        // required by LexerCommon; never produced by SigmaLexer
        ShellCommand,
        Ident,
    };

    struct Type : TokenBase<SigmaTokenEnumType> {
        Type() {}
        Type(Enum val, const LexerBase &lexer, int ln, Slice slice)
            : TokenBase<SigmaTokenEnumType>(val, lexer, ln, slice) {}
    };

    static const char *ToString(Enum val);
};

using SigmaToken = SigmaTokenEnumType::Type;

KAI_END
