#pragma once

#include "KAI/Language/Common/AstNodeBase.h"
#include "KAI/Language/Lisp/LispToken.h"

KAI_BEGIN

// The Lisp AST is deliberately tiny: everything is either an atom or a list.
// Special forms are not given their own node types -- they are recognised by
// the translator from the head of a List, which is what makes the grammar
// extensible without touching the parser.
struct LispAstNodeEnumType {
    enum Enum : int {
        None = 0,
        // A leaf atom carrying its token. AstNodeBase's token constructor
        // hard-codes this name, so it has to exist.
        TokenType,
        Program,
        List,
        Quote,
        Object,
    };

    struct Node : AstNodeBase<LispToken, LispAstNodeEnumType> {
        typedef AstNodeBase<LispToken, LispAstNodeEnumType> Parent;
        using typename Parent::Token;
        KAI_NAMESPACE(Object) object;

        Node() : Parent(Enum::None) {}
        Node(Enum e) : Parent(e) {}
        Node(Enum e, Token &tok) : Parent(e, tok) {}
        Node(Parent::Token tok) : Parent(tok) {}
    };

    static const char *ToString(Enum val);
};

typedef LispAstNodeEnumType LispAstNodes;
typedef LispAstNodes::Node LispAstNode;

KAI_END
