#include <iostream>
#include <sstream>

#include "KAI/Console/rang.hpp"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Core/Tree.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Executor/Compiler.h"
#include "KAI/Executor/SignedContinuation.h"
#include "KAI/Language/Common/Process.h"

using namespace std;

KAI_BEGIN

// The higher the trace number, the more verbose debug output.
int Process::trace = 0;

void Executor::Create() {
    data_ = New<Stack>();
    context_ = New<Stack>();
    break_ = false;
    traceLevel_ = 0;
    stepNumber_ = 0;
}

bool Executor::Destroy() { return true; }

void Executor::Register(Registry &registry, const char *name) {
    ClassBuilder<Executor>(registry, name);
    
    // At this time, we're not exposing any properties to scripts
    // registry.AddProperty("Trace", &Executor::GetTraceLevel, &Executor::SetTraceLevel);
}

bool operator<(const Executor &A, const Executor &B) {
    return A.GetDataStack() < B.GetDataStack();
}

bool operator==(const Executor &A, const Executor &B) {
    return A.GetDataStack() == B.GetDataStack();
}

StringStream &operator<<(StringStream &S, Executor const &exec) {
    S << "Executor: ";
    Value<const Stack> data = exec.GetDataStack();
    S << "Stack " << (data.Valid() ? "Valid" : "Invalid");
    if (data.Valid()) S << data;
    
    Value<Stack> context = exec.GetContextStack();
    S << ", Context " << (context.Valid() ? "Valid" : "Invalid");
    if (context.Valid()) S << context;
    
    return S;
}

BinaryStream &operator<<(BinaryStream &S, Executor const &exec) {
    S << exec.GetDataStack();
    S << exec.GetContextStack();
    // We can't properly serialize continuation, so leave it out
    return S;
}

BinaryPacket &operator>>(BinaryPacket &S, Executor &exec) {
    // This isn't properly implemented, but we'll leave a stub
    // that doesn't try to access private members
    return S;
}

//
// Below are functions that were split into separate files, but are included here
// to maintain build compatibility. In the future, these should be moved to their 
// own files after fixing build issues.
//

// ======================= Stack Operations ========================

void Executor::Push(Object const &Q) {
    // Push the referenced object if needed.
    if (Q.GetTypeNumber() == Type::Number::Object)
        Push(*data_, ConstDeref<Object>(Q));
    else
        Push(*data_, Q);
}

void Executor::Push(const std::pair<Object, Object> &P) {
    Push(New(Pair(P.first, P.second)));
}

Object Executor::Pop() { return Pop(*data_); }

Object Executor::Top() const { return data_->Top(); }

Value<Stack> Executor::GetDataStack() { return data_; }

Value<Stack> Executor::GetContextStack() const { return context_; }

void Executor::Push(Stack &stack, Object const &Q) { stack.Push(Q); }

Object Executor::Pop(Stack &stack) { return stack.Pop(); }

bool Executor::PopBool() {
    auto val = Pop();
    if (!val.IsType<bool>())
        KAI_THROW_1(Base, "Value is not a bool");
    return ConstDeref<bool>(val);
}

void Executor::ToArray() {
    // For empty arrays, just create and push an empty array
    if (data_->Size() == 0) {
        auto emptyArray = New<Array>();
        Push(emptyArray);
        return;
    }
    
    // Special handling for empty array case "[]"
    // In this case, the continuation will have pushed a 0 onto the stack
    if (data_->Size() == 1 && data_->Top().IsType<int>() && 
        ConstDeref<int>(data_->Top()) == 0) {
        data_->Pop(); // Remove the 0
        auto emptyArray = New<Array>();
        Push(emptyArray);
        return;
    }
    
    // Check if we already have an array on the stack (this can happen with our PiTranslator changes)
    if (data_->Size() == 1 && data_->Top().IsType<Array>()) {
        // An array is already on the stack, leave it there
        return;
    }
    
    // For non-empty arrays, follow the regular pattern
    // First, get the length
    auto len = ConstDeref<int>(Pop());
    if (len < 0) KAI_THROW_1(BadIndex, len);

    auto array = New<Array>();
    array->Resize(len);
    while (len--) array->RefAt(len) = Pop();

    Push(array);
}

void Executor::DropN() {
    auto count = Deref<int>(Pop());
    if (count < 0) KAI_THROW_1(BadIndex, count);

    while (count-- > 0) Pop();
}

// ClearStacks is already defined in the header file

void Executor::ClearContext() {
    context_->Clear();
}

