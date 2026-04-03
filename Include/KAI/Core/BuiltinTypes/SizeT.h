#pragma once

#include <KAI/Core/Type.h>

KAI_BEGIN

// Make sure size_t is handled properly
StringStream &operator<<(StringStream &S, size_t N);
StringStream &operator>>(StringStream &S, size_t &N);
BinaryStream &operator<<(BinaryStream &S, size_t N);
BinaryStream &operator>>(BinaryStream &S, size_t &N);

// Use Signed32 type since there's no specific unsigned type available
KAI_TYPE_TRAITS(size_t, Number::Signed32,
                Properties::Arithmetic | Properties::Multiplicative |
                    Properties::Streaming | Properties::Assign |
                    Properties::Relational | Properties::Absolute |
                    Properties::Boolean);

KAI_END