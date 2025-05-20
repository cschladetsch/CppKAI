#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Traits.h>

KAI_BEGIN

StringStream &operator<<(StringStream &, float);
StringStream &operator>>(StringStream &, float &);
BinaryStream &operator<<(BinaryStream &, float);
BinaryStream &operator>>(BinaryStream &, float &);

namespace Type {
// Custom traits for float to avoid macro issues
template <>
struct Traits<float>
    : TraitsBase<float, Number::Single,
                 Properties::Arithmetic | Properties::Multiplicative |
                     Properties::Streaming | Properties::Assign |
                     Properties::Relational | Properties::Absolute |
                     Properties::Boolean> {
    typedef TraitsBase<float, Number::Single,
                       Properties::Arithmetic | Properties::Multiplicative |
                           Properties::Streaming | Properties::Assign |
                           Properties::Relational | Properties::Absolute |
                           Properties::Boolean>
        Tr;
    using typename Tr::ConstReference;
    using typename Tr::Reference;
    using typename Tr::Store;
    static std::string Name() { return Tr::Name(); }
};
}  // namespace Type

StringStream &operator<<(StringStream &, double);
StringStream &operator>>(StringStream &, double &);
BinaryStream &operator<<(BinaryStream &, double);
BinaryStream &operator>>(BinaryStream &, double &);

namespace Type {
// Custom traits for double to avoid macro issues
template <>
struct Traits<double>
    : TraitsBase<double, Number::Double,
                 Properties::Arithmetic | Properties::Multiplicative |
                     Properties::Streaming | Properties::Assign |
                     Properties::Relational | Properties::Absolute |
                     Properties::Boolean> {
    typedef TraitsBase<double, Number::Double,
                       Properties::Arithmetic | Properties::Multiplicative |
                           Properties::Streaming | Properties::Assign |
                           Properties::Relational | Properties::Absolute |
                           Properties::Boolean>
        Tr;
    using typename Tr::ConstReference;
    using typename Tr::Reference;
    using typename Tr::Store;
    static std::string Name() { return Tr::Name(); }
};
}  // namespace Type

KAI_END
