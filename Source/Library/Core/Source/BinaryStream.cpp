
#include "KAI/Core/BinaryStream.h"

#include "KAI/Core/Object.h"

KAI_BEGIN

BinaryStream &operator<<(BinaryStream &S, const BinaryPacket &T) {
    int size = T.Size();
    S << size;
    if (size > 0) S.Write(size, T.Begin());
    return S;
}

BinaryStream &operator<<(BinaryStream &S, const BinaryStream &T) {
    return S << static_cast<const BinaryPacket &>(T);
}

void BinaryStream::Register(Registry &registry) {
    ClassBuilder<BinaryStream>(registry,
                               Label(Type::Traits<BinaryStream>::Name()))
        .Methods("Size", &BinaryStream::Size)("Clear", &BinaryStream::Clear);
}

KAI_END
