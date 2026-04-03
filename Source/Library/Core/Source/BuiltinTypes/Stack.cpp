
#include <algorithm>

#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

void Stack::Register(Registry &R) {
    ClassBuilder<Stack>(R, Label("Stack"))
        .Methods("Pop", &Stack::Pop)("Top", &Stack::Top)("Size", &Stack::Size)(
            "Empty", &Stack::Empty);
}

bool Stack::Destroy() {
    Clear();
    return true;
}

Object Stack::At(int N) const {
    if (N < 0) KAI_THROW_1(BadIndex, N);
    if (N >= Size()) KAI_THROW_0(EmptyStack);
    return stack[Size() - 1 - N];
}

Object Stack::Top() const {
    if (stack.empty()) KAI_THROW_0(EmptyStack);
    return stack.back();
}

void Stack::Push(Object const &Q) { stack.push_back(Q); }

void Stack::Clear() {
    while (!Empty()) Pop();
}

Stack::iterator Stack::Erase(Object const &Q) {
    iterator A = std::find(begin(), end(), Q);
    if (A != end()) return Erase(A);
    KAI_THROW_1(UnknownObject, Q.GetHandle());
}

Stack::iterator Stack::Erase(iterator A) {
    Detach(*A);
    return stack.erase(A);
}

Object Stack::Pop() {
    if (stack.empty()) KAI_THROW_0(EmptyStack);
    Object Q = Top();
    Detach(Q);
    stack.pop_back();
    return Q;
}

StringStream &operator<<(StringStream &stream, const Stack &stack) {
    stream << "Stack[";
    String sep;
    for (auto obj : stack) {
        stream << sep << obj;
        if (sep.empty()) sep = ", ";
    }
    return stream << "]";
}

BinaryStream &operator<<(BinaryStream &S, const Stack &T) {
    int length = T.Size();
    S << length;
    for (auto const &obj : T) {
        S << obj;
    }
    return S;
}

BinaryStream &operator>>(BinaryStream &S, Stack &T) {
    T.Clear();
    int length = 0;
    S >> length;
    if (length < 0) {
        KAI_THROW_1(BadIndex, length);
    }
    for (int N = 0; N < length; ++N) {
        Object obj;
        S >> obj;
        T.Push(obj);
    }
    return S;
}

KAI_END
