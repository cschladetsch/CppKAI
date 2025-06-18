#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Executor/BinaryOperationHandler.h>

KAI_BEGIN

// Delegate to BinaryOperationHandler
Object Executor::PerformBinaryOp(Object const &A, Object const &B,
                                 Operation::Type op) {
    if (!binaryOpHandler_) {
        KAI_TRACE_ERROR() << "BinaryOperationHandler not initialized";
        return Object();
    }
    return binaryOpHandler_->Perform(A, B, op);
}

KAI_END