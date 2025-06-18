#include <KAI/Language/Tau/TauToken.h>

#include <iostream>

TAU_BEGIN

#undef CASE
#undef CASE_LOWER
#undef CASE_REPLACE

const char *TauTokenEnumType::ToString(Enum val) {
    switch (val) {
#define CASE(N)           \
    case TauTokenEnum::N: \
        return #N;
#define CASE_LOWER(N)     \
    case TauTokenEnum::N: \
        return ToLower(#N);
#define CASE_REPLACE(N, M) \
    case TauTokenEnum::N:  \
        return M;

        case TauTokenEnumType::None:
            return "";
            CASE(Ident)
            CASE(OpenParan)
            CASE(CloseParan)
            CASE(Value)
            CASE(Const)
            CASE(Reference)
            CASE(Proxy)
            CASE(Agent)
            CASE(Semi)
            CASE(NewLine)
            CASE(OpenBrace)
            CASE(CloseBrace)
            CASE(Module)
            CASE(Namespace)
            CASE(ConstProxy)
            CASE(ConstAgent)
            CASE(ArrayProxy)
            CASE(QuotedIdent)
            CASE(Array)
            CASE(Sync)
            CASE(Class)
            CASE(Comma)
            CASE(Whitespace)
            CASE(Comment)
            CASE(Async)
            CASE(Tab)
            CASE(String)
            CASE(Number)
            CASE(Assign)
            CASE(Dot)
            CASE(Interface)
            CASE(Event)
            CASE(Struct)
            CASE(EnumKeyword)
            CASE(ConstKeyword)
            CASE(ShellCommand)
        default:
            std::cerr << "TauToken: Case not used: " << val << std::endl;
            return "??";
    }
}

std::ostream &operator<<(std::ostream &out, TauToken const &node) {
    if (node.type == TauTokenEnumType::None) return out;

    out << TauTokenEnumType::ToString(node.type);
    switch (node.type) {
        case TauTokenEnumType::Ident:
            out << "=" << node.Text();
    }

    return out;
}

TAU_END
