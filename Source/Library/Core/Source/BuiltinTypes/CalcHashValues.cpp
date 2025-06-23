#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Number.h>

KAI_BEGIN

// THESE ARE TERRIBLE HASH FUNCTIONS FOR CONTAINERS

HashValue GetHash(const Stack &S) { return 42; }

HashValue GetHash(const Array &A) {
    if (A.Empty()) return Type::Number::Array;
    return GetHash(A.At(0));
}

HashValue GetHash(const Map &M) {
    if (M.Empty()) return Type::Number::Map;
    Map::const_iterator A = M.Begin();
    return GetHash(A->first) ^ GetHash(A->second);
}

HashValue GetHash(const String &S) {
    HashValue hash = 5381;
    String::const_iterator A = S.begin(), B = S.end();
    for (; A != B; ++A) hash = ((hash << 5) + hash) + (int)*A;
    return hash;
}

KAI_END
