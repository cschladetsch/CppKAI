#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Type/Traits.h>

#include <list>

#include "KAI/Core/Object/Label.h"

KAI_BEGIN

/// A Pathname represents a qualified name_ for an Object
class Pathname {
   public:
    struct Literals {
        static const String::Char Parent;
        static const String::Char This;
        static const String::Char Separator;
        static const String::Char Quote;
        static const String::Char All[];
        static const String::Char AllButQuote[];
    };

    struct Element {
        enum Type { None, Quote, Separator, Parent, This, Name };
        Type type;
        Label name_;
        Element(Type T = None) : type(T) {}
        Element(const Label &L) : type(Name), name_(L) {}
        friend bool operator<(const Element &A, const Element &B) {
            return A.type < B.type || (A.type == B.type && A.name_ < B.name_);
        }
        friend bool operator==(const Element &A, const Element &B) {
            return A.type == B.type && A.name_ == B.name_;
        }
    };
    typedef std::vector<Element> Elements;

   private:
    Elements elements;

   public:
    Pathname() {}
    Pathname(const String &);
    Pathname(const Elements &);

    bool Quoted() const;
    bool Absolute() const;

    Elements GetElements() const { return elements; }

    void FromString(const String &);
    void FromString2(String);
    String ToString() const;

    bool Empty() const;

    Elements::const_iterator begin() const { return elements.begin(); }
    Elements::const_iterator end() const { return elements.end(); }

    friend bool operator<(const Pathname &A, const Pathname &B);
    friend bool operator==(const Pathname &A, const Pathname &B);
    friend Pathname operator+(const Pathname &A, const Pathname &B);

    static void Register(Registry &);

    bool Validate() const;
    void AddElement(StringStream &, Element::Type);
};

StringStream &operator<<(StringStream &, Pathname const &);
StringStream &operator>>(StringStream &, Pathname &);
BinaryStream &operator<<(BinaryStream &, Pathname const &);
BinaryPacket &operator>>(BinaryPacket &, Pathname &);

template <class T>
bool operator>(T const &A, T const &B) {
    return B < A;
}

KAI_TYPE_TRAITS(Pathname, Number::Pathname,
                Properties::Streaming | Properties::Relational |
                    Properties::Plus);

Pathname GetFullname(const StorageBase &);
Pathname GetFullname(const Object &);
Label GetName(const Object &);

void Set(Object scope, const Pathname &path, StorageBase *);
void Set(Object scope, const Pathname &path, Object const &Q);
void Set(Object const &root, Object const &scope, const Pathname &path,
         Object const &Q);
void Set(Object const &root, Object const &scope, Object const &ident,
         Object const &Q);

Object Get(Object scope, const Pathname &path);
Object Get(Object const &root, Object const &scope, const Pathname &path);
Object Get(Object const &root, Object const &scope, Object const &path);

void Remove(Object scope, const Pathname &path);
void Remove(Object const &root, Object const &scope, const Pathname &path);
void Remove(Object const &root, Object const &scope, Object const &ident);

bool Exists(Object const &root, Object const &scope, const Pathname &path);
bool Exists(Object const &scope, const Pathname &path);

KAI_END
