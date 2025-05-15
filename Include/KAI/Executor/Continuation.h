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
    
    // if true, this continuation should be automatically unwrapped when used
    // in binary operations. Used for Pi expressions in Rho language that should
    // return primitives not continuations
    Pointer<bool> specialHandling;

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
    
    // Special handling flag for continuations used in Pi expressions in Rho
    void SetSpecialHandling(bool special) {
        if (!specialHandling.Exists()) {
            specialHandling = New<bool>(special);
        }
        else {
            *specialHandling = special;
        }
    }
    
    bool GetSpecialHandling() const {
        return specialHandling.Exists() && *specialHandling;
    }

    int InitialStackDepth;

    void Enter(Executor *exec);
    void Reset();

    // get next object in the continuation
    bool Next() const;
    bool Next(Object &) const;

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
