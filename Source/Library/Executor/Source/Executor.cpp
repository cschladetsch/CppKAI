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

    // Save the current continuation for restoring later
    Value<Continuation> savedContinuation = continuation_;
    
    // Execute the continuation normally using the executor
    SetContinuation(C);
    Continue();
    
    // After execution, check if there's a result on the stack that needs unwrapping
    if (!data_->Empty()) {
        // Get the top value
        Object result = data_->Top();
        
        // If the result is a continuation, try to unwrap it using the KAI type trait system
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
    
    // Important: Set the specialHandling flag on the continuation if it's not already set
    // This ensures all continuations get the special handling needed for Pi expressions
    if (!cont->GetSpecialHandling()) {
        cont->SetSpecialHandling(true);
        KAI_TRACE() << "Setting specialHandling flag on continuation during unwrap";
    }
    
    // Look at the first code element - this usually contains the actual value
    // in Pi expression continuations
    Pointer<const Array> code = cont->GetCode();
    if (code->Size() > 0) {
        Object firstElement = code->At(0);
        
        // Check if it's a simple value type we can extract
        // Avoid extracting operations, pathnames, and other complex types
        if (firstElement.Exists() && 
            (firstElement.IsType<int>() || 
             firstElement.IsType<float>() || 
             firstElement.IsType<bool>() || 
             firstElement.IsType<String>() ||
             firstElement.IsType<Array>() ||
             firstElement.IsType<List>() ||
             firstElement.IsType<Map>() ||
             firstElement.IsType<Pair>())) {
            
            return firstElement;
        }
    }
    
    // If we can't unwrap, return the original
    return Q;
}

// This is a simplified version of the PerformBinaryOp
// It only handles basic operations for testing purposes
Object Executor::PerformBinaryOp(Object const &A, Object const &B, Operation::Type op) {
    switch (op) {
        case Operation::Plus:
            if (A.IsType<int>() && B.IsType<int>()) {
                return New(ConstDeref<int>(A) + ConstDeref<int>(B));
            }
            break;
            
        case Operation::Minus:
            if (A.IsType<int>() && B.IsType<int>()) {
                return New(ConstDeref<int>(A) - ConstDeref<int>(B));
            }
            break;
            
        case Operation::Multiply:
            if (A.IsType<int>() && B.IsType<int>()) {
                return New(ConstDeref<int>(A) * ConstDeref<int>(B));
            }
            break;
            
        case Operation::Divide:
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                return New(ConstDeref<int>(A) / divisor);
            }
            break;
            
        case Operation::Modulo:
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Modulo by zero");
                }
                return New(ConstDeref<int>(A) % divisor);
            }
            break;
            
        default:
            KAI_THROW_1(Base, "Unsupported operation");
    }
    
    // If we get here, we didn't handle the operation
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