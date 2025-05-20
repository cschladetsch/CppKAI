#include <KAI/Language/Tau/TauAstNode.h>

#include <iostream>

TAU_BEGIN

const char *TauAstEnumType::ToString(Enum ty) {
    switch (ty) {
#define CASE(N)             \
    case TauAstEnumType::N: \
        return #N;
#define CASE_LOWER(N)       \
    case TauAstEnumType::N: \
        return ToLower(#N);
#define CASE_REPLACE(N, M)  \
    case TauAstEnumType::N: \
        return M;

        case None:
            return "";

            CASE(Namespace)
            CASE(Class)
            CASE(Property)
            CASE(Method)
            CASE(TokenType)
            CASE(Module)
            CASE(Argument)
            CASE(Arglist)
            CASE(Value)
            CASE(Inherits)
            CASE(Interface)
            CASE(Event)
            CASE(Struct)
            CASE(EnumType)
            CASE(MemberAccess)
        default:
            std::cerr << "TauAstNode: Case not used: " << ty << std::endl;
            return "??";
    }
}

TAU_END
