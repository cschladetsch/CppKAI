#pragma once

#include <KAI/Core/FwdDeclarations.h>

#include <memory>
#include <string>

KAI_BEGIN

class String {
   public:
    typedef char Char;
    typedef typename std::basic_string<Char> Storage;
    // typedef std::string Storage;
    typedef typename Storage::const_iterator const_iterator;
    typedef typename Storage::iterator iterator;

   private:
    Storage _string;

   public:
    String() {}
    template <class II>
    String(II A, II B) {
        _string.assign(A, B);
    }
    String(const Char *S) {
        if (S != 0) _string = S;
    }
    String(const std::string &X) { _string = X; }
    String(const String &X) { _string = X._string; }
    //    String(String &&X) { _string = std::move(X._string); }
    explicit String(int N, Char C) : _string(N, C) {}

    friend bool operator==(String const &A, String const &B) {
        return A._string == B._string;
    }

    friend bool operator!=(String const &A, String const &B) {
        return A._string != B._string;
    }

    friend bool operator<(String const &A, String const &B) {
        return A._string < B._string;
    }

    friend bool operator<=(String const &A, String const &B) {
        return A._string <= B._string;
    }

    friend String &operator+=(String &A, Char B) {
        A._string += B;
        return A;
    }

    friend String &operator+=(String &A, String const &B) {
        A._string += B._string;
        return A;
    }

    // friend String operator+(String &A, Char B)
    // {
    //     A._string += B;
    //     return std::move(A);
    // }

    friend String operator+(String const &A, String const &B) {
        return String(A._string + B._string);
    }

    // We need this for the Boolean property to work properly
    explicit operator bool() const
    {
        return !empty();
    }

    const_iterator begin() const { return _string.begin(); }
    const_iterator end() const { return _string.end(); }

    iterator begin() { return _string.begin(); }
    iterator end() { return _string.end(); }

    iterator erase(iterator A, iterator B) { return _string.erase(A, B); }

    template <class II0, class II1>
    void insert(II0 where, II1 begin, II1 end) {
        _string.insert(where, begin, end);
    }

    int size() const { return (int)_string.size(); }
    bool empty() const { return _string.empty(); }
    std::string StdString() const { return _string; }
    const Storage &GetStorage() const { return _string; }
    void clear() { _string.clear(); }
    int Size() const { return size(); }
    bool Empty() const { return empty() || _string[0] == 0; }
    const_iterator Begin() const { return begin(); }
    void Clear() { _string.clear(); }

    String tolower() const { return LowerCase(); }
    String toupper() { return UpperCase(); }

    String LowerCase() const;
    String UpperCase() const;
    String Capitalise() const;

    bool Contains(String const &) const;
    bool StartsWith(String const &) const;
    bool EndsWith(String const &) const;

    Char &operator[](int N) { return _string.at(N); }
    const Char &operator[](int N) const { return _string.at(N); }
    const Char *c_str() const { return _string.c_str(); }

    void ReplaceFirst(String const &what, String const &with);
    void ReplaceLast(String const &what, String const &with);
    void RemoveAll(String const &what);

    void Insert(Char const *);

    static void Register(Registry &);

    friend bool operator<(const String &A, const String &B);
    friend bool operator==(const String &A, const String &B);
    friend bool operator>(const String &A, const String &B);
};

StringStream &operator<<(StringStream &, const String &);
StringStream &operator>>(StringStream &, String &);
BinaryStream &operator<<(BinaryStream &, const String &);
BinaryStream &operator>>(BinaryStream &, String &);

KAI_END

namespace boost {
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || \
    defined(_MSC_VER) || defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((const unsigned short *)(d)))
#endif

#if !defined(get16bits)
#define get16bits(d)                                      \
    ((((size_t)(((const unsigned char *)(d))[1])) << 8) + \
     (size_t)(((const unsigned char *)(d))[0]))
#endif
inline size_t hash_value(KAI_NAMESPACE(String) const &string) {
    size_t len = string.size();
    size_t hash = len, tmp;
    if (len <= 0) return 0;
    const char *data = reinterpret_cast<const char *>(&*string.begin());
    size_t rem = len & 3;
    len >>= 2;
    // Main loop; 4 bytes each iteration
    for (; len > 0; len--) {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 4;
        hash += hash >> 11;
    }
    // handle end cases
    switch (rem) {
        case 3:
            hash += get16bits(data);
            hash ^= hash << 16;
            hash ^= data[sizeof(unsigned short)] << 18;
            hash += hash >> 11;
            break;
        case 2:
            hash += get16bits(data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1:
            hash += *data;
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    // Force "avalanching" of final 127 bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash;
}
#undef get16bits
}  // namespace boost
