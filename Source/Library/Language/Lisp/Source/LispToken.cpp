#include "KAI/Language/Rho/RhoToken.h"

KAI_BEGIN

#undef CASE
#undef CASE_LOWER
#undef CASE_REPLACE

const char *RhoTokenEnumType::ToString(Enum val) {
    switch (val) {
#define CASE(N)        \
    case RhoTokens::N: \
        return #N;
#define CASE_LOWER(N)  \
    case RhoTokens::N: \
        return ToLower(#N);
#define CASE_REPLACE(N, M) \
    case RhoTokens::N:     \
        return M;

        CASE(None)
        CASE(Whitespace)
        CASE(Semi)
        CASE(Int)
        CASE(Float)
        CASE(String)
        CASE(True)
        CASE(False)
        CASE(Return)
        CASE(Ident)
        CASE(Dot)
        CASE(Comma)
        CASE(If)
        CASE(Else)
        CASE(For)
        CASE(While)
        CASE(OpenBrace)
        CASE(CloseBrace)
        CASE(OpenParan)
        CASE(CloseParan)
        CASE(Plus)
        CASE(Minus)
        CASE(Mul)
        CASE(Divide)
        CASE(Assign)
        CASE(Less)
        CASE(Equiv)
        CASE(NotEquiv)
        CASE(Greater)
        CASE(LessEquiv)
        CASE(GreaterEquiv)
        CASE(Not)
        CASE(And)
        CASE(Or)
        CASE(Xor)
        CASE(OpenSquareBracket)
        CASE(CloseSquareBracket)
        CASE(Increment)
        CASE(Decrement)
        CASE(BitAnd)
        CASE(BitOr)
        CASE(BitXor)
        CASE(Self)
        CASE(Lookup)
        CASE(Tab)
        CASE(NewLine)
        CASE(Fun)
        CASE(Comment)
        CASE(Yield)
        CASE(Suspend)
        CASE(Replace)
        CASE(Resume)
        CASE(PlusAssign)
        CASE(MinusAssign)
        CASE(MulAssign)
        CASE(DivAssign)
        CASE(In)
        CASE(Assert)
        CASE(ToPi)
        CASE(PiSequence)
        CASE(Debug)
        CASE(Quote)
        CASE(Sep)
        CASE(Pathname)
    }

    static char buff[BUFSIZ];
    sprintf(buff, "Unnamed RhoToken %d", val);
    return buff;
}

std::ostream &operator<<(std::ostream &out, RhoToken const &node) {
    if (node.type == RhoTokenEnumType::None) return out;

    out << RhoTokenEnumType::ToString(node.type);
    switch (node.type) {
        case RhoTokenEnumType::Int:
        case RhoTokenEnumType::String:
        case RhoTokenEnumType::Ident:
            out << "=" << node.Text();
    }

    return out;
}

KAI_END
