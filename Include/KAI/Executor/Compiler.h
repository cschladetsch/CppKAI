#pragma once

#include <KAI/Core/Debug.h>
#include <KAI/Executor/Continuation.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Common/Language.h>

#include <map>

KAI_BEGIN

// this is a strange beast.
// it works for any language that conforms to the concept defined in
// <KAI/Language/Common/TranslatorBase.h>
class Compiler : public Reflected {
    typedef std::map<Operation, String> OperationToString;
    typedef std::map<String, Pointer<Operation> > StringToOperation;

   private:
    OperationToString op_to_string;
    StringToOperation string_to_op;
    Language language_ = Language::Pi;
    int traceLevel_ = 0;

   public:
    bool Destroy();

    void SetLanguage(int);
    int GetLanguage() const;
    void SetTraceLevel(int n) { traceLevel_ = n; }

    // Template method removed to break dependency on language-specific types
    // Applications should implement their own language compilation

    Pointer<Continuation> Translate(const String &text,
                                    Structure st = Structure::Expression) const;
    Pointer<Continuation> CompileFile(const String &fileName,
                                      Structure st = Structure::Program) const;

    static void Register(Registry &, const char * = "Compiler");

    void AddOperation(int N, const String &S);
};

// you can interact with a Compiler at runtime
KAI_TYPE_TRAITS(Compiler, Number::Compiler, Properties::Reflected);

KAI_END
