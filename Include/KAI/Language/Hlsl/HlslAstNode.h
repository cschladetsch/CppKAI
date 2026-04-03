#pragma once

#include "KAI/Language/Common/AstNodeBase.h"
#include "KAI/Language/Hlsl/HlslToken.h"

KAI_BEGIN

struct HlslAstNodeEnumType {
    enum Enum : int {
        None = 0,
        Program = 1,
        Ident = 2,
        GetMember = 3,
        Function = 4,
        Block = 5,
        Conditional = 6,
        Assignment = 7,
        Call = 8,
        Increment = 9,
        Decrement = 10,
        TokenType = 11,
        Positive = 12,
        Negative = 13,
        ArgList = 14,
        IndexOp = 15,
        ForEach = 16,
        For = 17,
        List = 18,
        Map = 19,
        Plus = 20,
        Minus = 21,
        Mul = 22,
        Divide = 23,
        Less = 24,
        Equiv = 25,
        NotEquiv = 26,
        Greater = 27,
        Object = 28,
        Continuation = 29,
        Pathname = 30,
        Home = 31,
        Declaration,
    };

    struct Node : AstNodeBase<HlslToken, HlslAstNodeEnumType> {
        typedef AstNodeBase<HlslToken, HlslAstNodeEnumType> Parent;
        using typename Parent::Token;
        KAI_NAMESPACE(Object) object;

        Node() : Parent(Enum::None) {}
        Node(Enum e) : Parent(e) {}
        Node(Enum e, Token &tok) : Parent(e, tok) {}
        Node(Parent::Token tok) : Parent(tok) {}
    };

    static const char *ToString(Enum val);
};

typedef HlslAstNodeEnumType HlslAstNodes;
typedef HlslAstNodes::Node HlslAstNode;

KAI_END