void Executor::Expand() {
    Object Q = Pop();
    switch (Q.GetTypeNumber().value) {
        case Type::Number::Pair: {
            const Pair &P = ConstDeref<Pair>(Q);
            Push(P.first);
            Push(P.second);

            break;
        }

        case Type::Number::List:
            PushAll(ConstDeref<List>(Q));
            break;

        case Type::Number::Array:
            PushAll(ConstDeref<Array>(Q));
            break;

        case Type::Number::Map:
            PushAll(ConstDeref<Map>(Q));
            break;

        default:
            KAI_THROW_1(Base, "Invalid Expand target");
            break;
    }
}

void Executor::GetChildren() {
    const auto &scope = GetStorageBase(Pop());
    auto children = New<Array>();
    for (const auto &child : scope.GetDictionary())
        children->Append(New(child.first.ToString()));

    Push(children);
}

template <class Cont>
void Executor::PushAll(const Cont &cont) {
    for (const auto &A : cont) Push(A);

    Push(New(cont.Size()));
}

//PrintStack is already implemented elsewhere

void Executor::PrintStack(std::ostream &out) const {
    if (data_->Empty()) {
        out << "Stack is empty\n";
        return;
    }
    
    const Stack &stack = *data_;
    out << "Stack (size " << stack.Size() << "):\n";
    
    // Print the stack items in reverse order (top of stack at the bottom)
    for (int i = stack.Size() - 1; i >= 0; --i) {
        out << i << ": ";
        
        Object item = stack.At(i);
        if (!item.Exists()) {
            out << "[null]";
        } else {
            StringStream ss;
            ss << item;
            out << ss.ToString();
        }
        
        out << "\n";
    }
}

void Executor::DumpStack(Stack const &stack) {
    KAI_TRACE() << "Stack: " << stack.Size() << " items";
    for (int i = 0; i < stack.Size(); ++i)
        KAI_TRACE() << i << ": " << stack.At(i);
}

// ======================= Object Resolution ======================

Object Executor::Resolve(Object Q, bool ignoreQuote) const {
    // TODO: this double-handling of Labels and Pathnames is tedious and wrong.
    if (Q.IsType<Label>()) {
        const auto &l = ConstDeref<Label>(Q);
        if (l.Quoted() && !ignoreQuote) return Q;
        return Resolve(l);
    }

    if (Q.IsType<Pathname>()) {
        const auto &l = ConstDeref<Pathname>(Q);
        if (l.Quoted() && !ignoreQuote) return Q;
        return Resolve(l);
    }

    return Q;
}

Object Executor::TryResolve(Object const &Q) const {
    switch (Q.GetTypeNumber().ToInt()) {
        case Type::Number::Label:
            return TryResolve(ConstDeref<Label>(Q));
        case Type::Number::Pathname:
            return TryResolve(ConstDeref<Pathname>(Q));
    }

    return Object();
}

Object Executor::TryResolve(Label const &label) const {
    // Search in current scope.
    if (continuation_.Exists()) {
        Object scope = continuation_->GetScope();
        if (scope.Exists() && scope.Has(label)) return scope.Get(label);
    }

    // search in parent scopes...
    Stack const &scopes = *context_;
    for (int N = 0; N < scopes.Size(); ++N) {
        Pointer<Continuation> cont = scopes.At(N);
        if (!cont.Exists()) break;

        Object scope = cont->GetScope();
        if (scope.Exists() && scope.HasChild(label))
            return scope.GetChild(label);
    }

    // Finally, search the tree.
    return tree_->Resolve(label);
}

Object Executor::TryResolve(Pathname const &path) const {
    // If it's not an absolute path, search up the continuation scopes.
    if (path.Absolute()) return tree_->Resolve(path);

    // Search in current scope.
    if (continuation_.Exists()) {
        auto found = Get(continuation_->GetScope(), path);
        if (found.Exists()) return found;
    }

    // Search in parent scopes.
    Stack const &scopes = *context_;
    for (int N = 0; N < scopes.Size(); ++N) {
        Pointer<Continuation> cont = scopes.At(N);
        if (!cont.Exists()) continue;

        Object scope = cont->GetScope();
        if (Exists(scope, path)) return Get(scope, path);
    }

    return Object();
}

Object Executor::Resolve(Label const &label) const {
    Object Q = TryResolve(label);
    if (!Q.Valid()) KAI_THROW_1(CannotResolve, label);
    return Q;
}

Object Executor::Resolve(const Pathname &path) const {
    Object Q = TryResolve(path);
    if (!Q.Valid()) KAI_THROW_1(CannotResolve, path);
    return Q;
}

// ======================= Helper Methods ========================

