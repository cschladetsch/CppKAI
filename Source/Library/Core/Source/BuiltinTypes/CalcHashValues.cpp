#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Number.h>

KAI_BEGIN

static HashValue CombineHash(HashValue seed, HashValue value) {
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

HashValue GetHash(const Stack &S) {
    HashValue seed = Type::Number::Stack;
    for (auto const &obj : S) {
        seed = CombineHash(seed, GetHash(obj));
    }
    return seed;
}

HashValue GetHash(const Array &A) {
    HashValue seed = Type::Number::Array;
    for (auto const &obj : A) {
        seed = CombineHash(seed, GetHash(obj));
    }
    return seed;
}

HashValue GetHash(const List &L) {
    HashValue seed = Type::Number::List;
    for (auto const &obj : L) {
        seed = CombineHash(seed, GetHash(obj));
    }
    return seed;
}

HashValue GetHash(const Map &M) {
    HashValue seed = Type::Number::Map;
    for (auto const &entry : M) {
        seed = CombineHash(seed, GetHash(entry.first));
        seed = CombineHash(seed, GetHash(entry.second));
    }
    return seed;
}

HashValue GetHash(const String &S) {
    HashValue hash = 5381;
    String::const_iterator A = S.begin(), B = S.end();
    for (; A != B; ++A) hash = ((hash << 5) + hash) + (int)*A;
    return hash;
}

KAI_END
