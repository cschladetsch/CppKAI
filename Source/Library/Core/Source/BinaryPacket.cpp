#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

void BinaryPacket::Register(Registry &registry, const char *name) {
    ClassBuilder<BinaryPacket>(registry, Label(name))
        .Methods("Size", &BinaryPacket::Size);
}

bool BinaryPacket::Read(int len, Byte *dest) {
    if (!CanRead(len)) return false;

    memcpy(dest, current, len);
    current += len;
    return true;
}

bool BinaryPacket::CanRead(int len) const {
    if (len <= 0) return false;
    return len <= (last - current);
}

void BinaryStream::Clear() {
    bytes.clear();
    first = current = last = 0;
}

BinaryStream &BinaryStream::Write(int len, const Byte *src) {
    if (len <= 0) {
        return *this;  // Nothing to write
    }

    // `first`, `current`, and `last` start as null on a fresh stream.
    // Avoid pointer arithmetic on null by treating that case as an empty buffer.
    std::size_t cursor = first ? static_cast<std::size_t>(current - first) : 0;
    std::size_t cur_size = first ? static_cast<std::size_t>(last - first) : 0;

    // Reserve space efficiently - avoid frequent reallocations
    // If we need to grow, use exponential growth strategy
    size_t new_size = bytes.size() + len;
    if (bytes.capacity() < new_size) {
        bytes.reserve(std::max(new_size, bytes.size() * 2));
    }

    // Resize the buffer to fit the new data
    bytes.resize(new_size);

    // Update pointers after resize
    first = bytes.data();
    current = first + cursor;
    last = first + new_size;

    // Copy the new data at the end of current data
    memcpy((void *)(first + cur_size), src, len);

    return *this;
}

BinaryPacket &operator>>(BinaryPacket &S, BinaryPacket &T) {
    int size = 0;
    if (!S.Read(size)) {
        return S;
    }

    if (size <= 0) {
        return S;
    }

    if (!S.CanRead(size)) {
        KAI_THROW_0(PacketExtraction);
    }

    const BinaryPacket::Byte *data = S.Current();
    T = BinaryPacket(data, data + size, S.GetRegistry());

    for (int i = 0; i < size; ++i) {
        BinaryPacket::Byte dummy;
        S.Read(dummy);
    }

    return S;
}

BinaryPacket &operator>>(BinaryPacket &S, BinaryStream &T) {
    int size = 0;
    if (!S.Read(size)) {
        return S;
    }

    if (size <= 0) {
        T.Clear();
        return S;
    }

    if (!S.CanRead(size)) {
        KAI_THROW_0(PacketExtraction);
    }

    T.Clear();
    T.Write(size, S.Current());

    for (int i = 0; i < size; ++i) {
        BinaryPacket::Byte dummy;
        S.Read(dummy);
    }

    return S;
}

KAI_END

// EOF
