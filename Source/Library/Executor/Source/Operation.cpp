#include "KAI/Executor/Operation.h"

#include "KAI/Core/BinaryStream.h"  // Include the full implementation of BinaryStream
#include "KAI/Core/Object/ClassBuilder.h"  // Include ClassBuilder for Register function

KAI_BEGIN

const char *Operation::ToString(int value) {
    switch (value) {
#define CASE(N) \
    case N:     \
        return #N;
        CASE(Return)
        CASE(SuspendNew)
        CASE(None)
        CASE(True)
        CASE(False)
        CASE(Equiv)
        CASE(NotEquiv)
        CASE(Less)
        CASE(Greater)
        CASE(LessOrEquiv)
        CASE(GreaterOrEquiv)
        CASE(Suspend)
        CASE(Replace)
        CASE(Resume)
        CASE(CppFunctionCall)
        CASE(CppMethodCall)
        CASE(Plus)
        CASE(Minus)
        CASE(Multiply)
        CASE(Divide)
        CASE(Modulo)
        CASE(Min)
        CASE(Max)
        CASE(TypeOf)
        CASE(Store)
        CASE(Retreive)
        CASE(Remove)
        CASE(New)
        CASE(Assign)
        CASE(Freeze)
        CASE(Thaw)
        CASE(GetScope)
        CASE(ChangeScope)
        CASE(GetChildren)
        CASE(Contents)
        CASE(Break)
        CASE(Continue)
        CASE(Drop)
        CASE(DropN)
        CASE(Swap)
        CASE(Dup)
        CASE(Dup2)
        CASE(Drop2)
        CASE(Rot)
        CASE(RotN)
        CASE(Roll)
        CASE(Pick)
        CASE(Clear)
        CASE(Depth)
        CASE(Over)
        CASE(ToArray)
        CASE(ToList)
        CASE(ToMap)
        CASE(ToSet)
        CASE(ToHashMap)
        CASE(ToPair)
        CASE(ToVector2)
        CASE(ToVector3)
        CASE(ToVector4)
        CASE(Expand)
        CASE(Name)
        CASE(Fullname)
        CASE(LogicalNot)
        CASE(LogicalAnd)
        CASE(LogicalOr)
        CASE(LogicalXor)
        CASE(LogicalNand)
        CASE(BitwiseNot)
        CASE(BitwiseAnd)
        CASE(BitwiseOr)
        CASE(BitwiseXor)
        CASE(BitwiseNand)
        CASE(LeftShift)
        CASE(RightShift)
        CASE(Lookup)
        CASE(TraceAll)
        CASE(Trace)
        CASE(This)
        CASE(Self)
        CASE(Ref)
        CASE(Detach)
        CASE(Delete)
        CASE(Exists)
        CASE(Pin)
        CASE(Unpin)
        CASE(ContinuationBegin)
        CASE(ContinuationEnd)
        CASE(MarkAndSweep)
        CASE(ThisContext)
        CASE(ThisContinuation)
        CASE(If)
        CASE(IfElse)
        CASE(ForEachContained)
        CASE(IfThenSuspend)
        CASE(IfThenReplace)
        CASE(IfThenResume)
        CASE(IfThenSuspendElseSuspend)
        CASE(IfThenReplaceElseSuspend)
        CASE(IfThenResumeElseSuspend)
        CASE(IfThenSuspendElseReplace)
        CASE(IfThenReplaceElseReplace)
        CASE(IfThenResumeElseReplace)
        CASE(IfThenSuspendElseResume)
        CASE(IfThenReplaceElseResume)
        CASE(IfThenResumeElseResume)
        CASE(Executor)
        CASE(ExecFile)
        CASE(Assert)
        CASE(NTimes)
        CASE(ForEach)
        CASE(AcrossAllNodes)
        CASE(GetProperty)
        CASE(SetProperty)
        CASE(Index)
        CASE(PreInc)
        CASE(PreDec)
        CASE(PostInc)
        CASE(PostDec)
        CASE(WhileLoop)
        CASE(ForLoop)
        CASE(DoLoop)
        CASE(LevelStack)
        CASE(SetChild)
        CASE(GetChild)
        CASE(RemoveChild)
        CASE(SetManaged)
        CASE(PlusEquals)
        CASE(MinusEquals)
        CASE(MulEquals)
        CASE(DivEquals)
        CASE(ModEquals)
        CASE(Size)
        CASE(Print)
        CASE(ToPi)
        CASE(ToRho)
        CASE(StartPiSequence)
        CASE(StartRhoSequence)
        CASE(GarbageCollect)
        CASE(Jump)
        CASE(IfFalseJump)
        CASE(UnnnamedOp)
        CASE(ShellCommand)
        CASE(ToStringOp)
    }

    return "UnnnamedOp";
}

char const *Operation::ToString() const { return Operation::ToString(value); }

void Operation::Register(Registry &registry) {
    ClassBuilder<Operation>(registry, "Operation");
}

KAI_END

// Implement streaming operators for Operation - these must be outside the
// namespace
KAI_BEGIN

BinaryStream &operator<<(BinaryStream &stream, const Operation &op) {
    return stream << (int)op.GetTypeNumber();
}

BinaryStream &operator>>(BinaryStream &stream, Operation &op) {
    // Use the template method from BinaryPacket which is inherited by
    // BinaryStream
    int val;
    stream.BinaryPacket::Read(val);
    op.SetType((Operation::Type)val);
    return stream;
}

StringStream &operator<<(StringStream &stream, const Operation &op) {
    return stream << op.ToString();
}

StringStream &operator>>(StringStream &stream, Operation &op) {
    // Cannot deserialize from string
    // This is a stub implementation just to satisfy the compiler
    return stream;
}

KAI_END
