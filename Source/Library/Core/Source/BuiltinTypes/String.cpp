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

bool String::Contains(String const &substr) const {
    return _string.find(substr._string) != std::string::npos;
}

bool String::StartsWith(String const &prefix) const {
    if (prefix._string.size() > _string.size())
        return false;
    return _string.compare(0, prefix._string.size(), prefix._string) == 0;
}

void String::ReplaceFirst(String const &what, String const &with) {
    size_t pos = _string.find(what._string);
    if (pos != std::string::npos)
        _string.replace(pos, what._string.length(), with._string);
}

void String::ReplaceLast(String const &what, String const &with) {
    size_t pos = _string.rfind(what._string);
    if (pos != std::string::npos)
        _string.replace(pos, what._string.length(), with._string);
}

void String::RemoveAll(String const &what) {
    size_t pos = 0;
    while ((pos = _string.find(what._string, pos)) != std::string::npos) {
        _string.erase(pos, what._string.length());
    }
}

bool String::EndsWith(String const &suffix) const {
    if (suffix._string.size() > _string.size())
        return false;
    return _string.compare(_string.size() - suffix._string.size(),
                         suffix._string.size(), suffix._string) == 0;
}

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
