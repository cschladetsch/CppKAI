#include <iostream>

#include "KAI/Console/rang.hpp"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Core/Tree.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Executor/Compiler.h"
#include "KAI/Executor/SignedContinuation.h"
#include "KAI/Language/Common/Process.h"

using namespace std;

KAI_BEGIN

// The higher the trace number, the more verbose debug output.
int Process::trace = 0;

void Executor::Create() {
    _data = New<Stack>();
    _context = New<Stack>();
    _break = false;
    _traceLevel = 0;
    _stepNumber = 0;
}

bool Executor::Destroy() { return true; }

void Executor::Push(Object const &Q) {
    // Push the referenced object if needed.
    if (Q.GetTypeNumber() == Type::Number::Object)
        Push(*_data, ConstDeref<Object>(Q));
    else
        Push(*_data, Q);
}

void Executor::Push(const std::pair<Object, Object> &P) {
    Push(New(Pair(P.first, P.second)));
}

Object Executor::Pop() { return Pop(*_data); }

Value<Stack> Executor::GetDataStack() { return _data; }

Value<Stack> Executor::GetContextStack() const { return _context; }

void Executor::SetContinuation(Value<Continuation> C) { _continuation = C; }

struct Trace {
    static std::ostream &Debug() { 
        // Always apply bold style before returning cout
        std::cout << rang::style::bold;
        return std::cout; 
    }
};

void Executor::Continue() {
    while (true) {
        _break = false;
        Object next;
        if (_continuation->Next(next)) {
            KAI_TRY {
                if (_traceLevel > 10) KAI_TRACE() << "Start step\n";
                if (_traceLevel > 10) KAI_TRACE_1(_stepNumber);
                if (_traceLevel > 10) KAI_TRACE_1(_data);
                if (_traceLevel > 10) KAI_TRACE_1(_context);
                if (_traceLevel > 10) KAI_TRACE_1(next);

                Eval(next);
            }
            catch (Exception::Base &E) {
                KAI_TRACE_1(E);
            }
        } else
            _break = true;

        if (_break) {
            NextContinuation();
            if (!_continuation.Exists()) return;
        }
    }
}

void Executor::SetScope(Object scope) { _context->Push(scope); }

void Executor::PopScope() { _context->Pop(); }

Object Executor::GetScope() const { return _context->Top(); }

void Executor::Continue(Value<Continuation> C) {
    if (!C.Exists()) return;

    SetContinuation(C);
    Continue();
}

void Executor::NextContinuation() {
    if (_context->Empty()) {
        _continuation = Object();
        return;
    }

    const auto next = _context->Pop();
    SetContinuation(next);
}

void Executor::Push(Stack &stack, Object const &Q) { stack.Push(Q); }

