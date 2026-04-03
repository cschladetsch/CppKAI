#pragma once

#include <KAI/Core/Config/Base.h>

#include <string>

#include "String.h"

KAI_BEGIN

/// I chose not to attempt to find commonality between String and WideString.
struct WideString {
    typedef wchar_t Char;
    typedef std::basic_string<Char> Storage;
    typedef Storage::const_iterator const_iterator;
    typedef Storage::iterator iterator;

   private:
    Storage string;

   public:
    WideString() {}
    template <class II>
    WideString(II A, II B) : string(A, B) {}
    WideString(const Char *S) : string(S) {}
    WideString(const Storage &S) : string(S) {}
    WideString(const WideString &X) : string(X.string) {}
    WideString(const String &);

    WideString &operator+=(const String &A);
    WideString &operator+=(const WideString &A) {
        string += A.string;
        return *this;
    }

    WideString &operator+=(String::Char A);
    WideString &operator+=(Char A) {
        string += A;
        return *this;
    }

    friend WideString operator+(const WideString &A, const String &B);
    friend WideString operator+(const WideString &A, const WideString &B) {
        return A.string + B.string;
    }
    friend bool operator<(const WideString &A, const WideString &B);
    friend bool operator==(const WideString &A, const WideString &B);

    const_iterator begin() const { return string.begin(); }
    const_iterator end() const { return string.end(); }
    iterator begin() { return string.begin(); }
    iterator end() { return string.end(); }
    int size() const { return (int)string.size(); }
    bool empty() const { return string.empty(); }
    std::wstring StdString() const { return string; }
    const Storage &GetStorage() const { return string; }
    void clear() { string.clear(); }

    int Size() const { return size(); }
    bool Empty() const { return empty() || string[0] == 0; }
    const_iterator Begin() const { return begin(); }
    void Clear() { string.clear(); }
    const Char &operator[](int N) const { return string.at(N); }
    const Char *wc_str() const { return string.c_str(); }
    const Char *c_str() const;
};

StringStream &operator<<(StringStream &, const WideString &);
StringStream &operator>>(StringStream &, WideString &);
BinaryStream &operator<<(BinaryStream &, const WideString &);
BinaryPacket &operator>>(BinaryPacket &, WideString &);

KAI_END
