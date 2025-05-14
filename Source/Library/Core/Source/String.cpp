#include <KAI/Core/BuiltinTypes.h>

KAI_BEGIN

StringStream &operator>>(StringStream &S, String &T) {
    T.clear();
    char C = 0;
    while (S.Extract(C)) {
        if (!isspace(C)) {
            T += C;
            break;
        }
    }

    while (S.Extract(C)) {
        if (isspace(C)) break;
        T += C;
    }

    return S;
}

bool operator>(const String &A, const String &B) {
    return A.string_ > B.string_;
}

KAI_END
