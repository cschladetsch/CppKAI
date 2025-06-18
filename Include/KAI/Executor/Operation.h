#pragma once

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Type/TraitMacros.h"

KAI_BEGIN

struct Operation {
    enum Type {
        Return,
        SuspendNew,
        None,
        True,
        False,
        Equiv,
        NotEquiv,
        Less,
        Greater,
        LessOrEquiv,
        GreaterOrEquiv,
        Suspend,
        Replace,
        Resume,
        CppFunctionCall,
        CppMethodCall,
        Plus,
        Minus,
        Multiply,
        Divide,
        Modulo,
        TypeOf,

        Store,
        Retreive,
        Remove,

        New,
        Assign,

        Freeze,
        Thaw,

        GetScope,
        ChangeScope,

        GetChildren,
        Contents,
        Break,
        Continue,
        Drop,
        DropN,
        Swap,
        Dup,
        Dup2,
        Drop2,
        Rot,
        RotN,
        Roll,
        Pick,
        Clear,
        Depth,
        Over,
        Min,
        Max,
        ToArray,
        ToList,
        ToMap,
        ToSet,
        ToHashMap,
        ToPair,
        ToVector2,
        ToVector3,
        ToVector4,
        Expand,
        Name,
        Fullname,

        LogicalNot,
        LogicalAnd,
        LogicalOr,
        LogicalXor,
        LogicalNand,
        BitwiseNot,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseNand,
        LeftShift,
        RightShift,
        Lookup,
        TraceAll,
        Trace,
        This,
        Self,
        Ref,
        Detach,
        Delete,
        Exists,
        Pin,
        Unpin,
        ContinuationBegin,
        ContinuationEnd,
        MarkAndSweep,
        ThisContext,
        ThisContinuation,

        If,
        IfElse,
        ForEachContained,

        IfThenSuspend,
        IfThenReplace,
        IfThenResume,

        IfThenSuspendElseSuspend,
        IfThenReplaceElseSuspend,
        IfThenResumeElseSuspend,

        IfThenSuspendElseReplace,
        IfThenReplaceElseReplace,
        IfThenResumeElseReplace,

        IfThenSuspendElseResume,
        IfThenReplaceElseResume,
        IfThenResumeElseResume,

        Executor,
        ExecFile,

        Assert,
        NTimes,
        ForEach,
        AcrossAllNodes,  // New operation for network iteration

        GetProperty,
        SetProperty,

        Index,
        PreInc,
        PreDec,
        PostInc,
        PostDec,

        WhileLoop,
        ForLoop,
        DoLoop,

        LevelStack,

        SetChild,
        GetChild,
        RemoveChild,

        SetManaged,

        PlusEquals,
        MinusEquals,
        MulEquals,
        DivEquals,
        ModEquals,

        Size,

        Print,

        ToPi,
        ToRho,
        StartPiSequence,
        StartRhoSequence,
        GarbageCollect,
        Jump,         // Unconditional jump to a label
        IfFalseJump,  // Jump to a label if the top of the stack is false
        UnnnamedOp,
        ShellCommand,  // Execute shell command and push result
        ToStringOp,    // Convert top of stack to string
    };

   private:
    Type value;

   public:
    Operation(int T = 0) : value(Type(T)) {}

    void SetType(Type T) { value = T; }
    Type GetTypeNumber() const { return value; }

    static const char *ToString(int);
    const char *ToString() const;

    friend bool operator<(const Operation &A, const Operation &B) {
        return A.value < B.value;
    }
    friend bool operator==(const Operation &A, const Operation &B) {
        return A.value == B.value;
    }
    friend bool operator!=(const Operation &A, const Operation &B) {
        return A.value != B.value;
    }

    static void Register(Registry &);
};

KAI_TYPE_TRAITS(Operation, Number::Operation,
                Properties::Streaming | Properties::Assign);

// Add streaming operators for Operation
BinaryStream &operator<<(BinaryStream &, const Operation &);
BinaryStream &operator>>(BinaryStream &, Operation &);
StringStream &operator<<(StringStream &, const Operation &);
StringStream &operator>>(StringStream &, Operation &);

KAI_END
