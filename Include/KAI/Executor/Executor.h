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
    Executor();
    ~Executor();

    friend bool operator<(const Executor &A, const Executor &B);
    friend bool operator==(const Executor &A, const Executor &B);

    void Create();
    bool Destroy();

    void SetScope(Object);
    void PopScope();
    Object GetScope() const;

    void SetContinuation(Value<Continuation>);
    void Continue();
    void Continue(Value<Continuation>);
    void ContinueOnly(Value<Continuation> C);
    void ContinueOneInstruction();

    void SetSingleStep(bool enable) { singleStep_ = enable; }
    bool GetSingleStep() const { return singleStep_; }
    
    bool Step();

    Object GetCompiler() const { return compiler_; }
    void SetCompiler(Object c) { compiler_ = c; }

    void Eval(Object const &Q);
    void Dump(Object const &Q);

    std::string PrintStack() const;
    void PrintStack(std::ostream &out) const;
    void Run();

    template <class T>
    Value<T> New() {
        return Reg().New<T>();
    }

    template <class T>
    Value<T> New(T const &X) {
        return Reg().New(X);
    }

    void SetTree(Tree *T) { tree_ = T; }
    Tree *GetTree() const { return tree_; }

    void SetTraceLevel(int);
    int GetTraceLevel() const;

    template <class T>
    void Push(const Value<T> &val) {
        Push(val.GetObject());
    }

    template <class Ident>
    void EvalIdent(Object const &Q) {
        try {
            // Validate the input object
            if (!Q.Valid()) {
                KAI_TRACE_ERROR() << "EvalIdent: Invalid object";
                return;  // Return early instead of throwing
            }

            // Extract the identifier from the object
            Ident const &ident = ConstDeref<Ident>(Q);

            // For quoted identifiers, just push the original object
            if (ident.Quoted()) {
                if (traceLevel_ > 3) {
                    KAI_TRACE() << "EvalIdent: Pushing quoted identifier: "
                                << ident.ToString();
                }
                Push(Q);
                return;
            }

            // Handle empty labels as a special case
            if constexpr (std::is_same_v<Ident, Label> ||
                          std::is_same_v<Ident, Pathname>) {
                if (ident.ToString().empty()) {
                    KAI_TRACE() << "EvalIdent: Empty identifier name, creating "
                                   "placeholder";
                    // Push an empty object rather than throwing an exception
                    Push(Object());
                    return;
                }
            }

            // Try to resolve the identifier
            auto found = TryResolve(ident);

            // If found, push it onto the stack
            if (found.Valid() && found.Exists()) {
                if (traceLevel_ > 3) {
                    KAI_TRACE()
                        << "EvalIdent: Resolved identifier " << ident.ToString()
                        << " to " << found.ToString();
                    if (found.GetClass()) {
                        KAI_TRACE()
                            << "  (Type: " << found.GetClass()->GetName()
                            << ")";
                    }
                }

                // For all types including continuations, push the resolved
                // object This allows operations like & to control when
                // continuations execute
                Push(found);
            } else {
                // If not found, try to create a placeholder instead of throwing
                // an exception
                if constexpr (std::is_same_v<Ident, Label>) {
                    KAI_TRACE()
                        << "EvalIdent: Object not found: " << ident.ToString()
                        << ", creating placeholder";
                    // Create a placeholder object - use TryResolveOrCreate
                    auto placeholder = TryResolveOrCreate(ident);
                    Push(placeholder);
                } else {
                    // For non-Label types, we still need to handle the error
                    KAI_TRACE_ERROR()
                        << "EvalIdent: Object not found: " << ident.ToString();
                    // Instead of throwing, push an empty object
                    Push(Object());
                }
            }
        } catch (const Exception::Base &e) {
            KAI_TRACE_ERROR() << "EvalIdent: KAI exception: " << e.ToString();
            // Instead of rethrowing, push an empty object to allow execution to
            // continue
            Push(Object());
        } catch (const std::exception &e) {
            KAI_TRACE_ERROR() << "EvalIdent: std::exception: " << e.what();
            // Instead of rethrowing, push an empty object to allow execution to
            // continue
            Push(Object());
        } catch (...) {
            KAI_TRACE_ERROR() << "EvalIdent: Unknown exception";
            // Instead of rethrowing, push an empty object to allow execution to
            // continue
            Push(Object());
        }
    }

    void Push(Object const &);
    void Push(const std::pair<Object, Object> &);
    Object Pop();
    Object Top() const;

    Value<Stack> GetDataStack();
    Value<const Stack> GetDataStack() const {
        if (!data_.Valid() || !data_.Exists()) {
            KAI_TRACE_ERROR() << "GetDataStack: Invalid data stack";
            return Value<const Stack>();
        }
        return Value<const Stack>(data_.GetConstObject());
    }

    void SetDataStack(Value<Stack> stack) { data_ = stack; }

    Value<Stack> GetContextStack() const;

    void ClearStacks() {
        data_->Clear();
        context_->Clear();
    }

    static void Register(Registry &, const char * = "Executor");
    void ClearContext();
    void DropN();
    void ContinuePi();
    void EvalContinuation(Object const &Q);
    bool IsBinaryOp(Operation::Type op);
    Object PerformBinaryOp(Object const &A, Object const &B, Operation::Type op);
    Object Resolve(Object, bool ignoreQuote = false) const;
    Object Resolve(const Label &) const;
    Object Resolve(const Pathname &) const;
    Object TryResolveOrCreate(Label const &label, Type::Number type = Type::Number::None);
    Object ExtractValueFromContinuation(Object const &value);
    Object UnwrapValue(const Object &value);

public:
    void Perform(Operation::Type op);

protected:
    bool PopBool();

    void ToArray();
    void ProcessToArray(int len);

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
    Pointer<Continuation> NewContinuation(Value<Continuation> P);
    void ExecuteContinuationInline(Pointer<Continuation> cont);

    // Records the creating frame on a continuation being pushed as a value, so
    // a lambda can still reach its free variables after the frame that defined
    // them is gone. Returns Q untouched when there is nothing to capture.
    Object CaptureClosure(Object const &Q);

    Object TryResolve(Object const &) const;
    Object TryResolve(Label const &label) const;
    Object TryResolve(Pathname const &label) const;

private:
    Value<Continuation> continuation_;
    Value<Stack> context_;
    Value<Stack> data_;
    Object compiler_;
    bool break_;      // Set by Break operation to exit loops
    bool continue_;   // Set by Continue operation to skip to next loop iteration
    bool replace_;    // Set by Replace operation to replace current continuation
    Tree *tree_;
    int traceLevel_;
    int stepNumber_;
    bool singleStep_;
};

StringStream &operator<<(StringStream &, Executor const &);
BinaryStream &operator<<(BinaryStream &, Executor const &);
BinaryPacket &operator>>(BinaryPacket &, Executor &);

KAI_END

