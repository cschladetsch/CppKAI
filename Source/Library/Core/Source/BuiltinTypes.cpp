#include "KAI/Core/BuiltinTypes.h"

#include "KAI/Core/Exception.h"
#include "KAI/Core/FunctionBase.h"
#include "rang.hpp"

KAI_BEGIN

StringStream &operator>>(StringStream &S, float &F) {
    // return S >> F;
    KAI_NOT_IMPLEMENTED();
}

/* TODO WTF
StringStream &operator<<(StringStream &S, const BasePointerBase &B)
{
    B.Self->GetClass()->Insert(S, *B.Self);
    return S;
}
*/

void BasePointerBase::Register(Registry &R) {
    ClassBuilder<BasePointerBase>(R, Label("BasePointerBase"));
}

void Handle::Register(Registry &R) {
    ClassBuilder<Handle>(R, Label("Handle"))
        .Methods("GetValue", &Handle::GetValue);
}

void Pair::Register(Registry &R) {
    ClassBuilder<Pair>(R, "Pair").Methods.Properties("first", &Pair::first)(
        "second", &Pair::second);
}

void FileLocation::AddLocation(StringStream &str) const {
    // Always include file location information, regardless of TraceFileLocation
    // setting This ensures __FILE__ and __LINE__ always appear in log messages
    std::string loc = file.c_str();

    // Only strip path if that setting is enabled
    if (debug::Trace::StripPath) {
        // Handle both forward and backslashes (Unix and Windows paths)
        size_t lastSlash = loc.find_last_of('/');
        size_t lastBackslash = loc.find_last_of('\\');

        // Find the position of the last separator (whichever was found last)
        size_t lastSeparator = std::string::npos;
        if (lastSlash != std::string::npos &&
            lastBackslash != std::string::npos) {
            lastSeparator = std::max(lastSlash, lastBackslash);
        } else if (lastSlash != std::string::npos) {
            lastSeparator = lastSlash;
        } else if (lastBackslash != std::string::npos) {
            lastSeparator = lastBackslash;
        }

        // Extract the filename if a separator was found
        if (lastSeparator != std::string::npos) {
            loc = loc.substr(lastSeparator + 1);
        }
    }

    // Always show file and line information regardless of TraceFileLocation
    // setting
    if (!loc.empty()) {
#ifdef __MSVC__
        // Format is [filename(line)] for MSVC
        str << "[" << loc.c_str() << "(" << line << ")] ";
#else
        // Format is [filename:line] for other compilers
        str << "[" << loc.c_str() << ":" << line << "] ";
#endif
    }
}

void FileLocation::AddFunction(StringStream &str) const {
    if (debug::Trace::TraceFunction && !function.Empty())
        str << function << ": ";
}

String FileLocation::ToString() const {
    StringStream str;
    AddLocation(str);
    AddFunction(str);
    str << Ends;
    return str.ToString();
}

template <class Callable>
void WriteArgumentList(StringStream &S,
                       const typename Callable::ArgumentTypes &arguments) {
    S << "(";
    String sep = "";
    for (const auto &A : arguments) {
        S << sep << A.ToString();
        sep = ", ";
    }

    S << ")";
}

String MethodBase::ToString() const {
    StringStream S;
    S << return_type.ToString() << String(" ") << class_type.ToString()
      << String("::") << name;
    WriteArgumentList<MethodBase>(S, arguments);

    if (constness == Constness::Const)
        S << " const;";
    else
        S << ";";

    if (!Description.Empty()) S << " /* " << Description << " */";

    return S.ToString();
}

String FunctionBase::ToString() const {
    StringStream S;
    S << return_type.ToString() << " " << name;
    WriteArgumentList<FunctionBase>(S, arguments);
    S << ";";

    if (!Description.Empty()) S << " /* " << Description << " */";

    return S.ToString();
}

void FunctionBase::Register(Registry &R) {
    ClassBuilder<BasePointer<FunctionBase> >(R, "Function");
}

StringStream &operator<<(StringStream &S, int N) {
    S << std::to_string(N);
    return S;
}

StringStream &operator>>(StringStream &S, int &N) {
    String T;
    S >> T;
    N = atoi(T.c_str());
    return S;
}

StringStream &operator<<(StringStream &S, bool B) {
    if (B)
        S << "true";
    else
        S << "false";

    return S;
}

StringStream &operator<<(StringStream &S, double N) {
    KAI_UNUSED(N);
    return S;
}

// 'sprintf': This function or variable may be unsafe. Consider using sprintf_s
// instead.
#pragma warning(disable : 4996)

StringStream &operator<<(StringStream &S, float N) {
    char buffer[1000];
    sprintf(buffer, "%f", N);
    return S << buffer;
}

HashValue GetHash(const Pair &P) { return GetHash(P.first); }

StringStream &operator<<(StringStream &S, Type::Number T) {
    return S << T.ToString();  //"TypeNumber: " << T.ToInt() << "(" <<
                               // T.ToString() << ")";
}

StringStream &operator<<(StringStream &S, BasePointer<PropertyBase> const &P) {
    return S << "Property: name_=" << P->GetFieldName()
             << ", type=" << P->GetFieldTypeNumber().value
             << ", class=" << P->GetClassTypeNumber().value;
}

StringStream &operator<<(StringStream &S, Vector2 const &V) {
    return S << V.x << " " << V.y;
}

StringStream &operator>>(StringStream &S, Vector2 &V) {
    KAI_UNUSED_2(S, V);
    KAI_NOT_IMPLEMENTED();
}

BinaryStream &operator<<(BinaryStream &S, Vector2 const &V) {
    return S << V.x << V.y;
}

BinaryStream &operator>>(BinaryStream &S, Vector2 &V) {
    return S >> V.x >> V.y;
}

StringStream &operator<<(StringStream &S, Vector3 const &V) {
    return S << V.x << " " << V.y << " " << V.z;
}

StringStream &operator>>(StringStream &S, Vector3 &V) {
    return S >> V.x >> V.y >> V.z;
}

BinaryStream &operator<<(BinaryStream &S, Vector3 const &V) {
    return S << V.x << V.y << V.z;
}

BinaryStream &operator>>(BinaryStream &S, Vector3 &V) {
    return S >> V.x >> V.y >> V.z;
}

StringStream &operator<<(StringStream &S, Vector4 const &V) {
    return S << V.x << " " << V.y << " " << V.x << " " << V.w;
}

StringStream &operator>>(StringStream &, Vector4 &) { KAI_NOT_IMPLEMENTED(); }

BinaryStream &operator<<(BinaryStream &S, Vector4 const &V) {
    return S << V.x << V.y << V.z << V.w;
}

BinaryStream &operator>>(BinaryStream &S, Vector4 &V) {
    return S >> V.x >> V.y >> V.z >> V.w;
}

StringStream &operator<<(StringStream &S, std::size_t N) { return S << (int)N; }

StringStream &operator<<(StringStream &S, Handle H) {
    return S << "#" << H.GetValue();
}

StringStream &operator<<(StringStream &S, Pair const &P) {
    return S << "Pair(" << P.first << ", " << P.second << ")";
}

StringStream &operator>>(StringStream &S, Pair &P) {
    KAI_UNUSED_2(S, P);
    KAI_NOT_IMPLEMENTED();
}

BinaryStream &operator<<(BinaryStream &S, Pair const &P) {
    return S << P.first << P.second;
}

BinaryStream &operator>>(BinaryStream &S, Pair &P) {
    return S >> P.first >> P.second;
}

KAI_END

// EOF
