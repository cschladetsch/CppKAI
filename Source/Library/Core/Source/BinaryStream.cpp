
#include "KAI/Core/BinaryStream.h"

#include "KAI/Core/Object.h"

KAI_BEGIN

BinaryStream &operator<<(BinaryStream &S, const BinaryPacket &T) {
    // Write the binary packet's data to the stream
    if (T.Size() > 0) {
        S.Write(T.Size(), T.Begin());
    }
    return S;
}

BinaryStream &operator<<(BinaryStream &S, const BinaryStream &T) {
    // Write the binary stream's data to the stream
    if (T.Size() > 0) {
        S.Write(T.Size(), T.Begin());
    }
    return S;
}

void BinaryStream::Register(Registry &registry) {
    ClassBuilder<BinaryStream>(registry,
                               Label(Type::Traits<BinaryStream>::Name()))
        .Methods("Size", &BinaryStream::Size)("Clear", &BinaryStream::Clear);
}

KAI_END
