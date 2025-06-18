#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/File.h>
#include <KAI/Executor/Compiler.h>

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
    // Use the custom translation function if provided
    if (translateFunction_) {
        return translateFunction_(text, st);
    }

    // Otherwise, this base implementation doesn't know about specific languages
    KAI_UNUSED_2(text, st);
    KAI_NOT_IMPLEMENTED();
    return Object();
}

void Compiler::AddOperation(int id, const String &name) {
    Operation type = Operation::Type(id);
    Pointer<Operation> operation = Self->GetRegistry()->New(type);
    operation.SetManaged(false);
    string_to_op[name] = operation;
    op_to_string[type] = name;
}

Pointer<Continuation> Compiler::CompileFile(const String &fileName,
                                            Structure st) const {
    auto text = File::ReadAllText(fileName.c_str());
    return Translate(text.c_str(), st);
}

void Compiler::Register(Registry &registry, const char *name) {
    ClassBuilder<Compiler>(registry, name);
}

void Compiler::SetLanguage(int n) { language_ = static_cast<Language>(n); }

int Compiler::GetLanguage() const { return static_cast<int>(language_); }

// Operator definitions moved to Operation.cpp

KAI_END
