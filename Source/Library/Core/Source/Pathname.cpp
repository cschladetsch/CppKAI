#include <ctype.h>

#include "KAI/Core/BuiltinTypes.h"

// NOTE: This should all be moved to the Lexer/Parser logic, not
// logic inherent in the Pathname class itself.
//
// Rather, the Lexer/Parser should generate Pathnames.
//
// The main point of contention is the ability to make Pathnames (and
// for that matter, Labels), "On the fly" in code without a lexer or parser.
//
// Note that Pi and Rho share the exact same syntax and semantics for Pathnames
// and Labels.

KAI_BEGIN

const String::Char Pathname::Literals::Parent = '^';
const String::Char Pathname::Literals::This = '.';
const String::Char Pathname::Literals::Separator = '/';
const String::Char Pathname::Literals::Quote = '\'';
const String::Char Pathname::Literals::All[] = {Parent, This, Separator, Quote,
                                                0};
const String::Char Pathname::Literals::AllButQuote[] = {Parent, This, Separator,
                                                        0};

Pathname::Pathname(const Elements &e) : elements(e) {}

Pathname::Pathname(const String &text) { FromString(text); }

bool Pathname::Quoted() const {
    return !elements.empty() && elements.front().type == Element::Quote;
}

bool Pathname::Absolute() const {
    if (elements.empty()) return false;

    if (Quoted())
        return elements.size() > 1 && elements[1].type == Element::Separator;

    return elements.front().type == Element::Separator;
}

void Pathname::FromString2(String text) { FromString(text); }

// TODO: Pathnames and id's have been giving me grief for years.
// Need to sort it out once and for all.
// Need to use a static PiParser method or something. Doing it badly
// in three different places and across 3 different languages is insane.
void Pathname::FromString(const String &text) {
    elements.clear();
    if (text.empty()) return;

    const String::Char *S = text.c_str();

    StringStream name;
    for (; *S; ++S) {
        switch (*S) {
            case Literals::Quote:
                elements.push_back(Element::Quote);
                break;

            case Literals::Parent:
                AddElement(name, Element::Parent);
                break;

            case Literals::Separator:
                if (S[1] != 0) AddElement(name, Element::Separator);
                break;

            case Literals::This:
                AddElement(name, Element::This);
                break;

            default:
                if (!isalnum(*S) && *S != '_') {
                    elements.clear();
                    KAI_THROW_1(InvalidPathname, text);
                }
                name.Append(*S);
                break;
        }
    }

    name << Ends;
    String s = name.ToString();
    if (!s.Empty()) elements.push_back(Element(Label(s)));

    if (elements.empty()) return;

    if (elements.back().type == Element::Separator) elements.pop_back();

    if (!Validate()) {
        elements.clear();
        Validate();
        KAI_THROW_1(InvalidPathname, text);
    }
}

void Pathname::AddElement(StringStream &name, Element::Type type) {
    if (!name.Empty()) {
        name << Ends;
        elements.push_back(Element(Label(name.ToString())));
    }

    elements.push_back(type);
    name.Clear();
}

String Pathname::ToString() const {
    StringStream str;
    bool addedRoot = false;
    if (Absolute()) {
        addedRoot = true;
        str.Append(Literals::Separator);
    }

    for (auto element : elements) {
        switch (element.type) {
            case Element::Quote:
                str.Append(Literals::Quote);
                break;

            case Element::Separator:
                if (!addedRoot) str.Append(Literals::Separator);
                addedRoot = false;
                break;

            case Element::Parent:
                str.Append(Literals::Parent);
                break;

            case Element::This:
                str.Append(Literals::This);
                break;

            case Element::Name:
                str << element.name_.ToString();
                break;

            case Element::None:
                break;
        }
    }

    str << Ends;
    return str.ToString();
}

bool Pathname::Empty() const { return elements.empty(); }

bool Pathname::Validate() const {
    if (elements.empty()) return true;
    // TODO
    return true;
}

StringStream &operator<<(StringStream &S, Pathname const &P) {
    return S << P.ToString();
}

bool operator<(const Pathname &A, const Pathname &B) {
    return A.elements < B.elements;
}

bool operator==(const Pathname &A, const Pathname &B) {
    return A.elements == B.elements;
}

BinaryPacket &operator>>(BinaryPacket &, Pathname &) { KAI_NOT_IMPLEMENTED(); }

BinaryStream &operator<<(BinaryStream &, const Pathname &) {
    KAI_NOT_IMPLEMENTED();
}

StringStream &operator>>(StringStream &, Pathname &) { KAI_NOT_IMPLEMENTED(); }

// Plus operation for Pathname - creates a combined pathname
Pathname operator+(const Pathname &A, const Pathname &B) {
    // Only allow addition if BOTH pathnames are quoted
    // Otherwise it's a type error (unquoted pathnames should resolve to values
    // first)
    if (!A.Quoted() || !B.Quoted()) {
        KAI_THROW_1(Base, "Cannot add pathnames unless both are quoted");
    }

    // Create a quoted pathname combining both
    // Get the path elements without the quote
    Pathname::Elements elemsA = A.GetElements();
    Pathname::Elements elemsB = B.GetElements();

    // Build new elements starting with quote
    Pathname::Elements newElems;
    newElems.push_back(Pathname::Element(Pathname::Element::Quote));

    // Add elements from A (skip quote if present)
    for (auto it = elemsA.begin(); it != elemsA.end(); ++it) {
        if (it == elemsA.begin() && it->type == Pathname::Element::Quote) {
            continue;  // Skip the quote
        }
        newElems.push_back(*it);
    }

    // Add separator if needed
    if (!newElems.empty() && newElems.back().type == Pathname::Element::Name) {
        newElems.push_back(Pathname::Element(Pathname::Element::Separator));
    }

    // Add elements from B (skip quote if present)
    for (auto it = elemsB.begin(); it != elemsB.end(); ++it) {
        if (it == elemsB.begin() && it->type == Pathname::Element::Quote) {
            continue;  // Skip the quote
        }
        newElems.push_back(*it);
    }

    return Pathname(newElems);
}

void Pathname::Register(Registry &R) {
    ClassBuilder<Pathname>(R, Label("Pathname"))
        .Methods("Empty", &Pathname::Empty)("ToString", &Pathname::ToString)(
            "FromString", &Pathname::FromString2)(
            "absolute", &Pathname::Absolute)("quoted", &Pathname::Quoted);
}

KAI_END

// EOF
