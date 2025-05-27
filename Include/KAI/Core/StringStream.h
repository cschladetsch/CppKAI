#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Config/Base.h>

#include <compare>
#include <string_view>
#include <vector>

KAI_BEGIN

struct Trace;

// Seems like a boring and unnecessary replication of std::stringstream.
// Rather, it allows insertion and extration of arbitrary registered objects
// from a string. It's probably one of the more interesting classes_ in the
// entire KAI system - by the mere fact that it works.
//
// A StringStream is itself a registered object, so they can act recursively
// and be addressed over a Network::Domain.
class StringStream {
   public:
    typedef String::Char Char;
    typedef std::vector<Char> Storage;

   private:
    Storage stream;
    int read_offset;
    Registry *registry;

   public:
    StringStream() : read_offset(0), registry(0) {}
    explicit StringStream(String const &S) : read_offset(0), registry(0) {
        Append(S);
    }

    const Storage &GetStorage() const { return stream; }
    String ToString() const;
    bool Empty() const { return stream.empty(); }
    int Size() const { return (int)stream.size(); }
    void Clear() { stream.clear(); }
    bool CanRead(int = 1) const;

    void SetRegistry(Registry *R) { registry = R; }
    void Append(Char);
    void Append(std::string_view);
    void Append(std::string_view A, std::string_view B);
    void Append(const String &);

    bool Extract(int length, String &);
    bool Extract(Char &);
    char Peek() const;

    static void Register(Registry &);

    friend auto operator<=>(StringStream const &,
                            StringStream const &) = default;
    friend bool operator==(StringStream const &,
                           StringStream const &) = default;
};

struct EndsArgument {};
void Ends(EndsArgument);

StringStream &operator<<(StringStream &, void (*)(EndsArgument));
StringStream &operator<<(StringStream &, const String::Char *);

inline StringStream &operator<<(StringStream &S, const String &T) {
    return S << T.c_str();
}
std::ostream &operator<<(std::ostream &S, const StringStream &T);
std::istream &operator>>(std::istream &S, StringStream &T);

KAI_END
