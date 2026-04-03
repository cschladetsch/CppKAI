#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Properties.h>
#include <KAI/Core/Type/TraitMacros.h>

#include <vector>

#include "KAI/Core/BinaryPacket.h"
#include "KAI/Core/StringStream.h"

KAI_BEGIN

// A BinaryStream is-a BinaryPacket which can also resize and allows insertion
class BinaryStream : public BinaryPacket {
   public:
    typedef char Byte;
    typedef std::vector<Byte> Bytes;

   private:
    Bytes bytes;

   public:
    BinaryStream() {}
    BinaryStream(Registry &R) : BinaryPacket(R) {}
    BinaryStream(int start_size) : bytes(start_size) {}

    BinaryStream &Write(int len, const Byte *src);
    void Clear();

    template <class POD>
    BinaryStream &Write(const POD &pod) {
        return Write(sizeof(pod), reinterpret_cast<const Byte *>(&pod));
    }

    friend bool operator<(const BinaryPacket &A, const BinaryPacket &B);
    friend bool operator==(const BinaryPacket &A, const BinaryPacket &B);

    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, BinaryStream const &);
BinaryStream &operator<<(BinaryStream &, BinaryStream const &);
BinaryPacket &operator>>(BinaryPacket &, BinaryStream &);

KAI_TYPE_TRAITS(BinaryStream, Number::BinaryStream,
                Properties::StringStreamInsert |
                    Properties::BinaryStreamExtract |
                    Properties::BinaryStreamInsert)

KAI_END
