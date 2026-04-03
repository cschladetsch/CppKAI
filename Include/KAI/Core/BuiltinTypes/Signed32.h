#pragma once

#include <KAI/Core/Type.h>

KAI_BEGIN

StringStream &operator<<(StringStream &S, int N);
StringStream &operator>>(StringStream &S, int &N);
BinaryStream &operator<<(BinaryStream &S, int N);
BinaryStream &operator>>(BinaryStream &S, int &N);

// inline HashValue GetHash(int N) { return N; }

KAI_TYPE_TRAITS(int, Number::Signed32,
                Properties::Arithmetic | Properties::Multiplicative |
                    Properties::Streaming | Properties::Assign |
                    Properties::Relational | Properties::Absolute |
                    Properties::Boolean);

// KAI_TYPE_TRAITS(String, Number::String
//     , Properties::Arithmetic
//     | Properties::Streaming
//     | Properties::Assign
//     | Properties::Relational
//     );
KAI_END