void Executor::Eval(Object const &Q) {
    stepNumber_++;
    
    // If this is a continuation, make sure it has the specialHandling flag set
    if (Q.IsType<Continuation>()) {
        Pointer<Continuation> cont = Q;
        if (!cont->GetSpecialHandling()) {
            cont->SetSpecialHandling(true);
            KAI_TRACE() << "Setting specialHandling flag on continuation at evaluation time";
        }
    }

    switch (GetTypeNumber(Q).value) {
        case Type::Number::Operation: {
            const auto op = Deref<Operation>(Q).GetTypeNumber();
            Perform(op);
            break;
        }

        case Type::Number::Pathname:
            EvalIdent<Pathname>(Q);
            break;

        case Type::Number::Label:
            EvalIdent<Label>(Q);
            break;

        default:
            Push(Q.Clone());
            break;
    }
}

void Executor::SetScope(Object scope) { context_->Push(scope); }

void Executor::PopScope() { context_->Pop(); }

Object Executor::GetScope() const { return context_->Top(); }

void Executor::SetContinuation(Value<Continuation> C) { continuation_ = C; }

void Executor::Continue() {
    while (true) {
        break_ = false;
        Object next;
        if (continuation_->Next(next)) {
            KAI_TRY {
                if (traceLevel_ > 10) KAI_TRACE() << "Start step\n";
                if (traceLevel_ > 10) KAI_TRACE_1(stepNumber_);
                if (traceLevel_ > 10) KAI_TRACE_1(data_);
                if (traceLevel_ > 10) KAI_TRACE_1(context_);
                if (traceLevel_ > 10) KAI_TRACE_1(next);

                Eval(next);
            }
            catch (Exception::Base &E) {
                KAI_TRACE_1(E);
            }
        } else
            break_ = true;

        if (break_) {
            NextContinuation();
            if (!continuation_.Exists()) return;
        }
    }
}

void Executor::ContinueOnly(Value<Continuation> C) {
    // Add an empty context to break. this forces exection to stop after C is
    // finished.
    context_->Push(Object());
    Continue(C);
}

void Executor::Continue(Value<Continuation> C) {
    if (!C.Exists()) return;

    // Always set the specialHandling flag on continuations
    // This ensures proper type handling across the system
    if (!C->GetSpecialHandling()) {
        C->SetSpecialHandling(true);
        KAI_TRACE() << "Setting specialHandling flag on continuation";
    }

    // Save the current continuation and stack for restoring later
    Value<Continuation> savedContinuation = continuation_;
    
    // Check if this continuation has any binary operations that need special handling
    Pointer<const Array> code = C->GetCode();
    Object lastOp;
    Object firstArg, secondArg;
    bool hasOperands = false;
    Operation::Type opType = Operation::None;
    
    // Try to identify if this is a binary operation continuation
    if (code.Exists() && code->Size() >= 3) {
        // Find the operation
        for (int i = 0; i < code->Size(); i++) {
            if (code->At(i).IsType<Operation>()) {
                lastOp = code->At(i);
                opType = ConstDeref<Operation>(lastOp).GetTypeNumber();
            }
            else if (code->At(i).IsType<int>() || 
                     code->At(i).IsType<float>() || 
                     code->At(i).IsType<double>() || 
                     code->At(i).IsType<bool>() || 
                     code->At(i).IsType<String>()) {
                if (!firstArg.Exists()) {
                    firstArg = code->At(i);
                } else if (!secondArg.Exists()) {
                    secondArg = code->At(i);
                    hasOperands = true;
                }
            }
        }
    }
    
    // If this is a binary operation with known types, we can handle it specially
    if (hasOperands && IsBinaryOp(opType)) {
        // Directly compute the result with the appropriate type
        Object result = PerformBinaryOp(firstArg, secondArg, opType);
        
        // Push the properly typed result
        if (result.Exists()) {
            data_->Push(result);
            // Restore the previous continuation and return
            continuation_ = savedContinuation;
            return;
        }
    }
    
    // If it wasn't a binary operation or we couldn't handle it specially,
    // execute the continuation normally
    SetContinuation(C);
    Continue();
    
    // After execution, check if there's a result on the stack that needs unwrapping
    if (!data_->Empty()) {
        // Get the top value
        Object result = data_->Top();
        
        // If the result is a continuation, try to unwrap it
        if (result.IsType<Continuation>()) {
            KAI_TRACE() << "Unwrapping continuation result using type trait system";
            Object unwrapped = UnwrapValue(result);
            
            // Replace the top stack value with the unwrapped result
            data_->Pop();
            data_->Push(unwrapped);
        }
    }
    
    // Restore the previous continuation
    continuation_ = savedContinuation;
}

