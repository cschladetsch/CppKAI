#pragma once

#include <KAI/Core/BuiltinTypes/Stack.h>
#include <KAI/Core/Object/Reflected.h>
#include <KAI/Core/Pathname.h>
#include <KAI/Core/Value.h>
#include <KAI/Executor/Continuation.h>
#include <KAI/Executor/Operation.h>

KAI_BEGIN

class Tree;
struct Executor;

KAI_TYPE_TRAITS(Executor, Number::Executor, Properties::Reflected);

struct Executor : Reflected {
    void Create();
    bool Destroy();

    void SetScope(Object);
    void PopScope();
    Object GetScope() const;

    void SetContinuation(Value<Continuation>);
    void Continue(Value<Continuation>);
    void ContinueOnly(Value<Continuation> C);
    void Continue();
    // No need for language-specific methods - Executor only executes Pi

    Object GetCompiler() const { return _compiler; }
    void SetCompiler(Object c) { _compiler = c; }

    void Eval(Object const &Q);
    void Dump(Object const &Q);

    std::string PrintStack() const;
    void PrintStack(std::ostream &out) const;

    template <class T>
    Value<T> New() {
        return Reg().New<T>();
    }

    template <class T>
    Value<T> New(T const &X) {
        return Reg().New(X);
    }

    void SetTree(Tree *T) { _tree = T; }
    Tree *GetTree() const { return _tree; }

    void SetTraceLevel(int);
    int GetTraceLevel() const;

    // Executor only handles Pi language operations
    // No need for language-specific methods

    template <class T>
    void Push(const Value<T> &val) {
        Push(val.GetObject());
    }

    template <class Ident>
    void EvalIdent(Object const &Q) {
        Ident const &ident = ConstDeref<Ident>(Q);
        if (ident.Quoted()) {
            Push(Q);
            return;
        }

        auto found = TryResolve(ident);
        if (found.Exists())
            Push(found);
        else
            KAI_THROW_1(ObjectNotFound, ident.ToString());
    }

    void Push(Object const &);
    void Push(const std::pair<Object, Object> &);
    Object Pop();
    Object Top() const;

    Value<Stack> GetDataStack();
    Value<const Stack> GetDataStack() const {
        return Value<const Stack>(_data.GetConstObject());
    }

    // could be const, but more fun to mess with the context stack as needed
    // elsewhere
    Value</*const*/ Stack> GetContextStack() const;

    // This resets the entire process, other than static state stored
    // in referenced objects
    void ClearStacks() {
        _data->Clear();
        _context->Clear();
    }

    static void Register(Registry &, const char * = "Executor");

    friend bool operator<(const Executor &A, const Executor &B);
    friend bool operator==(const Executor &A, const Executor &B);

    void ClearContext();

    void DropN();

    // if ignoreQuote is true, then we resolve the identifier
    // even if it is quoted
    Object Resolve(Object, bool ignoreQuote = false) const;
    Object Resolve(const Label &) const;
    Object Resolve(const Pathname &) const;

   protected:
    bool PopBool();

    void Perform(Operation::Type op);
    void ToArray();

    void GetChildren();
    void Expand();
    void MarkAndSweep();
    void MarkAndSweep(Object &root);

    void Push(Stack &L, Object const &Q);
    Object Pop(Stack &stack);
    void NextContinuation();

    void DumpStack(Stack const &);
    static void DumpContinuation(Continuation const &, int);

   private:
    template <class C>
    Value<Array> ForEach(C const &, Object const &);
    template <class Cont>
    void PushAll(const Cont &cont);

    void TraceAll();
    void Trace(const Object &);
    void Trace(const Label &, const StorageBase &, StringStream &);
    void Trace(const Object &, StringStream &);
    void ConditionalContextSwitch(Operation::Type);
    Value<Continuation> NewContinuation(Value<Continuation> P);

    Object TryResolve(Object const &) const;
    Object TryResolve(Label const &label) const;
    Object TryResolve(Pathname const &label) const;

   private:
    Value<Continuation> _continuation;
    Value<Stack> _context;
    Value<Stack> _data;
    Object _compiler;
    bool _break;
    Tree *_tree;
    int _traceLevel;
    int _stepNumber;
    // Executor only handles Pi language operations
};

StringStream &operator<<(StringStream &, Executor const &);
BinaryStream &operator<<(BinaryStream &, Executor const &);
BinaryPacket &operator>>(BinaryPacket &, Executor &);

KAI_END
