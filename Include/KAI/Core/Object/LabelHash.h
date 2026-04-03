#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Object/Label.h>

KAI_BEGIN

namespace detail {

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || \
    defined(_MSC_VER) || defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((const unsigned short *)(d)))
#endif

#if !defined(get16bits)
#define get16bits(d)                                      \
    ((((size_t)(((const unsigned char *)(d))[1])) << 8) + \
     (size_t)(((const unsigned char *)(d))[0]))
#endif

struct LabelHash {
    friend bool operator==(const Label &A, const Label &B) { return A == B; }

    inline unsigned long operator()(Label const &label) const {
        const std::string &string = label.GetValue().GetStorage();

        unsigned long len = string.size();
        unsigned long hash = len, tmp;
        if (len <= 0) {
            return 0;
        }
        const char *data = reinterpret_cast<const char *>(&*string.begin());
        unsigned long rem = len & 3;
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
};
}  // namespace detail

#undef get16bits

KAI_END
