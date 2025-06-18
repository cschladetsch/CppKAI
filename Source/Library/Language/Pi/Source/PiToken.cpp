#include <KAI/Language/Pi/PiToken.h>
#include <ctype.h>

#include <algorithm>

using namespace std;

KAI_BEGIN

const char *PiTokenEnumType::ToString(Enum t) {
    switch (t) {
#define CASE(N)       \
    case PiTokens::N: \
        return #N;
#define CASE_LOWER(N) CASE(N)
#define CASE_REPLACE(N, M) \
    case PiTokens::N:      \
        return M;

#ifdef TRACE_WS
        CASE_LOWER(Whitespace)
        CASE_LOWER(NewLine)
        CASE_LOWER(None)
#else
        case PiTokens::NewLine:
        case PiTokens::Whitespace:
        case PiTokens::None:
            return "";
#endif

        CASE_LOWER(Freeze)
        CASE_LOWER(Thaw)
        CASE_LOWER(Bool)
        CASE_LOWER(Greater)
        CASE_LOWER(Debug)
        CASE_LOWER(Int)
        CASE_LOWER(Float)
        CASE_LOWER(String)
        CASE_LOWER(True)
        CASE_LOWER(False)
        CASE_LOWER(Size)
        CASE_LOWER(Print)
        CASE_REPLACE(Suspend, "&")
        CASE_REPLACE(Resume, "...")
        CASE_REPLACE(Replace, "!")
        CASE_LOWER(Ident)
        CASE_REPLACE(QuotedIdent, "'")
        CASE_REPLACE(Dot, ".")
        CASE_REPLACE(Comma, ",")
        CASE_LOWER(If)
        CASE_LOWER(IfElse)
        CASE_LOWER(For)
        CASE_LOWER(While)
        CASE_REPLACE(OpenBrace, "{")
        CASE_REPLACE(CloseBrace, "}")
        CASE_REPLACE(OpenParan, "(")
        CASE_REPLACE(CloseParan, ")")
        CASE_REPLACE(Plus, "+")
        CASE_REPLACE(Minus, "-")
        CASE_REPLACE(Mul, "*")
        CASE_REPLACE(Divide, "div")
        CASE_REPLACE(Assign, ":=")
        CASE_REPLACE(Less, "<")
        CASE_REPLACE(Equiv, "==")
        CASE_REPLACE(NotEquiv, "!=")
        CASE_REPLACE(LessEquiv, "le")
        CASE_REPLACE(GreaterEquiv, "ge")
        CASE_LOWER(Not)
        CASE_LOWER(And)
        CASE_LOWER(Exists)
        CASE_LOWER(Or)
        CASE_LOWER(Xor)
        CASE_REPLACE(OpenSquareBracket, "[")
        CASE_REPLACE(CloseSquareBracket, "]")
        CASE_REPLACE(Increment, "++")
        CASE_REPLACE(Decrement, "--")
        CASE_LOWER(BitAnd)
        CASE_LOWER(BitOr)
        CASE_LOWER(BitXor)
        CASE_REPLACE(Self, "self")
        CASE(This)
        CASE_LOWER(Lookup)
        CASE(Tab)
        CASE(Comment)
        CASE_REPLACE(PlusAssign, "+=")
        CASE_REPLACE(MinusAssign, "-=")
        CASE_REPLACE(MulAssign, "*=")
        CASE_REPLACE(DivAssign, "/=")
        CASE_REPLACE(In, ":")
        CASE_LOWER(Colon)
        CASE_LOWER(Assert)
        CASE_LOWER(Pathname)
        CASE_LOWER(Drop)
        CASE_LOWER(Dup)
        CASE_LOWER(Dup2)
        CASE_LOWER(Drop2)
        CASE_LOWER(Roll)
        CASE_LOWER(Over)
        CASE_LOWER(OverM)
        CASE_LOWER(PickN)
        CASE_LOWER(Depth)
        CASE_LOWER(Swap)
        CASE_LOWER(Rot)
        CASE_LOWER(RotN)
        CASE_LOWER(Clear)
        CASE_LOWER(Expand)
        CASE_LOWER(ToList)
        CASE_LOWER(ToArray)
        CASE_LOWER(ToMap)
        CASE_LOWER(ToSet)
        CASE_LOWER(ToHashMap)
        CASE_LOWER(ToPair)
        CASE_LOWER(ToVector2)
        CASE_LOWER(ToVector3)
        CASE_LOWER(ToVector4)
        CASE_LOWER(ToQuaternion)
        CASE_LOWER(RunScript)
        CASE_LOWER(Name)
        CASE_LOWER(Fullname)
        CASE_LOWER(GetProperty)
        CASE_LOWER(SetProperty)
        CASE_REPLACE(GarbageCollect, "gc")
        CASE_REPLACE(ChangeFolder, "cd")
        CASE_REPLACE(PrintFolder, "pwd")
        CASE_LOWER(Pipe)
        CASE_LOWER(DirectIn)
        CASE_LOWER(DirectOut)
        CASE_LOWER(DirectInOut)
        CASE_LOWER(GetPath)
        CASE_LOWER(GetName)
        CASE_LOWER(SplitPathName)
        CASE_LOWER(MakePathName)
        CASE_LOWER(SetName)
        CASE_LOWER(SetChild)
        CASE_LOWER(GetChild)
        CASE_LOWER(GetMethods)
        CASE_LOWER(GetProperties)
        CASE_REPLACE(GetType, "type")
        CASE_LOWER(GetSignature)
        CASE_LOWER(StringToType)
        CASE_LOWER(TypeToString)
        CASE_LOWER(Version)
        CASE_LOWER(DateNowSimple)
        CASE_LOWER(DateNow)
        CASE_LOWER(DurationSinceStart)
        CASE_LOWER(DeltaTime)
        CASE_LOWER(PushFolder)
        CASE_LOWER(PopFolder)
        CASE_LOWER(History)
        CASE_LOWER(Jobs)
        CASE_LOWER(Store)
        CASE_REPLACE(ToRho, "rho")
        CASE_REPLACE(ToRhoSequence, "rho{")
        CASE_LOWER(Modulo)
        CASE_LOWER(Min)
        CASE_LOWER(Max)
        CASE_LOWER(ShellCommand)
        CASE_REPLACE(ToStr, ">str")
    }

    KAI_TRACE() << " PiToken #" << (int)t;
    KAI_NOT_IMPLEMENTED();
}

KAI_END

// Define in global namespace for friend declaration
std::ostream &operator<<(std::ostream &out,
                         kai::PiTokenEnumType::Type const &node) {
    if (node.type == kai::PiTokenEnumType::None) return out << "<NONE>";

    switch (node.type) {
        case kai::PiTokenEnumType::True: {
            out << "true";
            return out;
        }
        case kai::PiTokenEnumType::False: {
            out << "false";
            return out;
        }
    }

    out << kai::PiTokenEnumType::ToString(node.type);

    switch (node.type) {
        case kai::PiTokenEnumType::Int:
        case kai::PiTokenEnumType::Float:
        case kai::PiTokenEnumType::Pathname:
        case kai::PiTokenEnumType::String:
        case kai::PiTokenEnumType::Ident:
        case kai::PiTokenEnumType::QuotedIdent:
        case kai::PiTokenEnumType::ShellCommand:
            out << "='" << node.Text() << "'";
            break;
    }

    return out;
}

// Also define in kai namespace for ADL
namespace kai {
std::ostream &operator<<(std::ostream &out, PiTokenEnumType::Type const &node) {
    return ::operator<<(out, node);
}
}  // namespace kai
