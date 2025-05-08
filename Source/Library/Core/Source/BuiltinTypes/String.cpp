#include <KAI/Core/Object/ClassBuilder.h>
#include <KAI/Core/Type/Properties.h>

#include <algorithm>

#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

String String::LowerCase() const {
    String result((int)_string.size(), ' ');
    std::transform(_string.begin(), _string.end(), result.begin(), ::tolower);
    return result;
}

String String::UpperCase() const {
    String result((int)_string.size(), ' ');
    std::transform(_string.begin(), _string.end(), result.begin(), ::toupper);
    return result;
}

String String::Capitalise() const {
    // if (_string.empty())
    //     return String();

    // String result = _string;
    // result[0] = (String::Char)::toupper(result[0]);
    // return result;
    KAI_NOT_IMPLEMENTED();
}

bool String::Contains(String const &) const { KAI_NOT_IMPLEMENTED(); }

bool String::StartsWith(String const &) const { KAI_NOT_IMPLEMENTED(); }

void String::ReplaceFirst(String const &what, String const &with) {
    KAI_UNUSED_2(what, with);
    KAI_NOT_IMPLEMENTED();
}

void String::ReplaceLast(String const &what, String const &with) {
    KAI_UNUSED_2(what, with);
    KAI_NOT_IMPLEMENTED();
}

void String::RemoveAll(String const &what) {
    KAI_UNUSED(what);
    KAI_NOT_IMPLEMENTED();
    // boost::algorithm::erase_all(*this, what);
}

bool String::EndsWith(String const &) const { KAI_NOT_IMPLEMENTED(); }

BinaryStream &operator<<(BinaryStream &S, const String &T) {
    int length = T.Size();
    S << length;
    if (length > 0) S.Write(length, (char *)&*T.Begin());

    return S;
}

BinaryStream &operator>>(BinaryStream &S, String &T) {
    int length = 0;
    S >> length;
    if (length == 0) {
        T = "";
        return S;
    }

    // TODO: allocate from String directly
    char *buffer = new char[length + 1];
    S.Read(length, buffer);
    buffer[length] = 0;
    T = buffer;
    delete[] buffer;
    return S;
}

void String::Register(Registry &R) {
    ClassBuilder<String>(R, Label("String"))
        .Methods("Size", &String::Size)("Empty", &String::Empty)(
            "Clear", &String::Clear);
    // Note: Plus and Equiv operations are already registered via
    // KAI_TYPE_TRAITS in TraitMacros.h
}

KAI_END