void Executor::Eval(Object const &Q) {
    _stepNumber++;

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

template <class Cont>
void Executor::PushAll(const Cont &cont) {
    for (const auto &A : cont) Push(A);

    Push(New(cont.Size()));
}

Value<Continuation> Executor::NewContinuation(Value<Continuation> orig) {
    Value<Continuation> cont = New<Continuation>();
    cont->SetCode(orig->GetCode());
    cont->args = orig->args;

    return cont;
}

void Executor::MarkAndSweep() {
    KAI_NOT_IMPLEMENTED();
    // MarkAndSweep(_tree->GetRoot());
}

void Executor::MarkAndSweep(Object &root) {
    root.GetRegistry()->GarbageCollect();
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

void Executor::ToArray() {
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

void Executor::ConditionalContextSwitch(Operation::Type op) {
    if (!ConstDeref<bool>(Pop())) {
        Pop();
        return;
    }

    switch (op) {
        case Operation::Suspend:
            _continuation->Next();
            _context->Push(_continuation);
            // fallthrough
        case Operation::Replace:
            _context->Push(NewContinuation(Pop()));
            // fallthrough
        case Operation::Resume:
            _break = true;
        default:
            KAI_NOT_IMPLEMENTED();
            break;
    }
}

void Executor::ContinueOnly(Value<Continuation> C) {
    // Add an empty context to break. this forces exection to stop after C is
    // finished.
    _context->Push(Object());
    Continue(C);
}

template <class Container>
Value<Array> Executor::ForEach(Container const &cont, Object const &fun) {
    auto array = New<Array>();
    for (auto const &elem : cont) {
        Push(elem);
        _context->Push(Object());
        Continue(fun);
        array->Append(Pop());
    }

    return array;
}

Object Executor::Pop(Stack &stack) { return stack.Pop(); }

Object Executor::Top() const { return _data->Top(); }

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
    // Search in _current _scope.
    if (_continuation.Exists()) {
        Object scope = _continuation->GetScope();
        if (scope.Exists() && scope.Has(label)) return scope.Get(label);
    }

    // search in parent scopes...
    Stack const &scopes = *_context;
    for (int N = 0; N < scopes.Size(); ++N) {
        Pointer<Continuation> cont = scopes.At(N);
        if (!cont.Exists()) break;

        Object scope = cont->GetScope();
        if (scope.Exists() && scope.HasChild(label))
            return scope.GetChild(label);
    }

    // Finally, search the _tree.
    return _tree->Resolve(label);
}

Object Executor::TryResolve(Pathname const &path) const {
    // If it's not an absolute _path, search up the continuation scopes.
    if (path.Absolute()) return _tree->Resolve(path);

    // Search in _current _scope.
    if (_continuation.Exists()) {
        auto found = Get(_continuation->GetScope(), path);
        if (found.Exists()) return found;
    }

    // Search in parent scopes.
    Stack const &scopes = *_context;
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

Object Executor::Resolve(Pathname const &path) const {
    Object Q = TryResolve(path);
    if (!Q.Valid()) KAI_THROW_1(CannotResolve, path);

    return Q;
}

void Executor::Trace(const Label &, const StorageBase &object,
                     StringStream &stream) {
    stream << "Handle=" << object.GetHandle().GetValue() << ": "
           << "Parent=" << object.GetParentHandle().GetValue() << ": "
           << "Fullname=" << GetFullname(object) << ": "
           << "Type=" << object.GetClass()->GetName() << ": "
           << "StrStrm='" << object << "'\n";
}

void Executor::Trace(const Object &Q, StringStream &S) {
    if (!Q.Valid()) {
        S << "INVALID_OBJECT";
        return;
    }

    Trace(GetName(Q), Q.GetStorageBase(), S);
}

void Executor::Trace(const Object &Q) {
    StringStream S;
    Trace(Q, S);
    Push(New(S.ToString()));
}

void Executor::ClearContext() {
    _continuation = Object();
    _context->Clear();
}

void Executor::TraceAll() {
    StringStream debug;
    debug << "DEBUG: ";
    for (const auto &elem : Self->GetRegistry()->GetInstances()) {
        try {
            if (elem.second == nullptr)
                debug << "INTERNAL ERROR: Null object in registry";
            else
                Trace(*elem.second, debug);
        } catch (Exception::Base &E) {
            debug << "TraceError :( " << E.ToString();
        } catch (std::exception &E) {
            debug << "TraceError std::exception: " << E.what();
        } catch (...) {
            debug << "TraceError unknown.";
        }
    }

    Push(New(debug.ToString()));
}

void Executor::DumpStack(Stack const &stack) {
    StringStream result;
    auto A = stack.Begin(), B = stack.End();
    for (int N = stack.Size() - 1; A != B; ++A, --N) {
        result << "[" << N << "] ";
        bool is_string = A->GetTypeNumber() == Type::Number::String;
        if (is_string) result << "\"";

        result << *A;
        if (is_string) result << "\"";

        if (A->GetTypeNumber() == Type::Number::Label)
            result << " = " << TryResolve(ConstDeref<Label>(*A));

        result << "\n";
    }

    // Make sure to apply bold style before output
    cout << rang::style::bold;
    Trace::Debug() << result.ToString().c_str();
    // Reapply bold style after output
    cout << rang::style::bold;
}

void Executor::DumpContinuation(Continuation const &C, int ip) {
    KAI_UNUSED_2(C, ip);
}

void Executor::SetTraceLevel(int N) { _traceLevel = N; }

int Executor::GetTraceLevel() const { return _traceLevel; }

void Executor::Register(Registry &R, const char *N) {
    ClassBuilder<Executor>(R, N).Methods(
        "SetTraceLevel", &Executor::SetTraceLevel)("GetTraceLevel",
                                                   &Executor::GetTraceLevel)
        //.Properties
        //    ("Continuation", &Executor::continuation)
        //    ("Context", &Executor::context)
        //    ("Data", &Executor::data)
        ;
}

void Executor::Dump(Object const &Q) {
    if (_traceLevel > 0) {
        if (_traceLevel > 1) {
            std::cout << "Stack:\n";
            DumpStack(*_data);
        }

        if (_traceLevel > 2) {
            std::cout << "Context:\n";
            for (auto c : *_context) {
                StringStream str;
                str << c;
                cout << str.ToString().c_str() << endl;
            }
        }

        std::cout << "\n[" << _stepNumber << "]: Eval: @"
                  << *_continuation->index << " " << Q.ToString().c_str()
                  << "\n";  // std::endl;
    }
}

std::string Executor::PrintStack() const {
    int n = 0;
    StringStream str;
    for (const auto &elem : _data->GetStack())
        str << "[" << n++ << "]: " << elem << "\n";

    return str.ToString().c_str();
}

std::ostream &operator<<(std::ostream &out, const String &str) {
    return out << str.c_str();
}

bool IsNumber(const Object &obj) {
    switch (obj.GetTypeNumber().ToInt()) {
        case Type::Number::Signed32:
        case Type::Number::Single:
            return true;
    }

    return false;
}

void WriteHumanReadableString(std::ostream &out, const Object &obj) {
    if (IsNumber(obj)) {
        out << obj.ToString();
        return;
    }

    // Only use bold style, no dim styling at all
    const auto bold = rang::style::bold;

    const auto str = obj.ToString();
    switch (obj.GetTypeNumber().ToInt()) {
        case Type::Number::Bool:
            out << bold << rang::fg::cyan << str;
            break;

        case Type::Number::String:
            // Keep quotes bold as well - no dim styling at all
            out << bold << '"' << str << '"';
            break;

        case Type::Number::Label:
            KAI_NOT_IMPLEMENTED();
            break;

        case Type::Number::Pathname: {
            const auto &label = ConstDeref<Pathname>(obj);
            out << bold;
            if (label.Quoted()) out << '\'';
            out << rang::fg::cyan << str;
        } break;

        default:
            out << bold << str;
            break;
    }
    // Always reapply bold after writing
    out << bold;
}

void Executor::PrintStack(std::ostream &out) const {
    int n = 0;
    for (const auto &obj : _data->GetStack()) {
        // Removed dim styling, using only bold styling with color
        out << rang::style::bold << rang::fg::gray << "[" << n++
            << "]: ";
        out << rang::style::bold << rang::fg::yellow;
        WriteHumanReadableString(out, obj);
        // Use newline character instead of std::endl to avoid flushing
        out << "\n";
        // Reapply bold after each line to ensure it persists
        out << rang::style::bold;
    }
    
    // Make sure we maintain bold style after printing the stack
    out << rang::style::bold;
}

// TODO: put container size in traits, as per above.
static int ContainerSize(Object cont) {
    switch (cont.GetTypeNumber().ToInt()) {
        case Type::Number::List:
            return ConstDeref<List>(cont).Size();
        case Type::Number::Array:
            return ConstDeref<Array>(cont).Size();
        case Type::Number::Map:
            return ConstDeref<Map>(cont).Size();
            // case Type::Number::Set:
            //     return ConstDeref<Set>(cont).Size();
            //     break;
    }

    KAI_NOT_IMPLEMENTED();

    return 0;
}

const char *ToString(Language lang) {
    switch (lang) {
        case Language::None:
            return "none";
        case Language::Pi:
            return "pi";
        case Language::Rho:
            return "rho";
        case Language::Tau:
            return "tau";
        case Language::Hlsl:
            return "hlsl";
    }

    return "Unknown Language";
}

bool Executor::PopBool() {
    Object Q = Pop();
    return Q.Exists() && Q.GetClass()->Boolean(Q);
}

// # MARK Perform
void Executor::Perform(Operation::Type op) {
    switch (op) {
        case Operation::ToPi:
            Deref<Compiler>(_compiler).SetLanguage(
                static_cast<int>(Language::Pi));
            break;

        case Operation::ToRho:
            Deref<Compiler>(_compiler).SetLanguage(
                static_cast<int>(Language::Rho));
            break;

        case Operation::Lookup:
            Push(Resolve(Pop()));
            break;

        case Operation::SetManaged: {
            auto object = Pop();
            object.SetManaged(ConstDeref<bool>(Pop()));

            break;
        }

        case Operation::SetChild: {
            Pointer<Label> label = Pop();
            Pop().SetChild(*label, Pop());

            break;
        }

        case Operation::GetChild: {
            Pointer<Label> label = Pop();
            Push(Pop().GetChild(*label));

            break;
        }

        case Operation::RemoveChild: {
            Pointer<Label> label = Pop();
            Pop().RemoveChild(*label);

            break;
        }

        case Operation::Freeze:
            Push(Bin::Freeze(Pop()));
            break;

        case Operation::Thaw:
            Push(Bin::Thaw(Pop()));
            break;

        case Operation::ToVector2: {
            Pointer<float> y = Pop();
            Pointer<float> x = Pop();
            auto V = New<Vector2>();
            V->x = *x;
            V->y = *y;
            Push(V);

            break;
        }

        case Operation::ToVector3: {
            Value<float> z = Pop();
            Value<float> y = Pop();
            Value<float> x = Pop();
            Value<Vector3> V = New<Vector3>();
            V->x = *x;
            V->y = *y;
            V->z = *z;
            Push(V);

            break;
        }

        case Operation::ToVector4: {
            Value<float> w = Pop();
            Value<float> z = Pop();
            Value<float> y = Pop();
            Value<float> x = Pop();
            Value<Vector4> V = New<Vector4>();
            V->x = *x;
            V->y = *y;
            V->z = *z;
            V->w = *w;
            Push(V);

            break;
        }

        case Operation::LevelStack: {
            const int required_depth = _continuation->InitialStackDepth;
            int depth = _data->Size();

            if (depth < required_depth)
                KAI_THROW_0(EmptyStack);  // we lost some objects off the stack

            int num_pops = depth - required_depth;
            for (int N = 0; N < num_pops; ++N) _data->Pop();

            break;
        }

        case Operation::PostInc: {
            Value<int> N = Pop();
            Value<int> M = New<int>();
            int &ref = *N;
            *M = ref;
            ++ref;
            Push(M);

            break;
        }

        case Operation::PostDec: {
            Value<int> N = Pop();
            Value<int> M = New<int>();
            int &ref = *N;
            *M = ref;
            --ref;
            Push(M);

            break;
        }

        case Operation::PreInc: {
            Pointer<int> N = Pop();
            ++*N;
            Push(N);

            break;
        }

        case Operation::PreDec: {
            Pointer<int> N = Pop();
            --*N;
            Push(N);

            break;
        }

        case Operation::Break:
            _break = true;
            break;

        case Operation::WhileLoop: {
            const Pointer<Continuation> body = Pop();
            const Pointer<Continuation> test = Pop();

            _context->Push(_continuation);
            
            // Execute test, continue if true
            while (true) {
                // Run the test condition
                _context->Push(Object());
                Continue(test);
                
                // Check if test passed
                if (!PopBool())
                    break;
                
                // Run the body
                ContinueOnly(body);
                
                // Break if requested
                if (_break)
                    break;
            }

            _context->Pop();

            break;
        }
        
        case Operation::ForLoop: {
            // The for loop takes 4 arguments:
            // 1. The initialization continuation
            // 2. The test continuation
            // 3. The increment continuation
            // 4. The body continuation
            const Pointer<Continuation> body = Pop();
            const Pointer<Continuation> increment = Pop();
            const Pointer<Continuation> test = Pop();
            const Pointer<Continuation> init = Pop();
            
            _context->Push(_continuation);
            
            // Run initialization once
            _context->Push(Object());
            Continue(init);
            
            // Execute loop
            while (true) {
                // Run the test condition
                _context->Push(Object());
                Continue(test);
                
                // Check if test passed
                if (!PopBool())
                    break;
                
                // Run the body
                _context->Push(Object());
                Continue(body);
                
                // Break if requested during body execution
                if (_break)
                    break;
                
                // Run the increment
                _context->Push(Object());
                Continue(increment);
                
                // Break if requested during increment
                if (_break)
                    break;
            }
            
            _context->Pop();
            
            break;
        }
        
        case Operation::DoLoop: {
            // Do-while loop takes 2 arguments:
            // 1. The body continuation
            // 2. The test continuation (executed after the body)
            const Pointer<Continuation> test = Pop();
            const Pointer<Continuation> body = Pop();
            
            _context->Push(_continuation);
            
            do {
                // Run the body
                _context->Push(Object());
                Continue(body);
                
                // Break if requested during body execution
                if (_break)
                    break;
                
                // Run the test condition
                _context->Push(Object());
                Continue(test);
                
                // Continue if test is true
            } while (PopBool() && !_break);
            
            _context->Pop();
            
            break;
        }

        case Operation::ThisContinuation:
            Push(_continuation);
            break;

        case Operation::Delete:
            Pop().Delete();
            break;

        case Operation::GetProperty: {
            Label const &L = ConstDeref<Label>(Pop());
            Object Q = Pop();
            Push(Q.GetClass()->GetProperty(L).GetValue(Q));

            break;
        }

        case Operation::SetProperty: {
            Label const &L = ConstDeref<Label>(Pop());
            Object Q = Pop();
            Q.GetClass()->GetProperty(L).SetValue(Q, Pop());

            break;
        }

        case Operation::Suspend: {
            if (_data->Size() < 1) {
                KAI_TRACE_ERROR() << "Suspend: nothing to suspend to";
                KAI_NOT_IMPLEMENTED();
            }

            auto where_to_go = Resolve(Pop());
            switch (where_to_go.GetTypeNumber().GetValue()) {
                case Type::Number::Function:
                    ConstDeref<BasePointer<FunctionBase> >(where_to_go)
                        ->Invoke(*where_to_go.GetRegistry(), *_data);
                    return;

                case Type::Number::SignedContinuation: {
                    SignedContinuation &signed_continuation =
                        Deref<SignedContinuation>(where_to_go);
                    signed_continuation.Enter(*_data);
                    where_to_go = signed_continuation.GetContinuation();
                    break;
                }

                case Type::Number::Continuation:
                    break;
            }

            _context->Push(_continuation);
            _context->Push(where_to_go);

            if (_traceLevel > 10) KAI_TRACE_2(_continuation, where_to_go);

            if (where_to_go.IsType<Continuation>())
                Deref<Continuation>(where_to_go).Enter(this);

            _break = true;

            break;
        }

        case Operation::Return: {
            int n = 0;
            for (auto sc : *_context) {
                if (*Deref<Continuation>(sc).scopeBreak) break;

                ++n;
            }

            for (; n > 0; --n) _context->Pop();

            break;
        }

        case Operation::Replace:
            _context->Push(NewContinuation(Pop()));
            // fallthrough
        case Operation::Resume:
            _break = true;
            break;

        case Operation::NTimes: {
            int M = ConstDeref<int>(Pop());
            if (M == 0) return;

            Pointer<Continuation> C = Pop();
            for (int N = 0; N < M; ++N) {
                // Push a null continuation to break the call chain.
                _context->Push(Object());
                // Re-continue the functor.
                Continue(C);
            }

            break;
        }

        case Operation::ForEach: {
            Object F = Pop();
            Object C = Pop();

            switch (C.GetTypeNumber().ToInt()) {
                case Type::Number::Array:
                    Push(ForEach(ConstDeref<Array>(C), F));
                    break;

                case Type::Number::Stack:
                    Push(ForEach(ConstDeref<Stack>(C), F));
                    break;

                case Type::Number::List:
                    Push(ForEach(ConstDeref<List>(C), F));
                    break;
                    
                /* Set not implemented yet
                case Type::Number::Set:
                    Push(ForEach(ConstDeref<Set>(C), F));
                    break;
                */
                    
                case Type::Number::Map:
                    Push(ForEach(ConstDeref<Map>(C), F));
                    break;
                    
                case Type::Number::String: {
                    // Special case for strings - iterate over characters
                    auto array = New<Array>();
                    const String& str = ConstDeref<String>(C);
                    for (auto ch : str) {
                        // Convert character to string and push
                        String charStr(1, ch);
                        Push(New(charStr));
                        
                        // Run the function on this character
                        _context->Push(Object());
                        Continue(F);
                        
                        // Store the result
                        array->Append(Pop());
                    }
                    Push(array);
                    break;
                }
                    
                default: {
                    String msg = String("ForEach not implemented for type ") + 
                        C.GetClass()->GetName().ToString();
                    KAI_THROW_1(Base, msg.c_str());
                    break;
                }
            }

            break;
        }
        
        case Operation::ForEachNetwork: {
            // ForEachNetwork takes 3 arguments:
            // 1. The function to apply to each element
            // 2. The collection to iterate over
            // 3. The network node to use for distributed execution (or null for local execution)
            
            Object F = Pop();      // The function to apply
            Object C = Pop();      // The collection to iterate over
            Object N = Pop();      // The network node (or null for local execution)
            
            // For this implementation, we'll just do local processing 
            // regardless of whether a network node is provided
            
            // Use local ForEach operation
            switch (C.GetTypeNumber().ToInt()) {
                case Type::Number::Array:
                    Push(ForEach(ConstDeref<Array>(C), F));
                    break;

                case Type::Number::Stack:
                    Push(ForEach(ConstDeref<Stack>(C), F));
                    break;

                case Type::Number::List:
                    Push(ForEach(ConstDeref<List>(C), F));
                    break;
                    
                case Type::Number::Map:
                    Push(ForEach(ConstDeref<Map>(C), F));
                    break;
                    
                case Type::Number::String: {
                    // Special case for strings, implement directly
                    auto array = New<Array>();
                    const String& str = ConstDeref<String>(C);
                    for (auto ch : str) {
                        // Convert character to string and push
                        String charStr(1, ch);
                        Push(New(charStr));
                        
                        // Run the function on this character
                        _context->Push(Object());
                        Continue(F);
                        
                        // Store the result
                        array->Append(Pop());
                    }
                    Push(array);
                    break;
                }
                    
                default: {
                    String msg = String("ForEachNetwork not implemented for type ") + 
                        C.GetClass()->GetName().ToString();
                    KAI_THROW_1(Base, msg.c_str());
                    break;
                }
            }
            
            break;
        }

        case Operation::Executor:
            Push(*Self);
            break;

        case Operation::ForEachContained: {
            // ForEachContained applies a function to each property/field of an object
            Object func = Pop();
            Object obj = Pop();
            
            if (!obj.Exists())
                KAI_THROW_0(NullObject);
                
            auto result = New<Array>();
            const StorageBase& storage = GetStorageBase(obj);
            
            // Get all children
            for (const auto& pair : storage.GetDictionary()) {
                // Push the key (label)
                Push(New(pair.first));
                
                // Push the value
                Push(pair.second);
                
                // Run the function on this pair
                _context->Push(Object());
                Continue(func);
                
                // Store the result
                result->Append(Pop());
            }
            
            Push(result);
            break;
        }

        case Operation::If: {
            if (!PopBool()) Pop();

            break;
        }

        case Operation::IfElse: {
            if (_data->Size() < 3) {
                KAI_TRACE_ERROR() << "Attempting IfElse, but stack of "
                                  << _data->Size() << " is too small.";
                KAI_NOT_IMPLEMENTED();
            }

            Object condition = Pop();
            Object falseClause = Pop();
            Object trueClause = Pop();

            if (ConstDeref<bool>(condition))
                Push(trueClause);
            else
                Push(falseClause);

            break;
        }

        case Operation::IfThenSuspend: {
            Object then = Pop();
            if (PopBool()) {
                _context->Push(_continuation);
                _context->Push(NewContinuation(then));
                _break = true;
            }

            break;
        }

        case Operation::IfThenSuspendElseSuspend: {
            const Pointer<Continuation> else_ = Pop();
            const Pointer<Continuation> then = Pop();
            _context->Push(_continuation);
            if (PopBool())
                _context->Push(NewContinuation(then));
            else
                _context->Push(NewContinuation(else_));

            _break = true;

            break;
        }

        case Operation::IfThenReplace:
            // ConditionalContextSwitch(Operation::Replace);
            break;

        case Operation::IfThenResume:
            // ConditionalContextSwitch(Operation::Resume);
            break;

        case Operation::Assign: {
            Object lhs = Pop();
            Object rhs = Pop();
            lhs.GetClass()->Assign(lhs.GetStorageBase(), rhs.GetStorageBase());

            break;
        }

        case Operation::ThisContext:
            Push(_continuation);
            break;

        case Operation::Remove:
            Remove(_tree->GetRoot(), _continuation->GetScope(), Pop());
            break;

        case Operation::MarkAndSweep:
            MarkAndSweep();
            break;

        case Operation::DropN:
            DropN();
            break;

        case Operation::Over: {
            Object A = Pop();
            Object B = Pop();
            Push(B);
            Push(A);
            Push(B);

            break;
        }

        case Operation::True:
            Push(New(true));
            break;

        case Operation::False:
            Push(New(false));
            break;

        case Operation::CppFunctionCall: {
            Object Q = Pop();
            ConstDeref<BasePointer<FunctionBase> >(Q)->Invoke(*Q.GetRegistry(),
                                                              *_data);

            break;
        }

        case Operation::Trace: {
            Trace(Pop());
            break;
        }

        case Operation::TraceAll:
            TraceAll();
            break;

        case Operation::Name:
            Push(New(GetName(Pop())));
            break;

        case Operation::Fullname:
            Push(New(GetFullname(Pop())));
            break;

        case Operation::New: {
            Object Q = Pop();
            switch (Q.GetTypeNumber().ToInt()) {
                case Type::Number::String:
                    Push(Reg().NewFromClassName(ConstDeref<String>(Q).c_str()));
                    break;

                case Type::Number::TypeNumber:
                    Push(Reg().NewFromTypeNumber(ConstDeref<Type::Number>(Q)));
                    break;

                case Type::Number::Class:
                    Push(Reg().NewFromClass(ConstDeref<const ClassBase *>(Q)));
                    break;

                default:
                    KAI_THROW_1(CannotNew, Q);
                    break;
            }

            break;
        }

        case Operation::Assert: {
            if (!PopBool()) {
                KAI_TRACE_ERROR_1(_continuation->Show()) << "Assertion failed";
                KAI_THROW_0(Assertion);
            }

            break;
        }

        case Operation::Ref:
            Push(Top());
            break;

        case Operation::Drop:
            Pop();
            break;

        case Operation::Swap: {
            auto A = Pop();
            auto B = Pop();
            Push(A);
            Push(B);

            break;
        }

        case Operation::Dup: {
            auto Q = Pop();
            Push(Q);
            Push(Q.Duplicate());

            break;
        }

        case Operation::Rot: {
            auto A = Pop();
            auto B = Pop();
            auto C = Pop();
            Push(B);
            Push(A);
            Push(C);

            break;
        }

        case Operation::Clear:
            _data->Clear();
            break;

        case Operation::Depth:
            Push(New(_data->Size()));
            break;

        case Operation::ToPair: {
            auto B = Pop();
            auto A = Pop();
            Push(New(Pair(A, B)));

            break;
        }

        case Operation::ToArray:
            ToArray();
            break;

        case Operation::Self:
            Push(_tree->GetScope());
            break;

        case Operation::This:
            Push(_continuation);
            break;

        case Operation::Expand:
            Expand();
            break;

        case Operation::TypeOf:
            Push(New(Pop().GetClass()));
            break;

        case Operation::Exists:
            Push(New(TryResolve(Pop()).Exists()));
            break;

        case Operation::Contents:
            Push(_tree->GetScope());
            GetChildren();
            break;

        case Operation::GetChildren:
            GetChildren();
            break;

        case Operation::ChangeScope: {
            Object id = Pop();
            if (GetTypeNumber(id) == Type::Number::Label)
                _tree->SetScope(GetStorageBase(_tree->GetScope())
                                    .Get(ConstDeref<Label>(id)));
            else
                _tree->SetScope(ConstDeref<Pathname>(id));

            break;
        }

        case Operation::PlusEquals: {
            auto arg = Pop();
            auto from = Pop();
            // TODO: this is lame. need to generalise across all numerics
            if (arg.IsType<float>() && from.IsType<float>()) {
                Deref<float>(from) += ConstDeref<float>(arg);
                break;
            }

            if (arg.IsType<int>() && from.IsType<int>()) {
                Deref<int>(from) += ConstDeref<int>(arg);
                break;
            }

            from.GetClass()->Assign(from, from.GetClass()->Plus(from, arg));

            break;
        }

        case Operation::MinusEquals: {
            Object arg = Pop();
            Object from = Pop();
            Object result = from.GetClass()->Minus(from, arg);
            from.GetClass()->Assign(from, result);

            break;
        }

        case Operation::MulEquals: {
            Object arg = Pop();
            Object from = Pop();
            Object result = from.GetClass()->Multiply(from, arg);
            from.GetClass()->Assign(from, result);

            break;
        }

        case Operation::DivEquals: {
            Object arg = Pop();
            Object from = Pop();
            Object result = from.GetClass()->Divide(from, arg);
            from.GetClass()->Assign(from, result);

            break;
        }

        case Operation::ModEquals:
            KAI_NOT_IMPLEMENTED();
            break;

        case Operation::Plus: {
            Object B = Pop();
            Object A = Pop();
            Push(A.GetClass()->Plus(A, B));

            break;
        }

        case Operation::Minus: {
            Object B = Pop();
            Object A = Pop();
            Push(A.GetClass()->Minus(A, B));

            break;
        }

        case Operation::Multiply: {
            Object B = Pop();
            Object A = Pop();
            Push(A.GetClass()->Multiply(A, B));

            break;
        }

        case Operation::Divide: {
            Object B = Pop();
            Object A = Pop();
            Push(A.GetClass()->Divide(A, B));

            break;
        }

        case Operation::Store: {
            Object ident = Pop();
            Object value = Pop();
            Set(_tree->GetRoot(), _continuation->GetScope(), ident, value);

            break;
        }

        case Operation::Retreive:
            Push(Resolve(Pop(), true));
            break;

        case Operation::Size:
            Push(New(ContainerSize(Pop())));
            break;

        case Operation::Less: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A.GetClass()->Less2(A, B)));

            break;
        }

        case Operation::NotEquiv: {
            Object B = Pop();
            Object A = Pop();
            Push(New(!A.GetClass()->Equiv2(A, B)));

            break;
        }

        case Operation::Equiv: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A.GetClass()->Equiv2(A, B)));

            break;
        }

        case Operation::Greater: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A.GetClass()->Greater2(A, B)));

            break;
        }

        case Operation::CppMethodCall: {
            const Label &method_name = ConstDeref<Label>(Pop());
            Object object = Pop();
            if (!object.Exists()) KAI_THROW_0(NullObject);

            const ClassBase *klass = object.GetClass();
            MethodBase *method = klass->GetMethod(method_name);
            if (method == 0)
                KAI_THROW_2(UnknownMethod, method_name.ToString(),
                            klass->GetName().ToString());

            method->Invoke(object, *_data);

            break;
        }

        case Operation::LogicalNot: {
            Push(New(!PopBool()));

            break;
        }

        case Operation::LogicalXor: {
            Push(New(PopBool() ^ PopBool()));

            break;
        }

        case Operation::LogicalAnd: {
            Push(New(PopBool() && PopBool()));

            break;
        }

        case Operation::LogicalOr: {
            Push(New(PopBool() || PopBool()));

            break;
        }

        case Operation::Pick: {
            int n = ConstDeref<int>(Pop());
            Push(Duplicate(_data->At(n)));

            break;
        }

        case Operation::ToList: {
            auto list = New<List>();
            int n = ConstDeref<int>(Pop());
            while (n-- > 0) {
                list->Append(Pop());
            }

            Push(list);

            break;
        }

