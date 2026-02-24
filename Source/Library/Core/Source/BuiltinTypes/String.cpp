#include <KAI/Core/Object/ClassBuilder.h>
#include <KAI/Core/Type/Properties.h>

#include <algorithm>
#include <cctype>

#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

String String::LowerCase() const {
    String result((int)string_.size(), ' ');
    std::transform(string_.begin(), string_.end(), result.begin(),
                   [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
    return result;
}

String String::UpperCase() const {
    String result((int)string_.size(), ' ');
    std::transform(string_.begin(), string_.end(), result.begin(),
                   [](unsigned char ch) {
                       return static_cast<char>(std::toupper(ch));
                   });
    return result;
}

String String::Capitalise() const {
    if (string_.empty()) return String();
    String result = LowerCase();
    result.string_[0] = static_cast<char>(
        std::toupper(static_cast<unsigned char>(result.string_[0])));
    return result;
}

bool String::Contains(String const &substr) const {
    return string_.find(substr.string_) != std::string::npos;
}

bool String::StartsWith(String const &prefix) const {
    if (prefix.string_.size() > string_.size()) return false;
    return string_.compare(0, prefix.string_.size(), prefix.string_) == 0;
}

void String::ReplaceFirst(String const &what, String const &with) {
    size_t pos = string_.find(what.string_);
    if (pos != std::string::npos)
        string_.replace(pos, what.string_.length(), with.string_);
}

void String::ReplaceLast(String const &what, String const &with) {
    size_t pos = string_.rfind(what.string_);
    if (pos != std::string::npos)
        string_.replace(pos, what.string_.length(), with.string_);
}

void String::RemoveAll(String const &what) {
    size_t pos = 0;
    while ((pos = string_.find(what.string_, pos)) != std::string::npos) {
        string_.erase(pos, what.string_.length());
    }
}

bool String::EndsWith(String const &suffix) const {
    if (suffix.string_.size() > string_.size()) return false;
    return string_.compare(string_.size() - suffix.string_.size(),
                           suffix.string_.size(), suffix.string_) == 0;
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
    if (length < 0) {
        KAI_THROW_1(BadIndex, length);
    }
    if (length == 0) {
        T = "";
        return S;
    }

    if (!S.CanRead(length)) {
        KAI_THROW_0(PacketExtraction);
    }

    std::string buffer(static_cast<std::size_t>(length), '\0');
    if (!S.Read(length, &buffer[0])) {
        KAI_THROW_0(PacketExtraction);
    }
    T = buffer;
    return S;
}

void String::Register(Registry &R) {
    ClassBuilder<String>(R, Label("String"))
        .Methods("Size", &String::Size)("Empty", &String::Empty)(
            "Clear", &String::Clear);
    // Note: Plus and Equiv operations are already registered via
    // KAI_TYPE_TRAITS in TraitMacros.h
}

// Add the missing ostream operator for String
std::ostream &operator<<(std::ostream &stream, const String &str) {
    return stream << str.StdString();
}

KAI_END
