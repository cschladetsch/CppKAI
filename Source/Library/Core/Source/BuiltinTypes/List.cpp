#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

void List::Append(Object const &Q) {
    if (Attach(Q)) objects.push_back(Q);
}

List::iterator List::Erase(Object const &Q) {
    List::iterator iter = begin(), last = end();
    for (; iter != last; ++iter) {
        if (iter->GetHandle() == Q.GetHandle()) {
            return Erase(iter);
        }
    }
    KAI_THROW_1(UnknownObject, Q.GetHandle());
}

List::iterator List::Erase(iterator A) {
    if (A->Exists()) Detach(*A);
    return objects.erase(A);
}

Object List::Pop() {
    if (objects.empty()) {
        KAI_THROW_0(EmptyStack);
    }
    Object Q = objects.back();
    Erase(--objects.end());
    return Q;
}

void List::Clear() {
    iterator A = begin(), B = end();
    while (A != B) A = Erase(A);
}

BinaryStream &operator<<(BinaryStream &stream, List const &list) {
    int size = list.Size();
    stream << size;
    for (auto object : list) stream << object;
    return stream;
}

BinaryStream &operator>>(BinaryStream &stream, List &list) {
    int size = 0;
    stream >> size;
    for (int n = 0; n < size; ++n) {
        Object object;
        stream >> object;
        list.Append(object);
    }
    return stream;
}

StringStream &operator<<(StringStream &stream, List const &list) {
    stream << "List[";
    String sep = "";
    for (auto object : list) {
        stream << sep << object;
        if (sep.empty()) sep = ", ";
    }
    return stream << "]";
}

void List::Register(Registry &R) {
    ClassBuilder<List>(R, Label(Type::Traits<List>::Name()))
        .Methods("PopBack", &List::PopBack,
                 "Remove last object from the sequence")(
            "PushBack", &List::Append2, "Add an Object to the end")(
            "Erase", &List::Erase2, "Remove an object from the sequence")(
            "Size", &List::Size, "Get the size")(
            "Empty", &List::Empty, "Returns True if Array has no Objects")(
            "Clear", &List::Clear, "Removes all Objects from the Array");
}

KAI_END