void Executor::NextContinuation() {
    if (context_->Empty()) {
        continuation_ = Object();
        return;
    }

    const auto next = context_->Pop();
    SetContinuation(next);
}

Value<Continuation> Executor::NewContinuation(Value<Continuation> orig) {
    Value<Continuation> cont = New<Continuation>();
    cont->SetCode(orig->GetCode());
    cont->args = orig->args;

    return cont;
}

void Executor::ConditionalContextSwitch(Operation::Type op) {
    if (!ConstDeref<bool>(Pop())) {
        Pop();
        return;
    }

    switch (op) {
        case Operation::Suspend:
            continuation_->Next();
            context_->Push(continuation_);
            // fallthrough
        case Operation::Replace:
            context_->Push(NewContinuation(Pop()));
            // fallthrough
        case Operation::Resume:
            break_ = true;
        default:
            KAI_NOT_IMPLEMENTED();
            break;
    }
}

void Executor::TraceAll() {
    KAI_TRACE_1(data_);
    KAI_TRACE_1(context_);
    KAI_TRACE_1(continuation_);
}

void Executor::Trace(const Object &Q) {
    StringStream str;
    Trace(Q, str);
    KAI_TRACE() << str.ToString();
}

void Executor::Trace(const Object &object, StringStream &str) {
    if (!object.Exists()) {
        str << "<null>";
        return;
    }

    // Get the storage base for the object
    const auto &storage = GetStorageBase(object);
    
    // Get the object's children (dictionary entries)
    const auto &children = storage.GetDictionary();
    for (const auto &child : children) {
        str << child.first << ": ";
        Trace(child.second, str);
        str << "\n";
    }
}

void Executor::Trace(const Label &L, const StorageBase &Q, StringStream &str) {
    str << L << ": ";
    
    // Just trace the label itself
    Object val = Object();
    
    str << val;
    if (val.Exists() && val.GetTypeNumber().ToInt() != Type::Number::None &&
        val.GetTypeNumber().ToInt() != Type::Number::Object)
        str << " (" << val.GetClass()->GetName() << ")";
    str << "\n";
}

void Executor::MarkAndSweep() {
    KAI_NOT_IMPLEMENTED();
    // MarkAndSweep(tree_->GetRoot());
}

void Executor::MarkAndSweep(Object &root) {
    root.GetRegistry()->GarbageCollect();
}

template <class Container>
Value<Array> Executor::ForEach(Container const &cont, Object const &fun) {
    auto array = New<Array>();
    for (auto const &elem : cont) {
        Push(elem);
        context_->Push(Object());
        Continue(fun);
        array->Append(Pop());
    }

    return array;
}

void Executor::DumpContinuation(Continuation const &cont, int level) {
    KAI_UNUSED_1(level);
    KAI_TRACE() << "----- CONTINUATION -------";
    KAI_TRACE_1(cont.GetScope());
    
    // Get the code
    Pointer<const Array> code = cont.GetCode();
    if (!code.Exists()) {
        KAI_TRACE() << "No code.";
        return;
    }

    if (code->Empty()) {
        KAI_TRACE() << "Empty code.";
        return;
    }

    // Don't access the position, just show the code
    KAI_TRACE() << "Code size: " << code->Size();
    for (int N = 0; N < code->Size(); ++N) {
        StringStream str;
        str << N << ": " << code->At(N);
        KAI_TRACE() << str.ToString();
    }
}

