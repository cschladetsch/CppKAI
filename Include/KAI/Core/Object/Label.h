#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Type/TraitMacros.h>

KAI_BEGIN

struct Label {
    typedef String Value;

   private:
    Value _value;
    bool _quoted = false;

   public:
    Label() = default;
    explicit Label(const String::Char *S) { FromString(S); }
    explicit Label(const Value &S) { FromString(S); }

    bool Empty() const { return _value.empty(); }
    bool Quoted() const { return _quoted; }
    void SetQuoted(bool q) { _quoted = q; }

    void FromString(const Value &S);
    String ToString() const;

    void FromString2(Value S);
    const Value &GetValue() const { return _value; }

    friend bool operator<(const Label &A, const Label &B) {
        return A.ToString() < B.ToString();
    }

    friend bool operator==(const Label &A, const Label &B) {
        return A.ToString() == B.ToString();
    }

    static void Register(Registry &);
};

StringStream &operator<<(StringStream &S, const Label &L);
StringStream &operator>>(StringStream &S, Label &L);
BinaryStream &operator<<(BinaryStream &, Label const &);
BinaryStream &operator>>(BinaryStream &, Label &);
std::ostream &operator<<(std::ostream &, const Label &);

KAI_TYPE_TRAITS(Label, Number::Label,
                Properties::Streaming | Properties::Relational);

KAI_END

//#include "KAI/Core/LabelHash.h"

namespace boost {
inline size_t hash_value(KAI_NAMESPACE(Label) const &label) {
    return hash_value(label.GetValue());
}
}  // namespace boost
