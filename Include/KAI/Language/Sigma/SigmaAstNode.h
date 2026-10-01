#pragma once

#include <KAI/Language/Common/AstNodeBase.h>
#include <KAI/Language/Sigma/SigmaToken.h>

KAI_BEGIN

/// Sigma AST. Layouts (children in order):
///
///   Program      statements...
///   Block        statements...
///   Function     (token: name)    Params, Type|None (result), Block
///   Params       Param...
///   Param        (token: name)    Type
///   Type         (token: name | 'fun')
///                  named: type arguments...
///                  fun:   None(parameter Types...), Type|None (result)
///   Declaration  (token: name)    Type, value
///   Assignment   (token: = += -= *= /= %=)  target, value
///   If           condition, Block, [Block | If]
///   While        condition, Block
///   DoWhile      Block, condition
///   For          init|None, condition|None, step|None, Block
///   ForEach      (token: variable)  iterable, Block
///   Return       [value]
///   Break, Continue
///   Assert       condition
///   ExprStatement  expression
///   Binary       (token: operator)  left, right
///   Unary        (token: operator)  operand
///   Ternary      condition, then, else
///   Call         callee, Args
///   Args         arguments...
///   Index        container, index
///   Member       (token: member name)  object
///   List         elements...
///   Map          MapEntry...
///   MapEntry     (token: key string)  value
///   PiBlock      (token: whole `pi { ... }` text)
///   TokenType    literals and names
struct SigmaAstNodeEnumType {
    enum Enum {
        None = 0,
        TokenType,
        Object,
        Program,
        Block,
        Function,
        Params,
        Param,
        Type,
        Declaration,
        Assignment,
        If,
        While,
        DoWhile,
        For,
        ForEach,
        Return,
        Break,
        Continue,
        Assert,
        ExprStatement,
        Binary,
        Unary,
        Ternary,
        Call,
        Args,
        Index,
        Member,
        List,
        Map,
        MapEntry,
        PiBlock,
    };

    static const char *ToString(Enum val);
};

using SigmaAstNode = AstNodeBase<SigmaToken, SigmaAstNodeEnumType>;
using SigmaAstNodePtr = std::shared_ptr<SigmaAstNode>;

KAI_END
