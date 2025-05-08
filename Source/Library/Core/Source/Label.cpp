#include "KAI/Core/BuiltinTypes/All.h"

KAI_BEGIN

String Label::ToString() const {
    if (!_quoted) return _value;

    return String("'") + _value;
}

void Label::FromString2(String text) { FromString(text); }

void Label::FromString(const Value &S) {
    _quoted = false;
    _value = "";
    if (S.empty()) return;

    const String::Char *str = S.c_str();
    if (str[0] == '\'') {
        _quoted = true;
        str++;
    }

    _value = str;
}

StringStream &operator<<(StringStream &S, const Label &L) {
    return S << L.ToString();
}

StringStream &operator>>(StringStream &S, Label &L) {
    bool quoted = S.Peek() == '\'';
    if (quoted) {
        char ch;
        S.Extract(ch);
    }

    String val;
    S >> val;
    L.FromString(val);
    L.SetQuoted(quoted);
    return S;
}

BinaryStream &operator<<(BinaryStream &S, Label const &L) {
    return S << L.ToString();
}

BinaryStream &operator>>(BinaryStream &S, Label &L) {
    String value;
    S >> value;
    L.FromString(value);
    return S;
}

std::ostream &operator<<(std::ostream &S, const Label &L) {
    return S << L.ToString();
}

void Label::Register(Registry &R) {
    ClassBuilder<Label>(R, Label("Label"))
        .Methods("ToString", &Label::ToString, "ToString")(
            "FromString", &Label::FromString2, "FromString");
}

KAI_END
