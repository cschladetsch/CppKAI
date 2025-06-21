#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Object/ClassBuilder.h>
#include <KAI/Core/Object/Object.h>

KAI_BEGIN

void Array::RemoveAt(int index) {
    if (index < 0 || index >= static_cast<int>(Size())) {
        KAI_THROW_1(BadIndex, index);
    }

    objects.erase(begin() + index);
}

void Array::Insert(int index, Object const &obj) {
    if (index < 0 || index > static_cast<int>(Size())) {
        KAI_THROW_1(BadIndex, index);
    }

    if (Attach(obj)) {
        objects.insert(begin() + index, obj);
    }
}

Array::iterator Array::Erase(iterator A) {
    Detach(*A);
    return objects.erase(A);
}

void Array::Append(Object const &Q) {
    if (Attach(Q)) objects.push_back(Q);
}

Object Array::PopBack() {
    Object Q = objects.back();
    Detach(Q);
    objects.pop_back();
    return Q;
}

void Array::Clear() {
    auto A = begin(), B = end();
    for (; A != B; ++A) Detach(*A);
    objects.clear();
}

void Array::Erase(Object const &Q) { Erase(Q.GetHandle()); }

void Array::Erase(Handle H) {
    auto A = objects.begin(), B = objects.end();
    for (; A != B; ++A) {
        if (A->GetHandle() == H) {
            Erase(A);
            return;
        }
    }
}

StringStream &operator<<(StringStream &S, const Array &A) {
    S << "[";
    auto B = A.Begin(), C = A.End();
    String sep = "";
    for (; B != C; ++B) {
        S << sep << *B;
        sep = " ";
    }
    return S << "]";
}

BinaryStream &operator<<(BinaryStream &S, const Array &Q) {
    int length = Q.Size();
    S << length;
    auto A = Q.Begin(), B = Q.End();
    for (; A != B; ++A) S << *A;
    return S;
}

BinaryStream &operator>>(BinaryStream &S, Array &Q) {
    Q.Clear();
    int length = 0;
    S >> length;
    for (int N = 0; N < length; ++N) {
        Object R;
        S >> R;
        Q.Append(R);
    }
    return S;
}

// Static function for array concatenation
// TODO: Currently unused - consider exposing as a utility function or removing
/*
static Array ConcatArrays(const Array &A, const Array &B) {
    Array result;
    // Copy all elements from first array
    for (auto const &elem : A) {
        result.Append(elem);
    }
    // Append all elements from second array
    for (auto const &elem : B) {
        result.Append(elem);
    }
    return result;
}
*/

void Array::Register(Registry &R) {
    void (Array::*remove_method)(Object) = &Array::Erase2;

    ClassBuilder<Array>(R, Label(Type::Traits<Array>::Name()))
        .Methods
        // TODO ("Append", &Array::Append2, "Add an Object to the end")
        ("Erase", remove_method, "Remove an object from the sequence")(
            "RemoveAt", &Array::RemoveAt,
            "Remove the object at the given index from the sequence")(
            "Insert", &Array::Insert2,
            "Insert an object at the given index in the sequence")(
            "PopBack", &Array::PopBack, "Remove last object from the sequence")(
            "Size", &Array::Size, "Get the size")(
            "Empty", &Array::Empty, "Returns True if Array has no Objects")(
            "At", &Array::At, "Returns the Object at the given Signed32 index")(
            "Clear", &Array::Clear, "Removes all Objects from the Array");
}

KAI_END
