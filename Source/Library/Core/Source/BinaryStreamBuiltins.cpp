#include <KAI/Core/BuiltinTypes.h>

KAI_BEGIN

BinaryStream &operator<<(BinaryStream &S, double N) { return S.Write(N); }

BinaryStream &operator<<(BinaryStream &S, float N) { return S.Write(N); }

BinaryStream &operator<<(BinaryStream &S, bool N) { return S.Write(N); }

BinaryStream &operator<<(BinaryStream &S, int N) { return S.Write(N); }

BinaryStream &operator>>(BinaryStream &S, int &N) {
    if (!S.Read(N)) KAI_THROW_0(PacketExtraction);

    return S;
}

BinaryStream &operator>>(BinaryStream &S, double &N) {
    if (!S.Read(N)) KAI_THROW_0(PacketExtraction);

    return S;
}

BinaryStream &operator>>(BinaryStream &S, float &N) {
    if (!S.Read(N)) KAI_THROW_0(PacketExtraction);

    return S;
}

BinaryStream &operator>>(BinaryStream &S, bool &N) {
    if (!S.Read(N)) KAI_THROW_0(PacketExtraction);

    return S;
}

KAI_END