#define OPERATION_NOT_IMPLEMENTED(Op) \
    case Operation::Op: {             \
        KAI_NOT_IMPLEMENTED_1(#Op);   \
    } break;
            OPERATION_NOT_IMPLEMENTED(IfThenReplaceElseSuspend);
            OPERATION_NOT_IMPLEMENTED(IfThenResumeElseSuspend);
            OPERATION_NOT_IMPLEMENTED(IfThenSuspendElseReplace);
            OPERATION_NOT_IMPLEMENTED(IfThenReplaceElseReplace);
            OPERATION_NOT_IMPLEMENTED(IfThenResumeElseReplace);
            OPERATION_NOT_IMPLEMENTED(IfThenSuspendElseResume);
            OPERATION_NOT_IMPLEMENTED(IfThenReplaceElseResume);
            OPERATION_NOT_IMPLEMENTED(IfThenResumeElseResume);
            OPERATION_NOT_IMPLEMENTED(RotN);
            OPERATION_NOT_IMPLEMENTED(LessOrEquiv);
            OPERATION_NOT_IMPLEMENTED(GreaterOrEquiv);
            OPERATION_NOT_IMPLEMENTED(LogicalNand);
            OPERATION_NOT_IMPLEMENTED(BitwiseNot);
            OPERATION_NOT_IMPLEMENTED(BitwiseAnd);
            OPERATION_NOT_IMPLEMENTED(BitwiseOr);
            OPERATION_NOT_IMPLEMENTED(BitwiseXor);
            OPERATION_NOT_IMPLEMENTED(BitwiseNand);
    }
}

StringStream &operator<<(StringStream &str, const Executor &exec) {
    return str << "Executor " << exec.Self->GetHandle()
               << ", data.size=" << exec.GetDataStack()->Size()
               << ", context.size=" << exec.GetContextStack()->Size();
}

KAI_END

// EOF
