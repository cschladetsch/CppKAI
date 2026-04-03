#include "KAI/Language/Hlsl/HlslAstNode.h"

KAI_BEGIN

#undef CASE
#undef CASE_LOWER
#undef CASE_REPLACE

const char *HlslAstNodes::ToString(Enum ty) {
    switch (ty) {
#define CASE(N)           \
    case HlslAstNodes::N: \
        return #N;
#define CASE_LOWER(N)     \
    case HlslAstNodes::N: \
        return ToLower(#N);
#define CASE_REPLACE(N, M) \
    case HlslAstNodes::N:  \
        return M;
        CASE(None)
        CASE(Program)
        CASE(Ident)
        CASE(GetMember)
        CASE(Function)
        CASE(Block)
        CASE(Conditional)
        CASE(Assignment)
        CASE(Call)
        CASE(ArgList)
        CASE(IndexOp)
        CASE(ForEach)
        CASE(For)
        CASE(List)
        CASE(Map)
        CASE(Continuation)
        CASE(TokenType)
        CASE(Pathname)
    }

    static char buff[BUFSIZ];
    sprintf(buff, "Case %d", ty);
    return buff;
}

KAI_END
