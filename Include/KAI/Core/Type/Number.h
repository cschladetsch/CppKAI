#pragma once

#include <KAI/Core/Base.h>

#include <string>
#include <type_traits>

KAI_TYPE_BEGIN

struct Number {
    enum Value {
        // $BEGIN_TYPE_NUMBER_VALUE_ENUMERATION$
        Undefined = 0,
        None = 1,
        Void = 2,
        Handle = 11,
        Object = 12,
        Type = 13,
        TypeNumber = 14,
        BasePointer = 15,
        Callable = 16,
        Function = 17,
        Method = 18,
        Property = 19,
        Class = 20,
        Bool = 21,
        Signed32 = 22,
        Single = 23,
        Double = 24,
        String = 25,
        Label = 26,
        Pathname = 27,
        Array = 28,
        Pair = 29,
        Map = 30,
        HashMap = 31,
        Set = 32,
        HashSet = 33,
        Stack = 34,
        MapConstIterator = 35,
        MapIterator = 36,
        SetConstIterator = 37,
        SetIterator = 38,
        HashMapConstIterator = 39,
        HashMapIterator = 40,
        HashSetConstIterator = 41,
        HashSetIterator = 42,
        Operation = 43,
        Continuation = 44,
        Compiler = 45,
        Executor = 46,
        StreamBase = 47,
        StringStream = 48,
        XmlStream = 49,
        GraphStream = 50,
        BinaryStream = 51,
        BinaryPacket = 52,
        Console = 53,
        Node = 54,
        Browser = 55,
        SignedContinuation = 56,
        Vector2 = 57,
        Vector3 = 58,
        Vector4 = 59,
        List = 60,

        SystemLast = 200,
        Last = 1000,
        // $END_TYPE_NUMBER_VALUE_ENUMERATION$
    };

    Value value;

    Number() : value(None) {}
    Number(int N) : value(Value(N)) {}
    friend bool operator<(Number A, Number B) { return A.value < B.value; }
    friend bool operator==(Number A, Number B) { return A.value == B.value; }
    friend bool operator!=(Number A, Number B) { return A.value != B.value; }
    std::string ToString() const;
    int ToInt() const { return (int)value; }
    int GetValue() const { return (int)value; }
};

KAI_TYPE_END

KAI_BEGIN

//    StringStream &operator<<(StringStream &, Number""::Number);
//    StringStream &operator>>(StringStream &, Type::Number &);
//    BinaryStream &operator<<(BinaryStream &, Type::Number);
//    BinaryStream &operator>>(BinaryStream &, Type::Number &);

typedef int TypeNumber;

// struct HashType
//{
//     enum { bucket_size = 4, min_buckets = 16 };
//     std::size_t operator()(const Type::Number::Value &A) const
//     {
//         return
//         static_cast<std::size_t>(std::underlying_type<Type::Number::Value>::type(A));
//     }
//
//     bool operator()(const Type::Number::Value &A, const Type::Number::Value
//     &B) const
//     {
//         return A == B;
//     }
// };

KAI_END