Object Executor::UnwrapValue(Object const &Q) {
    // If not a continuation, just return the object as is
    if (!Q.IsType<Continuation>()) {
        return Q;
    }
    
    // Get the continuation
    Pointer<Continuation> cont = Q;
    
    // If the continuation has no code, can't extract a value
    if (!cont->GetCode().Exists() || cont->GetCode()->Size() == 0) {
        return Q;
    }
    
    // Always set the specialHandling flag on continuations
    if (!cont->GetSpecialHandling()) {
        cont->SetSpecialHandling(true);
        KAI_TRACE() << "Setting specialHandling flag on continuation during unwrap";
    }
    
    // Check if this is a marked continuation that should be evaluated
    if (cont->GetSpecialHandling()) {
        // First, look for primitive values in the code
        Pointer<const Array> code = cont->GetCode();
        if (code->Size() > 0) {
            Object firstElement = code->At(0);
            
            // Check if it's a simple value type we can extract directly
            if (firstElement.Exists() && 
                (firstElement.IsType<int>() || 
                 firstElement.IsType<float>() || 
                 firstElement.IsType<double>() || 
                 firstElement.IsType<bool>() || 
                 firstElement.IsType<String>() ||
                 firstElement.IsType<Array>() ||
                 firstElement.IsType<List>() ||
                 firstElement.IsType<Map>() ||
                 firstElement.IsType<Pair>())) {
                
                return firstElement;
            }
        }
        
        // Check if this is a binary operation that we can directly perform
        // This optimizes common Rho/Pi language patterns
        bool hasBinaryOperation = false;
        Operation::Type opType = Operation::None;
        std::vector<Object> operands;
        
        // First find operations
        for (int i = 0; i < code->Size(); i++) {
            if (code->At(i).IsType<Operation>()) {
                Object op = code->At(i);
                opType = ConstDeref<Operation>(op).GetTypeNumber();
                
                if (IsBinaryOp(opType)) {
                    hasBinaryOperation = true;
                }
            }
        }
        
        // Then collect operands if we found a binary operation
        if (hasBinaryOperation) {
            for (int i = 0; i < code->Size(); i++) {
                Object item = code->At(i);
                if (!item.IsType<Operation>() && 
                    (item.IsType<int>() || 
                     item.IsType<float>() || 
                     item.IsType<double>() || 
                     item.IsType<bool>() || 
                     item.IsType<String>() ||
                     item.IsType<Array>() ||
                     item.IsType<List>() ||
                     item.IsType<Map>() ||
                     item.IsType<Pair>())) {
                    operands.push_back(item);
                }
            }
            
            // If we have exactly two operands and a binary operation, perform it directly
            if (operands.size() == 2 && IsBinaryOp(opType)) {
                Object A = operands[0];
                Object B = operands[1];
                
                KAI_TRACE() << "Directly computing binary operation: " 
                          << A.ToString() << " " << Operation::ToString(opType) 
                          << " " << B.ToString();
                          
                Object result = PerformBinaryOp(A, B, opType);
                if (result.Exists()) {
                    return result;
                }
            }
        }
        
        // For continuations with operations, we need to evaluate them
        // Save the original data stack for later restoration
        auto tempStack = New<Stack>();
        auto oldStack = data_;
        data_ = tempStack;
        
        try {
            // Execute the continuation directly
            Continue(cont);
            
            // Get the result from the top of the stack if available
            if (!tempStack->Empty()) {
                Object result = tempStack->Top();
                
                // Special handling for primitive types
                if (result.Exists()) {
                    // Check for Rho/Pi language operation results that may need type correction
                    if (hasBinaryOperation && operands.size() >= 2) {
                        Object A = operands[0];
                        Object B = operands[1];
                        
                        // Integer operations
                        if ((A.IsType<int>() && B.IsType<int>()) && 
                            (opType == Operation::Plus || opType == Operation::Minus || 
                             opType == Operation::Multiply || opType == Operation::Divide || 
                             opType == Operation::Modulo)) {
                            
                            // Try to convert the result to an int if it wasn't already
                            if (!result.IsType<int>()) {
                                try {
                                    int intValue = std::stoi(result.ToString().c_str());
                                    result = New<int>(intValue);
                                } catch (...) {
                                    // If conversion fails, keep the original result
                                }
                            }
                        }
                        // Float operations
                        else if ((A.IsType<float>() || B.IsType<float>()) && 
                                 (opType == Operation::Plus || opType == Operation::Minus || 
                                  opType == Operation::Multiply || opType == Operation::Divide)) {
                            
                            // Try to convert the result to a float if it wasn't already
                            if (!result.IsType<float>()) {
                                try {
                                    float floatValue = std::stof(result.ToString().c_str());
                                    result = New<float>(floatValue);
                                } catch (...) {
                                    // If conversion fails, keep the original result
                                }
                            }
                        }
                        // Boolean operations
                        else if ((opType == Operation::Equiv || opType == Operation::NotEquiv || 
                                  opType == Operation::Less || opType == Operation::Greater ||
                                  opType == Operation::LessOrEquiv || opType == Operation::GreaterOrEquiv ||
                                  opType == Operation::LogicalAnd || opType == Operation::LogicalOr || 
                                  opType == Operation::LogicalXor)) {
                            
                            // Try to convert the result to a bool if it wasn't already
                            if (!result.IsType<bool>()) {
                                try {
                                    String strValue = result.ToString();
                                    bool boolValue = (strValue == "true" || strValue == "1");
                                    result = New<bool>(boolValue);
                                } catch (...) {
                                    // If conversion fails, keep the original result
                                }
                            }
                        }
                        // String operations
                        else if ((A.IsType<String>() || B.IsType<String>()) && 
                                 opType == Operation::Plus) {
                            
                            // Try to convert the result to a String if it wasn't already
                            if (!result.IsType<String>()) {
                                try {
                                    String strValue = result.ToString();
                                    result = New<String>(strValue);
                                } catch (...) {
                                    // If conversion fails, keep the original result
                                }
                            }
                        }
                    }
                    
                    // Restore the stack and return the result
                    data_ = oldStack;
                    return result;
                }
            }
            
            // Restore original stack before returning
            data_ = oldStack;
        }
        catch (const std::exception &e) {
            // If evaluation fails, restore the stack and continue with normal processing
            data_ = oldStack;
            KAI_TRACE_ERROR() << "Error unwrapping continuation: " << e.what();
        }
        catch (...) {
            // Catch any other exceptions and restore stack
            data_ = oldStack;
            KAI_TRACE_ERROR() << "Unknown error unwrapping continuation";
        }
    }
    
    // If we can't unwrap, return the original
    return Q;
}

