#pragma once

#include <KAI/Core/BinaryStream.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Object/Reflected.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/Type.h>

KAI_BEGIN

struct Executor;

class Continuation : public Reflected {
   public:
    typedef Pointer</*const*/ Array> Code;

    Object scope;
    Pointer<Array> code;
    Pointer<Array> args;
    Pointer<String> source_code;
    Pointer<int> index;
    Pointer<bool> entered;

    // if true, this is a 'top-level' continuation, so
    // name_ resolution should stop here
    //
    // I hate this idea. needs to be re-thought through clearly.
    Pointer<bool> scopeBreak;

   public:
    void Create();
    bool Destroy();

    template <class T>
    Pointer<T> New() const {
        return Self->GetRegistry()->New<T>();
    }

    template <class T>
    Pointer<T> New(const T &val) const {
        return Self->GetRegistry()->New<T>(val);
    }

    void SetCode(Code);
    void SetCode(Code, String const *source);

    Pointer<const Array> GetCode() const { return code; }

    void AddArg(Label const &arg) { args->Append(New(arg)); }

    Code &GetCode() { return code; }

    Pointer<String> GetSourceCode() const { return source_code; }
    void SetSourceCode(const char *C);

    void SetScope(Object const &Q) { scope = Q; }
    Object GetScope() const { return scope; }
    bool HasScope() const { return scope.Exists(); }

    bool HasCode() const { return code.Exists(); }

    int InitialStackDepth;

    void Enter(Executor *exec);
    void Reset();

    // get next object in the continuation
    bool Next() const;
    bool Next(Object &) const;

    // Special handling for direct continuation evaluation
    // Tests use this functionality and expect it to work
    void SetSpecialHandling(bool v) {
        // We store this in the 'entered' field to maintain compatibility
        // without adding extra fields
        if (!entered.Exists())
            entered = New(v);
        else
            *entered = v;
    }

    bool GetSpecialHandling() const {
        if (!entered.Exists()) return false;
        return *entered;
    }

    // Set the instruction pointer (for jump operations)
    void SetInstructionPointer(int pos) {
        if (!index.Exists())
            index = New<int>(pos);
        else
            *index = pos;
    }

    // Get the current instruction pointer
    int GetInstructionPointer() const {
        if (!index.Exists()) return 0;
        return *index;
    }

    String Show() const;
    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, const Continuation &);
StringStream &operator>>(StringStream &, Continuation &);
BinaryStream &operator<<(BinaryStream &, const Continuation &);
BinaryStream &operator>>(BinaryStream &, Continuation &);

KAI_TYPE_TRAITS(Continuation, Number::Continuation,
                Properties::Streaming | Properties::Reflected);

KAI_END
