#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/File.h>
#include <KAI/Executor/Compiler.h>
#include <KAI/Language/Pi/PiTranslator.h>
#include <KAI/Language/Rho/RhoTranslator.h>

using namespace std;

KAI_BEGIN

bool Compiler::Destroy() {
    // These are un-managed while the compiler is alive, to avoid them being
    // part of the GC process. probably an overkill for performance. in any
    // case, when the compiler goes out of scope, we want to ensure the
    // operation objects are returned to the pool for GC.
    //
    // Again, all this work and text describing it is probably more cost than
    // just letting op's be managed from the start.
    for (auto op : string_to_op) op.second.SetManaged(true);

    return true;
}

Pointer<Continuation> Compiler::Translate(const String &text,
                                          Structure st) const {
    switch (language_) {
        case Language::None:
            return Object();

        case Language::Pi:
            return Compile<PiTranslator>(text, st);

        case Language::Rho:
            return Compile<RhoTranslator>(text, st);

        case Language::Tau:
            KAI_NOT_IMPLEMENTED();  // Tau is not a compiled language - it is an
                                    // IDL
            break;
    }

    return Object();
}

void Compiler::AddOperation(int N, const String &S) {
    Operation T = Operation::Type(N);
    Pointer<Operation> Q = Self->GetRegistry()->New(T);
    Q.SetManaged(false);
    string_to_op[S] = Q;
    op_to_string[T] = S;
}

Pointer<Continuation> Compiler::CompileFile(const String &fileName,
                                            Structure st) const {
    auto text = File::ReadAllText(fileName.c_str());
    return Translate(text.c_str(), st);
}

void Compiler::Register(Registry &R, const char *name) {
    ClassBuilder<Compiler>(R, name);
}

void Compiler::SetLanguage(int n) { language_ = static_cast<Language>(n); }

int Compiler::GetLanguage() const { return static_cast<int>(language_); }

// Operator definitions moved to Operation.cpp

KAI_END
