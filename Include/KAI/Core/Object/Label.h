#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Type/TraitMacros.h>

KAI_BEGIN

struct Label {
    typedef String Value;

   private:
    Value value_;
    bool quoted_ = false;

   public:
    Label() = default;
    explicit Label(const String::Char *S) { FromString(S); }
    explicit Label(const Value &S) { FromString(S); }

    bool Empty() const { return value_.empty(); }
    bool Quoted() const { return quoted_; }
    void SetQuoted(bool q) { quoted_ = q; }

    void FromString(const Value &S);
    String ToString() const;

    void FromString2(Value S);
    const Value &GetValue() const { return value_; }

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

// #include "KAI/Core/LabelHash.h"

namespace boost {
inline size_t hash_value(KAI_NAMESPACE(Label) const &label) {
    return hash_value(label.GetValue());
}
}  // namespace boost
