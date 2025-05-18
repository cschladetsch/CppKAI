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

    Object GetCompiler() const { return compiler_; }
    void SetCompiler(Object c) { compiler_ = c; }

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

    void SetTree(Tree *T) { tree_ = T; }
    Tree *GetTree() const { return tree_; }

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
        try {
            // Extract the identifier from the object
            Ident const &ident = ConstDeref<Ident>(Q);
            
            // For quoted identifiers, just push the original object
            if (ident.Quoted()) {
                if (traceLevel_ > 3) {
                    KAI_TRACE() << "EvalIdent: Pushing quoted identifier: " << ident.ToString();
                }
                Push(Q);
                return;
            }
            
            // Try to resolve the identifier
            auto found = TryResolve(ident);
            
            // If found, push it onto the stack
            if (found.Valid() && found.Exists()) {
                if (traceLevel_ > 3) {
                    KAI_TRACE() << "EvalIdent: Resolved identifier " << ident.ToString() 
                              << " to " << found.ToString();
                    if (found.GetClass()) {
                        KAI_TRACE() << "  (Type: " << found.GetClass()->GetName() << ")";
                    }
                }
                
                // Handle special case of resolved continuation
                if (found.GetTypeNumber() == Type::Number::Continuation) {
                    // For continuations, execute them directly
                    Continue(found);
                } else {
                    // For other types, push the resolved object
                    Push(found);
                }
            } else {
                // If not found, throw an exception
                KAI_TRACE_ERROR() << "EvalIdent: Object not found: " << ident.ToString();
                KAI_THROW_1(ObjectNotFound, ident.ToString());
            }
        }
        catch (const Exception::Base& e) {
            KAI_TRACE_ERROR() << "EvalIdent: KAI exception: " << e.ToString();
            throw; // Rethrow to let the caller handle it
        }
        catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "EvalIdent: std::exception: " << e.what();
            throw; // Rethrow to let the caller handle it
        }
        catch (...) {
            KAI_TRACE_ERROR() << "EvalIdent: Unknown exception";
            throw; // Rethrow to let the caller handle it
        }
    }

    void Push(Object const &);
    void Push(const std::pair<Object, Object> &);
    Object Pop();
    Object Top() const;

    Value<Stack> GetDataStack();
    Value<const Stack> GetDataStack() const {
        return Value<const Stack>(data_.GetConstObject());
    }
    
    // Add setter for data stack to support RhoTranslator
    void SetDataStack(Value<Stack> stack) { data_ = stack; }
    
    // could be const, but more fun to mess with the context stack as needed
    // elsewhere
    Value</*const*/ Stack> GetContextStack() const;

    // This resets the entire process, other than static state stored
    // in referenced objects
    void ClearStacks() {
        data_->Clear();
        context_->Clear();
    }

    static void Register(Registry &, const char * = "Executor");

    friend bool operator<(const Executor &A, const Executor &B);
    friend bool operator==(const Executor &A, const Executor &B);

    void ClearContext();

    void DropN();
    
    // Helper method for handling Pi language operations
    void ContinuePi();
    
    // Helper method for evaluating continuations
    void EvalContinuation(Object const &Q);
    
    
    // Helper method to perform binary operations with proper type handling
    // This method is used by tests to directly execute binary operations
    Object PerformBinaryOp(Object const &A, Object const &B, Operation::Type op);
    
    // Helper method to determine if an operation is a binary operation
    bool IsBinaryOp(Operation::Type op);
    
    // Note: Special pattern handling for "5 dup +" is now done in the Dup operation

    // if ignoreQuote is true, then we resolve the identifier
    // even if it is quoted
    Object Resolve(Object, bool ignoreQuote = false) const;
    Object Resolve(const Label &) const;
    Object Resolve(const Pathname &) const;

   public:
    // Execute a Pi operation directly (moved from protected to support tests)
    void Perform(Operation::Type op);
    
   protected:
    bool PopBool();

    void ToArray();
    void ProcessToArray(int len); // Helper method for ToArray

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

    Object TryResolve(Object const &) const;
    Object TryResolve(Label const &label) const;
    Object TryResolve(Pathname const &label) const;

   private:
    Value<Continuation> continuation_;
    Value<Stack> context_;
    Value<Stack> data_;
    Object compiler_;
    bool break_;
    Tree *tree_;
    int traceLevel_;
    int stepNumber_;
    // Executor only handles Pi language operations
};

StringStream &operator<<(StringStream &, Executor const &);
BinaryStream &operator<<(BinaryStream &, Executor const &);
BinaryPacket &operator>>(BinaryPacket &, Executor &);

KAI_END
