#pragma once

#include <KAI/Core/Object.h>
#include <KAI/Executor/Operation.h>

KAI_BEGIN

class BinaryOperationHandler {
   public:
    BinaryOperationHandler() = default;
    ~BinaryOperationHandler() = default;

    // Main method to perform binary operations
    Object Perform(Object const &A, Object const &B, Operation::Type op);

    // Static method to check if an operation is binary
    static bool IsBinaryOp(Operation::Type op);

   private:
    // Helper to create new objects with proper registry
    template <typename T>
    Object CreateNew(Registry *registry, T value);

    // Operation-specific handlers
    Object PerformArithmetic(Object const &A, Object const &B,
                             Operation::Type op, Registry *registry);
    Object PerformComparison(Object const &A, Object const &B,
                             Operation::Type op, Registry *registry);
    Object PerformLogical(Object const &A, Object const &B, Operation::Type op,
                          Registry *registry);
    Object PerformBitwise(Object const &A, Object const &B, Operation::Type op,
                          Registry *registry);
    Object PerformMinMax(Object const &A, Object const &B, Operation::Type op,
                         Registry *registry);
    Object PerformIndex(Object const &A, Object const &B, Registry *registry);

    // Helper to find a valid registry from objects
    Registry *FindRegistry(Object const &A, Object const &B);
};

KAI_END