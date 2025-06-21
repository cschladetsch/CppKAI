#pragma once

#include "KAI/Core/Object/Object.h"
#include "KAI/Core/Pathname.h"

KAI_BEGIN

namespace Exception {
struct CannotResolve : Base {
    Object object;
    Label label;
    Pathname path;
    CannotResolve(const FileLocation &L, Object const &Q)
        : Base(L, "CannotResolve"), object(Q) {}
    CannotResolve(const FileLocation &L, Label const &Q)
        : Base(L, "CannotResolve"), label(Q) {}
    CannotResolve(const FileLocation &L, Pathname const &Q)
        : Base(L, "CannotResolve"), path(Q) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct InvalidPathname : Base {
    String text;
    InvalidPathname(const FileLocation &L, const String &T)
        : Base(L, "InvalidPathname"), text(T) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct InvalidIdentifier : Base {
    Object what;
    InvalidIdentifier(const FileLocation &L, const Object &T)
        : Base(L, "InvalidIdentifier"), what(T) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct NotImplemented : Base {
    String text;
    NotImplemented(const FileLocation &L, const char *T = "<unnamed>")
        : Base(L, "Not Implemented"), text(T) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct ObjectNotInTree : Base {
    Object object;
    ObjectNotInTree(const FileLocation &L, Object const &Q)
        : Base(L, "Object not in tree"), object(Q) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct InternalError : Base {
    InternalError(const FileLocation &L, const char *T) : Base(L, T) {}
};

struct UnknownMethod : Base {
    String name_;
    String class_name_;
    UnknownMethod(const FileLocation &L, const String &N, const String &B)
        : Base(L, "Unknown Method"), name_(N), class_name_(B) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct UnknownHandle : Base {
    Handle handle;
    UnknownHandle(const FileLocation &L, Handle N)
        : Base(L, "Unknown Handle"), handle(N) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct InvalidStringLiteral : Base {
    String text;
    InvalidStringLiteral(const FileLocation &L, const String &T)
        : Base(L, "Invalid string literal"), text(T) {}
    void WriteExtendedInformation(StringStream &) const;
};

template <class T = meta::Null>
struct UnknownClass : Base {
    String name_;
    Type::Number type_number;
    UnknownClass(const FileLocation &L, const String &N)
        : Base(L, "Unknown Class"), name_(N) {}
    UnknownClass(const FileLocation &L, Type::Number N)
        : Base(L, "Unknown Class"), type_number(N) {}
    UnknownClass(const FileLocation &L)
        : Base(L, "Unknown Class"),
          type_number(Type::Traits<T>::Number),
          name_("TODO boost::typeindex") {}
    void WriteExtendedInformation(StringStream &S) const {
        S << "name_=" << name_ << ", type_number_=" << type_number.value;
    }
};

struct CannotNew : Base {
    Object arg;
    CannotNew(const FileLocation &L, const Object &Q)
        : Base(L, "Cannot new"), arg(Q) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct AssertionFailed : Base {
    AssertionFailed(const FileLocation &L) : Base(L, "AssertionFailed") {}
};

struct UnknownKey : Base {
    Object key;
    UnknownKey(const FileLocation &L, Object K)
        : Base(L, "Unknown key in Map"), key(K) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct BadIndex : Base {
    int index;
    BadIndex(const FileLocation &L, int N) : Base(L, "Bad Index"), index(N) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct BadUpCast : Base {
    BadUpCast(const FileLocation &L) : Base(L, "BadUpCast") {}
};

struct FileNotFound : Base {
    String filename;
    FileNotFound(const FileLocation &L, String const &F)
        : Base(L, "FileNotFound"), filename(F) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct UnknownProperty : Base {
    Label klass;
    Label prop;
    UnknownProperty(const FileLocation &L, Label const &K, Label const &P)
        : Base(L, "UnknownProperty"), klass(K), prop(P) {}
    void WriteExtendedInformation(StringStream &) const;
};

struct NoInput : Base {
    NoInput(const FileLocation &L) : Base(L, "UnknownProperty") {}
};

struct DivideByZero : Base {
    DivideByZero(const FileLocation &L) : Base(L, "DivideByZero") {}
};

}  // namespace Exception

KAI_END
