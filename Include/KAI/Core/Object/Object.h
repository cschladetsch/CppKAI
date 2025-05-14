#pragma once

#include <KAI/Core/BuiltinTypes/Dictionary.h>
#include <KAI/Core/Type.h>

#include <list>

#include "KAI/Core/Object/Constness.h"
#include "KAI/Core/Object/Handle.h"
#include "KAI/Core/Object/ObjectConstructParams.h"
#include "KAI/Core/ObjectColor.h"

KAI_BEGIN

class Object {
    const ClassBase *class_base{nullptr};
    Registry *registry{nullptr};
    Handle handle;

#ifdef KAI_CACHE_OBJECT_LOOKUPS
    // these fields are used to cache results for speed
    int gcIndex_{0};
    bool valid_{false};
    void *value{nullptr};
#endif

   public:
    enum Switch {
        Marked = 1,
        Managed = 2,
        Const = 4,
        Clean = 8,
        // if set, when marking, will not mark children
        NoRecurse = 16,
        DefaultSwitches = Managed
    };

    Object() = default;
    Object(Object const &);
    explicit Object(ObjectConstructParams const &P);
    Object &operator=(Object const &);

    template <class T>
    bool IsType() const {
        return Exists() && GetTypeNumber() == Type::Traits<T>::Number;
    }

    StorageBase &GetStorageBase() const;
    int GetSwitches() const;
    ObjectColor::Color GetColor() const;
    void SetColor(ObjectColor::Color C) const;
    void SetColorRecursive(ObjectColor::Color C) const;
    void SetColorRecursive(ObjectColor::Color C, HandleSet &) const;
    bool IsWhite() const { return GetColor() == ObjectColor::White; }
    bool IsGrey() const { return GetColor() == ObjectColor::Grey; }
    bool IsBlack() const { return GetColor() == ObjectColor::Black; }
    void SetWhite() const { SetColor(ObjectColor::White); }
    void SetGrey() const { SetColor(ObjectColor::Grey); }
    void SetBlack() const { SetColor(ObjectColor::Black); }
    Object GetPropertyValue(Label const &L) const;
    Type::Number GetTypeNumber() const;
    const ClassBase *GetClass() const { return class_base; }
    Registry *GetRegistry() const { return registry; }
    Object Duplicate() const;
    Object Clone() const { return Duplicate(); }
    Handle GetParentHandle() const;
    void SetParentHandle(Handle);
    Handle GetHandle() const { return handle; }
    Object GetParent() const;
    void Delete() const;
    bool Valid() const;
    bool Exists() const;
    bool OnDeathRow() const;
    bool IsConst() const;
    bool IsManaged() const;
    bool IsMarked() const;
    bool IsClean() const;
    void SetSwitch(int, bool) const;
    void SetSwitches(int) const;
    void SetMarked(bool = true) const;
    void SetConst() const;
    void SetManaged(bool = true) const;
    void SetClean(bool = true) const;
    bool IsMutable() const { return !IsConst(); }
    bool IsUnmanaged() const { return !IsManaged(); }
    bool IsUnmarked() const { return !IsMarked(); }
    bool IsDirty() const { return !IsClean(); }
    void Set(const char *label, const Object &Q) const { Set(Label(label), Q); }
    Object Get(const char *label) const { return Get(Label(label)); }
    void Add(const Label &label, const Object &child) const {
        Set(label, child);
    }
    void Set(const Label &, const Object &) const;
    Object Get(const Label &) const;
    bool Has(const Label &) const;
    void Remove(const Label &) const;
    void Detach(const Label &L) const { Remove(L); }
    void Detach(const Object &Q) const;
    Dictionary const &GetDictionary() const;
    void SetChild(const Label &L, const Object &Q) const { Set(L, Q); }
    Object GetChild(const Label &L) const { return Get(L); }
    void RemoveChild(const Label &L) const { Remove(L); }
    void DetachChild(const Label &L) const { Remove(L); }
    void DetachChild(const Object &Q) const { Detach(Q); }
    bool HasChild(const Label &L) const { return Has(L); }
    Label GetLabel() const;
    String ToString() const;
    String ToXmlString() const;
    Object NewFromTypeNumber(Type::Number N) const;
    void Assign(StorageBase &, StorageBase const &);
    StorageBase *GetStorageBase(Handle other) const;
    void SetPropertyValue(Label const &, Object const &) const;
    void SetPropertyObject(Label const &, Object const &) const;
    Object GetPropertyObject(Label const &) const;
    bool HasProperty(Label const &name) const;
    static void Register(Registry &);
    void RemovedFromContainer(Object container) const;
    void AddedToContainer(Object container) const;
    StorageBase *GetBasePtr() const;
    StorageBase *GetParentBasePtr() const;
    using ObjectList = std::list<Object>;
    void GetPropertyObjects(ObjectList &contained) const;
    void GetContainedObjects(ObjectList &contained) const;
    void GetChildObjects(ObjectList &contained) const;
    void GetAllReferencedObjects(ObjectList &contained) const;

    bool IsTypeNumber(int typeNumber) const {
        if (!Exists()) return typeNumber == Type::Number::None;

        return GetTypeNumber() == typeNumber;
    }

    class ChildProxy {
        friend class Object;
        Registry *registry;
        Handle handle;
        Label label;
        Constness konst;
        ChildProxy(Object const &Q, const char *);
        ChildProxy(Object const &Q, Label const &L);
        Object GetObject() const;

       public:
        template <class T>
        ChildProxy &operator=(T const &value) {
            // GetObject().Set(
            throw;
            // return *this;
        }

        template <class T>
        ChildProxy &operator=(Pointer<T> const &value) {
            GetObject().Set(label, value);
            return *this;
        }
        ChildProxy &operator=(Object const &child) {
            GetObject().Set(label, child);
            return *this;
        }
        operator Object() const { return GetObject().Get(label); }
    };

    ChildProxy operator[](const char *label) const {
        return ChildProxy(*this, label);
    }

   protected:
    Dictionary &GetDictionaryRef();
};

StringStream &operator<<(StringStream &S, const Object &Q);
StringStream &operator>>(StringStream &S, Object &Q);
BinaryStream &operator<<(BinaryStream &S, const Object &Q);
BinaryStream &operator>>(BinaryStream &stream, Object &Q);

bool operator<(Object const &A, Object const &B);
bool operator==(Object const &A, Object const &B);
inline bool operator!=(Object const &A, Object const &B) { return !(A == B); }
bool operator>(const Object &A, Object const &B);

Object operator+(Object const &A, Object const &B);
// WTF Object operator-(Object const &Object Absolute(Object const &A);

KAI_TYPE_TRAITS(Object, Number::Object,
                Properties::StringStreamInsert |
                    Properties::BinaryStreamInsert |
                    Properties::BinaryStreamExtract);

HashValue GetHash(Object const &);

void MarkObject(Object const &, bool = true);
void MarkObjectAndChildren(Object const &, bool = true);
void MarkObject(StorageBase &, bool = true);
void MarkObjectAndChildren(StorageBase &, bool = true);
Object Duplicate(Object const &);

KAI_END

namespace boost {
inline size_t hash_value(KAI_NAMESPACE(Object) const &H) {
    return H.GetHandle().GetValue();
}
}  // namespace boost
