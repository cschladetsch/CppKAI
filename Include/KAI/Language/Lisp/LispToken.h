#pragma once

#include <KAI/Language/Common/TokenBase.h>

KAI_BEGIN

// Tokens for s-expression Lisp.
//
// Lisp has almost no surface syntax: `+`, `<`, `set!` and friends are ordinary
// symbols, not operators, so they all lex as Ident. That keeps this enum down
// to the handful of genuinely structural things, unlike Rho's.
//
// There is deliberately no ShellCommand token: Lisp does not get to run shell
// commands. LexerCommon::AddShellCommandToken degrades to a lex error for
// token enums that omit it.
struct LispTokenEnumType {
    enum Enum {
        None = 0,
        Whitespace,
        Tab,
        NewLine,
        Comment,
        OpenParan,
        CloseParan,
        Quote,
        Int,
        Float,
        String,
        True,
        False,
        Nil,
        Ident,
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
