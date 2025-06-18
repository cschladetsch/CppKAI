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
        CASE(Label)
        CASE(Dot)
        CASE(Comma)
        CASE(If)
        CASE(Else)
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
        CASE(BitNot)
        CASE(LeftShift)
        CASE(RightShift)
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
        CASE(Assert)
        CASE(ToPi)
        CASE(PiSequence)
        CASE(Debug)
        CASE(Quote)
        CASE(Sep)
        CASE(Pathname)
        CASE(AcrossAllNodes)
        CASE(Mod)
        CASE(Colon)
        CASE(ModAssign)
        CASE(DoubleColon)
        CASE(While)
        CASE(For)
        CASE(DoWhile)
        CASE(ForEach)
        CASE(Break)
        CASE(Continue)
        CASE(ShellCommand)
    }

    static char buff[BUFSIZ];
    sprintf(buff, "Unnamed RhoToken %d", val);
    return buff;
}

KAI_END

// Define in global namespace for friend declaration
std::ostream &operator<<(std::ostream &out,
                         kai::RhoTokenEnumType::Type const &node) {
    if (node.type == kai::RhoTokenEnumType::None) return out;

    out << kai::RhoTokenEnumType::ToString(node.type);
    switch (node.type) {
        case kai::RhoTokenEnumType::Int:
        case kai::RhoTokenEnumType::String:
        case kai::RhoTokenEnumType::Label:
        case kai::RhoTokenEnumType::ShellCommand:
            out << "=" << node.Text();
    }

    return out;
}

// Also define in kai namespace for ADL
namespace kai {
std::ostream &operator<<(std::ostream &out,
                         RhoTokenEnumType::Type const &node) {
    return ::operator<<(out, node);
}
}  // namespace kai
