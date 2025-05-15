#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Traits.h>

KAI_BEGIN

StringStream &operator<<(StringStream &, float);
StringStream &operator>>(StringStream &, float &);
BinaryStream &operator<<(BinaryStream &, float);
BinaryStream &operator>>(BinaryStream &, float &);

// Use the macro for float type to ensure consistency with int
KAI_TYPE_TRAITS(float, Number::Single,
                Properties::Arithmetic | Properties::Multiplicative |
                    Properties::Streaming | Properties::Assign |
                    Properties::Relational | Properties::Absolute |
                    Properties::Boolean);

StringStream &operator<<(StringStream &, double);
StringStream &operator>>(StringStream &, double &);
BinaryStream &operator<<(BinaryStream &, double);
BinaryStream &operator>>(BinaryStream &, double &);

// Use the macro for double type to ensure consistency
KAI_TYPE_TRAITS(double, Number::Double,
                Properties::Arithmetic | Properties::Multiplicative |
                    Properties::Streaming | Properties::Assign |
                    Properties::Relational | Properties::Absolute |
                    Properties::Boolean);

KAI_END
