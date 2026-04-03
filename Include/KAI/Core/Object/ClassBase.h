#pragma once

#include <KAI/Core/Base.h>
#include <KAI/Core/Exception/Extended.h>
#include <KAI/Core/Type/Number.h>
#include <KAI/Core/Type/Properties.h>

#include <list>
#include <unordered_map>

#include "KAI/Core/Exception/ExceptionMacros.h"
#include "KAI/Core/Object/Label.h"
#include "KAI/Core/Object/LabelHash.h"
#include "KAI/Core/Object/MethodBase.h"
#include "KAI/Core/Object/PropertyBase.h"
#include "KAI/Core/StringStream.h"

KAI_BEGIN

class MethodBase;
class PropertyBase;

/// Base for all Class<T> types. ClassBase defines the type-independent
/// interface that all Class<T>s must define
class ClassBase {
   public:
    typedef std::unordered_map<Label, MethodBase *, detail::LabelHash> Methods;
    typedef std::unordered_map<Label, PropertyBase *, detail::LabelHash>
        Properties;
    typedef std::list<Object> ObjectList;

   protected:
    Label name_;
    Methods methods_;
    Properties properties_;
    Type::Number type_number_;

   public:
    ClassBase(Label const &name, Type::Number T)
        : name_(name), type_number_(T) {}
    virtual ~ClassBase();

    const Label &GetName() const { return name_; }
    const Label &GetLabel() const { return GetName(); }
    Type::Number GetTypeNumber() const { return type_number_; }

    virtual void SetReferencedObjectsColor(StorageBase &base,
                                           ObjectColor::Color color,
                                           HandleSet &handles) const;

    void GetPropertyObjects(StorageBase &object, ObjectList &contained) const;

    /// methods_
    void AddMethod(const Label &L, MethodBase *M) { methods_[L] = M; }
    const Methods &GetMethods() const { return methods_; }
    MethodBase *GetMethod(const Label &L) const {
        const auto found = methods_.find(L);
        return found == methods_.end() ? nullptr : found->second;
    }

    /// properties_
    void AddProperty(Label const &label, PropertyBase *property) {
        properties_[label] = property;
    }
    bool HasProperty(Label const &label) const {
        return properties_.find(label) != properties_.end();
    }

    Properties const &GetProperties() const { return properties_; }

    PropertyBase const &GetProperty(Label const &L) const {
        auto found = properties_.find(L);
        if (found == properties_.end())
            KAI_THROW_2(UnknownProperty, GetName(), L);
        return *found->second;
    }

    bool HasOperation(int N) const { return HasTraitsProperty(N); }

    virtual void MakeReachableGrey(StorageBase &base) const = 0;
    virtual bool CanBlackenReferencedObjects(StorageBase &base) const {
        return true;
    }
    virtual void GetContainedObjects(StorageBase &object,
                                     ObjectList &contained) const = 0;
    virtual void CreateProperties(StorageBase &object) const = 0;
    virtual Object Duplicate(StorageBase const &) const = 0;
    virtual void DetachFromContainer(StorageBase &, Object const &) const = 0;
    virtual int GetTraitsProperties() const = 0;
    virtual bool HasTraitsProperty(int N) const = 0;
    virtual StorageBase *NewStorage(Registry *, Handle) const = 0;
    virtual void Create(StorageBase &) const = 0;
    virtual bool Destroy(StorageBase &) const = 0;
    virtual void Delete(StorageBase &) const = 0;
    virtual void Assign(StorageBase &, StorageBase const &) const = 0;
    virtual void SetSwitch(StorageBase &Q, int S, bool M) const = 0;
    void SetMarked(StorageBase &Q, bool M) const;
    virtual void SetMarked2(StorageBase &Q, bool M) const = 0;
    virtual Object UpCast(StorageBase &) const = 0;
    virtual Object CrossCast(StorageBase &, Type::Number) const = 0;
    virtual Object DownCast(StorageBase &, Type::Number) const = 0;
    virtual HashValue GetHashValue(const StorageBase &) const = 0;
    virtual Object Absolute(const StorageBase &) const = 0;
    virtual bool Less(const StorageBase &, const StorageBase &) const = 0;
    virtual bool Greater(const StorageBase &, const StorageBase &) const = 0;
    virtual bool Equiv(const StorageBase &, const StorageBase &) const = 0;
    virtual bool Boolean(const StorageBase &) const = 0;
    virtual void Insert(StringStream &, const StorageBase &) const = 0;
    virtual StorageBase *Extract(Registry &, StringStream &) const = 0;
    virtual void ExtractValue(Object &object,
                              StringStream &strstream) const = 0;
    virtual void Insert(BinaryStream &, const StorageBase &) const = 0;
    virtual StorageBase *Extract(Registry &, BinaryStream &) const = 0;
    virtual StorageBase *Plus(StorageBase const &,
                              StorageBase const &) const = 0;
    virtual StorageBase *Minus(StorageBase const &,
                               StorageBase const &) const = 0;
    virtual StorageBase *Multiply(StorageBase const &,
                                  StorageBase const &) const = 0;
    virtual StorageBase *Divide(StorageBase const &,
                                StorageBase const &) const = 0;

    Object Absolute(Object const &object) const {
        return Absolute(object.GetStorageBase());
    }

    bool Less(Object const &lhs, Object const &rhs) const {
        return Less(lhs.GetStorageBase(), rhs.GetStorageBase());
    }

    bool Equiv(Object const &lhs, Object const &rhs) const {
        return Equiv(lhs.GetStorageBase(), rhs.GetStorageBase());
    }

    bool Greater(Object const &lhs, Object const &rhs) const {
        return Greater(lhs.GetStorageBase(), rhs.GetStorageBase());
    }

    bool Boolean(Object const &Q) const { return Boolean(Q.GetStorageBase()); }

    void Assign(Object A, Object B) const {
        Assign(A.GetStorageBase(), B.GetStorageBase());
    }
    Object Plus(Object A, Object B) const {
        return *Plus(A.GetStorageBase(), B.GetStorageBase());
    }
    Object Minus(Object A, Object B) const {
        return *Minus(A.GetStorageBase(), B.GetStorageBase());
    }
    Object Multiply(Object A, Object B) const {
        return *Multiply(A.GetStorageBase(), B.GetStorageBase());
    }
    Object Divide(Object A, Object B) const {
        return *Divide(A.GetStorageBase(), B.GetStorageBase());
    }

    bool Equiv2(const Object &A, const Object &B) const {
        return Equiv(A.GetStorageBase(), B.GetStorageBase());
    }
    bool Less2(const Object &A, const Object &B) const {
        return Less(A.GetStorageBase(), B.GetStorageBase());
    }
    bool Greater2(const Object &A, const Object &B) const {
        return Greater(A.GetStorageBase(), B.GetStorageBase());
    }
};

StringStream &operator<<(StringStream &, const ClassBase *);

KAI_TYPE_TRAITS_NAMED(const ClassBase *, Number::Class, "Class",
                      Properties::StringStreamInsert)

KAI_END
