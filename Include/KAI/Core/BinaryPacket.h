#pragma once

#include <KAI/Core/Config/Base.h>

#include "Registry.h"

KAI_BEGIN

/// A Binary-packet is a fixed-size sequence of bytes which allows only
/// extraction this allows it to be used to extract data from network packets
/// without copying.
class BinaryPacket {
   public:
    typedef char Byte;
    typedef const Byte *const_iterator;

   protected:
    const_iterator first, current, last;
    Registry *registry;

   public:
    BinaryPacket() : registry(0) { first = last = current = 0; }
    BinaryPacket(Registry &R) : registry(&R) { first = last = current = 0; }
    BinaryPacket(const_iterator F, const_iterator L, Registry *R = 0)
        : first(F), last(L), current(F), registry(R) {}

    const_iterator Begin() const { return first; }
    const_iterator Current() const { return current; }
    const_iterator Last() const { return last; }

    int Size() const { return (int)(last - first); }
    bool Empty() const { return last == first; }
    bool Read(int len, Byte *dest);
    bool CanRead(int len) const;

    template <class POD>
    bool Read(POD &pod) {
        return Read(sizeof(pod), reinterpret_cast<Byte *>(&pod));
    }

    void SetRegistry(Registry *R) { registry = R; }
    Registry *GetRegistry() const { return registry; }

    static void Register(Registry &, const char *);

    friend bool operator<(const BinaryPacket &A, const BinaryPacket &B);
    friend bool operator==(const BinaryPacket &A, const BinaryPacket &B);
};

StringStream &operator<<(StringStream &, BinaryPacket const &);
BinaryStream &operator<<(BinaryStream &, BinaryPacket const &);
BinaryPacket &operator>>(BinaryPacket &, BinaryPacket &);

KAI_TYPE_TRAITS(BinaryPacket, Number::BinaryPacket,
                Properties::StringStreamInsert |
                    Properties::BinaryStreamExtract |
                    Properties::BinaryStreamInsert)

KAI_END
