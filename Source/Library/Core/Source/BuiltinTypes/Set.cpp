
#include <KAI/Core/Object/ClassBuilder.h>

#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

void ObjectSet::Register(Registry &R) {
    ClassBuilder<ObjectSet>(R, Label("Set"));
}

bool ObjectSet::Empty() const { return objects.empty(); }

int ObjectSet::Size() const { return static_cast<int>(objects.size()); }

bool ObjectSet::Destroy() {
    Clear();
    return true;
}

void ObjectSet::Insert(Object const &Q) { Append(Q); }

void ObjectSet::Append(Object const &Q) {
    if (Attach(Q)) objects.insert(Q);
}

void ObjectSet::Clear() {
    while (!Empty()) Erase(begin());
}

ObjectSet::iterator ObjectSet::Erase(iterator iter) {
    if (iter->Exists()) Detach(*iter);

    return objects.erase(iter);
}

ObjectSet::iterator ObjectSet::Erase(Object const &Q) {
    iterator A = objects.find(Q);
    if (A != end()) return Erase(A);

    KAI_THROW_1(UnknownObject, Q.GetHandle());
}

StringStream &operator<<(StringStream &S, const ObjectSet &T) {
    return S << "Set: size=" << T.Size();
}

BinaryStream &operator<<(BinaryStream &S, const ObjectSet &T) {
    int length = T.Size();
    S << length;
    for (auto const &obj : T) {
        S << obj;
    }
    return S;
}

BinaryStream &operator>>(BinaryStream &S, ObjectSet &T) {
    T.Clear();
    int length = 0;
    S >> length;
    if (length < 0) {
        KAI_THROW_1(BadIndex, length);
    }
    for (int N = 0; N < length; ++N) {
        Object obj;
        S >> obj;
        T.Append(obj);
    }
    return S;
}

KAI_END
