#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Number.h>

KAI_BEGIN

class SystemType {
    Type::Number type_number;
    Label label;

   public:
    void Set(Type::Number N, const Label &L) {
        type_number = N;
        label = L;
    }

    Type::Number GetTypeNumber() const { return type_number; }
    const Label &GetLabel() const { return label; }

    friend bool operator<(const SystemType &A, const SystemType &B) {
        return A.type_number < B.type_number;
    }
    friend bool operator==(const SystemType &A, const SystemType &B) {
        return A.type_number == B.type_number;
    }
};

StringStream &operator<<(StringStream &, SystemType const &);
BinaryStream &operator<<(BinaryStream &, SystemType const &);
BinaryPacket &operator>>(BinaryPacket &, SystemType &);

KAI_TYPE_TRAITS_STRING_STREAM_INSERT(SystemType, Type::Number::Type);

KAI_END

#endif  // KAI_BUILTIN_TYPES_SYSTEM_TYPE_H

// EOF
