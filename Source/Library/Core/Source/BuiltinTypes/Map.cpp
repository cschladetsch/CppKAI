
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Core/Object/Object.h"

KAI_BEGIN

StringStream &operator>>(StringStream &S, std::pair<Object, Object> &P) {
    return S >> P.first >> P.second;
}

StringStream &operator>>(StringStream &S, Map &M) { return S; }

// StringStream &operator<<(StringStream &S, typename Map::const_iterator const
// &P)
//{
//     KAI_UNUSED(P);
//     return S << "MapConstIterator";
// }

std::size_t HashMapCompare::operator()(const Object &A) const {
    if (!A.Valid()) return 0;
    return GetHash(A);
}

bool HashMapCompare::operator()(const Object &A, const Object &B) const {
    return A == B;
}

bool MapCompare::operator()(const Object &A, const Object &B) const {
    return A < B;
}

KAI_END

// EOF
