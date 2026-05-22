#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Executor/Continuation.h>

KAI_BEGIN

BinaryStream &operator<<(BinaryStream &stream, const Continuation &cont) {
    const bool entered = cont.entered.Exists() ? *cont.entered : false;
    const bool scope_break =
        cont.scopeBreak.Exists() ? *cont.scopeBreak : false;

    stream << cont.scope;
    stream << static_cast<const Object &>(cont.code);
    stream << static_cast<const Object &>(cont.args);
    stream << static_cast<const Object &>(cont.source_code);
    stream << cont.GetInstructionPointer();
    stream << entered;
    stream << scope_break;
    stream << cont.InitialStackDepth;

    return stream;
}

BinaryStream &operator>>(BinaryStream &stream, Continuation &cont) {
    if (stream.GetRegistry() == 0) KAI_THROW_1(Base, "NullRegistry");

    Registry &registry = *stream.GetRegistry();

    Object scope;
    Object code;
    Object args;
    Object source_code;
    int ip = 0;
    bool entered = false;
    bool scope_break = false;
    int initial_stack_depth = -1;

    stream >> scope;
    stream >> code;
    stream >> args;
    stream >> source_code;
    stream >> ip;
    stream >> entered;
    stream >> scope_break;
    stream >> initial_stack_depth;

    cont.scope = scope;
    cont.code = code.Exists() ? Pointer<Array>(code) : registry.New<Array>();
    cont.args = args.Exists() ? Pointer<Array>(args) : registry.New<Array>();
    cont.source_code = source_code.Exists() ? Pointer<String>(source_code)
                                            : registry.New<String>();
    cont.index = registry.New<int>(ip);
    cont.entered = registry.New<bool>(entered);
    cont.scopeBreak = registry.New<bool>(scope_break);
    cont.InitialStackDepth = initial_stack_depth;

    return stream;
}

KAI_END
