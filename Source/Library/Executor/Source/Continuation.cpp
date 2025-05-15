#define NOMINMAX

#include <KAI/Core/BuiltinTypes/Bool.h>
#include <KAI/Core/BuiltinTypes/Signed32.h>
#include <KAI/Core/BuiltinTypes/Stack.h>
#include <KAI/Core/BuiltinTypes/Void.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Debug.h>
#include <KAI/Core/Object/ClassBuilder.h>
#include <KAI/Executor/Continuation.h>

#undef min
#undef max

KAI_BEGIN

const static int MaxDepth = 3;
const static int MaxLen = 80;

void Continuation::Create() {
    args = New<Array>();
    index = New(0);
}

bool Continuation::Destroy() { return true; }

String Continuation::Show() const {
    StringStream str;
    int around = 4;
    auto current = *index;
    auto min = std::max(0, current - around);
    auto max = std::min(code->Size() - 1, current + around);
    str << "IP=" << current << "\n";
    for (int n = min; n < max; ++n) {
        if (n == current - 1) str << ">>> ";
        str << code->At(n) << "\n";
    }

    return str.ToString();
}

void Continuation::SetCode(Code C) { code = C; }

void Continuation::Enter(Executor *exec) {
    if (code.Exists() && !code->Empty()) {
        if (!scope.Exists()) scope = exec->New<void>();

        Stack &data = *exec->GetDataStack();
        if (data.Size() < args->Size()) {
            KAI_TRACE_ERROR_2(data.Size(), args->Size())
                << "Failed to enter continuation: not enough args";
            KAI_THROW_0(EmptyStack);
        }

        for (auto arg : *args) {
            Object a = data.Pop();
            scope.Set(ConstDeref<Label>(arg), a);
        }
    }

    *index = 0;
}

bool Continuation::Next() const {
    Object unused;
    return Next(unused);
}

bool Continuation::Next(Object &next) const {
    if (!code.Exists()) return false;

    int &n = Deref<int>(index);
    if (n == code->Size()) return false;

    next = code->At(n++);

    return true;
}

void Continuation::Reset() { *index = 0; }

StringStream &InsertContinuation(StringStream &stream, const Array &code,
                                 size_t index, int depth) {
    // too long
    if (stream.Size() >= MaxLen) return stream << "...}";

    // finished
    if (index == (int)code.Size()) return stream << "}";

    auto const &next = code.At(index);

    // print next thing
    if (next.IsType<Continuation>()) {
        // limit depth of coro pritning
        if (depth == MaxDepth) return stream << "{...}...";

        auto const &coro = ConstDeref<Continuation>(next);
        return InsertContinuation(stream, *coro.GetCode(), 0, ++depth);
    }

    while (code.Size() > (int)index) {
        stream << code.At(index) << " ";
        InsertContinuation(stream, code, ++index, depth);
    }

    return stream;
}

StringStream &operator<<(StringStream &str, const Continuation &cont) {
    str << "Continuation " << cont.Self->GetHandle() << "[";
    for (const auto &cmd : *cont.GetCode()) str << cmd << " ";
    return str << "] @" << cont.index << "/" << cont.code->Size();
}

StringStream &operator>>(StringStream &, Continuation &) {
    KAI_NOT_IMPLEMENTED();
}

BinaryStream &operator<<(BinaryStream &S, const Continuation &C) {
    // TODO: instruction pointer and scope!
    return S << C.GetCode();
}

BinaryStream &operator>>(BinaryStream &S, Continuation &C) {
    Object code;
    // TODO: instruction pointer and scope!
    S >> code;
    C.SetCode(code);
    return S;
}

void Continuation::Register(Registry &R) {
    ClassBuilder<Continuation>(R, "Continuation")
        .Methods.Properties("code", &Continuation::code)(
            "args", &Continuation::args)("scope", &Continuation::scope)(
            "source_code", &Continuation::source_code);
}

KAI_END
