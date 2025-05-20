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
    return len > 0 && current + len <= last;
}

void BinaryStream::Clear() {
    bytes.clear();
    first = current = last = 0;
}

BinaryStream &BinaryStream::Write(int len, const Byte *src) {
    if (len <= 0) {
        return *this;  // Nothing to write
    }

    // Calculate current state
    std::size_t cursor = current - first;
    std::size_t cur_size = last - first;

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
    // Read data from S into T
    // First read the size of data to transfer
    int size = S.Size() - (S.Current() - S.Begin());
    if (size <= 0) {
        return S;  // Nothing to read
    }

    // Create a new BinaryPacket with the read data
    // This assumes T is properly initialized and can store the data
    const BinaryPacket::Byte *data = S.Current();
    T = BinaryPacket(data, data + size, S.GetRegistry());

    // Advance S's current position
    for (int i = 0; i < size; ++i) {
        BinaryPacket::Byte dummy;
        S.Read(dummy);
    }

    return S;
}

BinaryPacket &operator>>(BinaryPacket &S, BinaryStream &T) {
    // Read data from S into T
    // First read the size of data to transfer
    int size = S.Size() - (S.Current() - S.Begin());
    if (size <= 0) {
        return S;  // Nothing to read
    }

    // Clear T and then write the data
    T.Clear();
    T.Write(size, S.Current());

    // Advance S's current position
    for (int i = 0; i < size; ++i) {
        BinaryPacket::Byte dummy;
        S.Read(dummy);
    }

    return S;
}

KAI_END

// EOF
