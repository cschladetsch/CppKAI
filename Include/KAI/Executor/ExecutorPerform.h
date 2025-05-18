#pragma once

#include <KAI/Core/Object/Object.h>
#include <KAI/Executor/Operation.h>

KAI_BEGIN

struct Executor;

// Functions are declared in ExecutorPerform.h and defined in
// ExecutorPerform.cpp

// Helper method to convert a stack of objects to an array
void Executor::ProcessToArray(int len);

// Helper method to determine if an operation is a binary operation
bool Executor::IsBinaryOp(Operation::Type op);

// Implementation of PerformBinaryOp to directly execute binary operations
Object Executor::PerformBinaryOp(Object const &A, Object const &B,
                                 Operation::Type op);

// Helper method to unwrap continuations and extract the underlying value
Object Executor::UnwrapValue(Object const &Q);

KAI_END