// Enhanced version of PerformBinaryOp that handles all operation types using KAI type traits
Object Executor::PerformBinaryOp(Object const &A, Object const &B, Operation::Type op) {
    // Use the KAI type traits system to perform operations based on the object types
    
    // Helper to check if a type has a specific property using the type traits system
    auto hasProperty = [](const Object& obj, const int property) -> bool {
        if (!obj.Exists() || !obj.GetClass()) return false;
        return obj.GetClass()->HasProperty(property);
    };
    
    // First, handle the operation based on type using KAI type traits
    switch (op) {
        // Arithmetic operations
        case Operation::Plus:
            // Int + Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Plus::Perform(ConstDeref<int>(A), ConstDeref<int>(B));
                return New<int>(result);
            }
            // Float + Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Plus::Perform(ConstDeref<float>(A), ConstDeref<float>(B));
                return New<float>(result);
            }
            // Float + Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) + static_cast<float>(ConstDeref<int>(B));
                return New<float>(result);
            }
            // Int + Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) + ConstDeref<float>(B);
                return New<float>(result);
            }
            // String + String = String (concatenation)
            else if (A.IsType<String>() && B.IsType<String>()) {
                String result = Type::Traits<String>::Plus::Perform(ConstDeref<String>(A), ConstDeref<String>(B));
                return New<String>(result);
            }
            // Use type traits for other types that support Plus
            else if (hasProperty(A, Properties::Plus) && A.GetTypeNumber() == B.GetTypeNumber()) {
                // Handle generic case using dynamic dispatch through the registry
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Plus);
                return result;
            }
            break;
            
        case Operation::Minus:
            // Int - Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Minus::Perform(ConstDeref<int>(A), ConstDeref<int>(B));
                return New<int>(result);
            }
            // Float - Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Minus::Perform(ConstDeref<float>(A), ConstDeref<float>(B));
                return New<float>(result);
            }
            // Float - Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) - static_cast<float>(ConstDeref<int>(B));
                return New<float>(result);
            }
            // Int - Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) - ConstDeref<float>(B);
                return New<float>(result);
            }
            // Use type traits for other types that support Minus
            else if (hasProperty(A, Properties::Minus) && A.GetTypeNumber() == B.GetTypeNumber()) {
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Minus);
                return result;
            }
            break;
            
        case Operation::Multiply:
            // Int * Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Multiply::Perform(ConstDeref<int>(A), ConstDeref<int>(B));
                return New<int>(result);
            }
            // Float * Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Multiply::Perform(ConstDeref<float>(A), ConstDeref<float>(B));
                return New<float>(result);
            }
            // Float * Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) * static_cast<float>(ConstDeref<int>(B));
                return New<float>(result);
            }
            // Int * Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) * ConstDeref<float>(B);
                return New<float>(result);
            }
            // Use type traits for other types that support Multiply
            else if (hasProperty(A, Properties::Multiply) && A.GetTypeNumber() == B.GetTypeNumber()) {
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Multiply);
                return result;
            }
            break;
            
        case Operation::Divide:
            // Int / Int = Int (integer division)
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                int result = Type::Traits<int>::Divide::Perform(ConstDeref<int>(A), divisor);
                return New<int>(result);
            }
            // Float / Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float divisor = ConstDeref<float>(B);
                if (divisor == 0.0f) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                float result = Type::Traits<float>::Divide::Perform(ConstDeref<float>(A), divisor);
                return New<float>(result);
            }
            // Float / Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                float result = ConstDeref<float>(A) / static_cast<float>(divisor);
                return New<float>(result);
            }
            // Int / Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float divisor = ConstDeref<float>(B);
                if (divisor == 0.0f) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                float result = static_cast<float>(ConstDeref<int>(A)) / divisor;
                return New<float>(result);
            }
            // Use type traits for other types that support Divide
            else if (hasProperty(A, Properties::Divide) && A.GetTypeNumber() == B.GetTypeNumber()) {
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Divide);
                return result;
            }
            break;
            
        case Operation::Modulo:
            // Int % Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Modulo by zero");
                }
                int result = ConstDeref<int>(A) % divisor;
                return New<int>(result);
            }
            // Note: modulo with floats would require fmod() from <cmath>, but we're skipping for now
            break;
            
        // Comparison operations
        case Operation::Equiv:
            // Int == Int -> bool
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Equiv::Perform(ConstDeref<int>(A), ConstDeref<int>(B));
                return New<bool>(result);
            }
            // Float == Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Equiv::Perform(ConstDeref<float>(A), ConstDeref<float>(B));
                return New<bool>(result);
            }
            // Float == Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = ConstDeref<float>(A) == static_cast<float>(ConstDeref<int>(B));
                return New<bool>(result);
            }
            // Int == Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = static_cast<float>(ConstDeref<int>(A)) == ConstDeref<float>(B);
                return New<bool>(result);
            }
            // Bool == Bool -> bool
            else if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = Type::Traits<bool>::Equiv::Perform(ConstDeref<bool>(A), ConstDeref<bool>(B));
                return New<bool>(result);
            }
            // String == String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Equiv::Perform(ConstDeref<String>(A), ConstDeref<String>(B));
                return New<bool>(result);
            }
            // General object equality
            else {
                bool result = A == B;
                return New<bool>(result);
            }
            break;
            
        case Operation::NotEquiv:
            // Invert Equiv result
            {
                Object equivResult = PerformBinaryOp(A, B, Operation::Equiv);
                if (equivResult.IsType<bool>()) {
                    bool result = !ConstDeref<bool>(equivResult);
                    return New<bool>(result);
                }
            }
            break;
            
        case Operation::Less:
            // Int < Int -> bool
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Less::Perform(ConstDeref<int>(A), ConstDeref<int>(B));
                return New<bool>(result);
            }
            // Float < Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Less::Perform(ConstDeref<float>(A), ConstDeref<float>(B));
                return New<bool>(result);
            }
            // Float < Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = ConstDeref<float>(A) < static_cast<float>(ConstDeref<int>(B));
                return New<bool>(result);
            }
            // Int < Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = static_cast<float>(ConstDeref<int>(A)) < ConstDeref<float>(B);
                return New<bool>(result);
            }
            // String < String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Less::Perform(ConstDeref<String>(A), ConstDeref<String>(B));
                return New<bool>(result);
            }
            // Use type traits for other types that support Less
            else if (hasProperty(A, Properties::Less) && A.GetTypeNumber() == B.GetTypeNumber()) {
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Less);
                return result;
            }
            break;
            
        case Operation::Greater:
            // Int > Int -> bool (invert Less)
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Less::Perform(ConstDeref<int>(B), ConstDeref<int>(A));
                return New<bool>(result);
            }
            // Float > Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Less::Perform(ConstDeref<float>(B), ConstDeref<float>(A));
                return New<bool>(result);
            }
            // Float > Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = static_cast<float>(ConstDeref<int>(B)) < ConstDeref<float>(A);
                return New<bool>(result);
            }
            // Int > Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = ConstDeref<float>(B) < static_cast<float>(ConstDeref<int>(A));
                return New<bool>(result);
            }
            // String > String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Less::Perform(ConstDeref<String>(B), ConstDeref<String>(A));
                return New<bool>(result);
            }
            // Use type traits for other types that support Greater
            else if (hasProperty(A, Properties::Greater) && A.GetTypeNumber() == B.GetTypeNumber()) {
                Object result = A.GetRegistry()->PerformOperation(A, B, Operation::Greater);
                return result;
            }
            break;
            
        case Operation::LessOrEquiv:
            // Check if A is less than B or equivalent to B
            {
                Object lessResult = PerformBinaryOp(A, B, Operation::Less);
                Object equivResult = PerformBinaryOp(A, B, Operation::Equiv);
                
                if (lessResult.IsType<bool>() && equivResult.IsType<bool>()) {
                    bool result = ConstDeref<bool>(lessResult) || ConstDeref<bool>(equivResult);
                    return New<bool>(result);
                }
            }
            break;
            
        case Operation::GreaterOrEquiv:
            // Check if A is greater than B or equivalent to B
            {
                Object greaterResult = PerformBinaryOp(A, B, Operation::Greater);
                Object equivResult = PerformBinaryOp(A, B, Operation::Equiv);
                
                if (greaterResult.IsType<bool>() && equivResult.IsType<bool>()) {
                    bool result = ConstDeref<bool>(greaterResult) || ConstDeref<bool>(equivResult);
                    return New<bool>(result);
                }
            }
            break;
            
        // Logical operations
        case Operation::LogicalAnd:
            // Bool && Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) && ConstDeref<bool>(B);
                return New<bool>(result);
            }
            break;
            
        case Operation::LogicalOr:
            // Bool || Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) || ConstDeref<bool>(B);
                return New<bool>(result);
            }
            break;
            
        case Operation::LogicalXor:
            // Bool XOR Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) != ConstDeref<bool>(B);
                return New<bool>(result);
            }
            break;
            
        // Bitwise operations
        case Operation::BitwiseAnd:
            // Int & Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) & ConstDeref<int>(B);
                return New<int>(result);
            }
            break;
            
        case Operation::BitwiseOr:
            // Int | Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) | ConstDeref<int>(B);
                return New<int>(result);
            }
            break;
            
        case Operation::BitwiseXor:
            // Int ^ Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) ^ ConstDeref<int>(B);
                return New<int>(result);
            }
            break;
            
        // Assignment-related operations
        case Operation::Store:
            // Special handling for the store operation
            return B;  // Return the value (second argument) for Store
            
        case Operation::Index:
            // Array[Int] -> Object
            if (A.IsType<Array>() && B.IsType<int>()) {
                const Array& array = ConstDeref<Array>(A);
                int index = ConstDeref<int>(B);
                
                if (index >= 0 && index < static_cast<int>(array.Size())) {
                    // For arrays, we can access elements directly
                    return array.At(index);
                }
                
                KAI_THROW_1(BadIndex, index);
            }
            // List[Int] -> Object
            else if (A.IsType<List>() && B.IsType<int>()) {
                const List& list = ConstDeref<List>(A);
                int index = ConstDeref<int>(B);
                
                if (index >= 0 && index < static_cast<int>(list.Size())) {
                    // For lists, we need to iterate to the correct position
                    auto it = list.begin();
                    for (int i = 0; i < index && it != list.end(); ++i, ++it) {
                        // Just advance the iterator
                    }
                    
                    if (it != list.end()) {
                        return *it;
                    }
                }
                
                KAI_THROW_1(BadIndex, index);
            }
            // Map[Key] -> Value
            else if (A.IsType<Map>()) {
                const Map& map = ConstDeref<Map>(A);
                auto it = map.Find(B);
                
                // Check if the key exists
                if (it != map.end()) {
                    // Return the value from the iterator
                    return it->second;
                }
                
                KAI_THROW_1(Base, "Key not found in map");
            }
            break;
            
        default:
            // For unsupported operations, provide a helpful error message
            KAI_TRACE_ERROR() << "Unsupported operation in PerformBinaryOp: " << Operation::ToString(op);
            KAI_THROW_1(Base, "Unsupported operation in binary operation");
    }
    
    // If we reach here, it means we couldn't handle the operation with the given types
    KAI_TRACE_ERROR() << "Unsupported types for operation: " << A.GetClass()->GetName() 
                      << " and " << B.GetClass()->GetName() 
                      << " for operation " << Operation::ToString(op);
    
    // Try one more approach: see if the registry can handle this operation
    try {
        Object result = A.GetRegistry()->PerformOperation(A, B, op);
        if (result.Exists()) {
            return result;
        }
    }
    catch (const Exception::Base&) {
        // If the registry fails, continue to the error message
    }
    
    KAI_THROW_1(Base, "Unsupported types for operation");
    
    // This will never be reached due to the exception above
    return Object();
}

void Executor::SetTraceLevel(int n) {
    traceLevel_ = n;
}

int Executor::GetTraceLevel() const {
    return traceLevel_;
}

bool Executor::IsBinaryOp(Operation::Type op) {
    switch (op) {
        case Operation::Plus:
        case Operation::Minus:
        case Operation::Multiply:
        case Operation::Divide:
        case Operation::Modulo:
        case Operation::Equiv:
        case Operation::NotEquiv:
        case Operation::Less:
        case Operation::Greater:
        case Operation::LessOrEquiv:
        case Operation::GreaterOrEquiv:
        case Operation::LogicalAnd:
        case Operation::LogicalOr:
        case Operation::LogicalXor:
        case Operation::BitwiseAnd:
        case Operation::BitwiseOr:
        case Operation::BitwiseXor:
            return true;
            
        default:
            return false;
    }
}

// ======================= Perform Implementation ================

#include "KAI/Executor/ExecutorPerform.inl"

KAI_END