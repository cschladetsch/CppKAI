#pragma once

#include <KAI/Core/Debug.h>
#include <KAI/Executor/Continuation.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Common/Language.h>

#include <functional>
#include <map>

KAI_BEGIN

class Compiler : public Reflected {
    typedef std::map<Operation, String> OperationToString;
    typedef std::map<String, Pointer<Operation> > StringToOperation;
    typedef std::function<Pointer<Continuation>(const String &, Structure)>
        TranslateFunction;

   private:
    OperationToString op_to_string;
    StringToOperation string_to_op;
    Language language_ = Language::Pi;
    int traceLevel_ = 0;
    TranslateFunction translateFunction_;

   public:
    bool Destroy();

    void SetLanguage(int);
    int GetLanguage() const;
    void SetTraceLevel(int n) { traceLevel_ = n; }
    int GetTraceLevel() const { return traceLevel_; }
    void SetTranslateFunction(TranslateFunction func) {
        translateFunction_ = func;
    }

    Pointer<Continuation> Translate(const String &text,
                                    Structure st = Structure::Expression) const;
    Pointer<Continuation> CompileFile(const String &fileName,
                                      Structure st = Structure::Program) const;

    static void Register(Registry &, const char * = "Compiler");

    void AddOperation(int N, const String &S);
};

KAI_TYPE_TRAITS(Compiler, Number::Compiler, Properties::Reflected);

KAI_END
