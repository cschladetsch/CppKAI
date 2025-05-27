#pragma once

#include <KAI/Core/Registry.h>
#include <KAI/Core/Type/Deref.h>
#include <KAI/Core/Type/Traits.h>

#include "KAI/Core/Object/ClassBase.h"
#include "KAI/Core/Object/GetStorageBase.h"
#include "KAI/Core/Object/Label.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Core/Object/ObjectConstructParams.h"
#include "KAI/Core/TriColor.h"
#include "KAI/Core/Value.h"

#undef RegisterClass

KAI_BEGIN

#pragma warning(push)

// warning C4702: unreachable code
#pragma warning(disable : 4702)

void MarkGrey(Object const &);

template <class T>
class Class : public ClassBase {
   public:
    typedef typename Type::Traits<T> Traits;
    enum { Props = Traits::Props };

    Class(Label const &name) : ClassBase(name, Type::Traits<T>::Number) {}

    int GetTraitsProperties() const { return Props; }

    bool HasTraitsProperty(int N) const { return (Props & N) != 0; }

    // Lifetime management
    StorageBase *NewStorage(Registry *registry, Handle handle) const {
        auto result = registry->GetMemorySystem().Allocate<Storage<T> >(
            ObjectConstructParams(registry, this, handle));

        if (!result.has_value()) return nullptr;

        Storage<T> *born = result.value();
        born->SetClean();
        return born;
    }

    void Create(StorageBase &storage) const {
        CreateProperties(storage);
        Traits::LifetimeManager::Create(TypedStorage(storage));
    }

    bool Destroy(StorageBase &storage) const {
        return Traits::LifetimeManager::Destroy(TypedStorage(storage));
    }

    void Delete(StorageBase &storage) const {
        if (!properties_.empty()) {
            for (auto X : properties_) {
                PropertyBase const &prop = *X.second;
                if (!prop.IsSystemType()) continue;

                Object K = prop.GetObject(storage);
                if (K.Exists()) {
                    K.RemovedFromContainer(storage);
                }
            }
        }
        Traits::LifetimeManager::Delete(TypedStorage(storage));
    }

    Storage<T> *TypedStorage(StorageBase &storage) const {
        return reinterpret_cast<Storage<T> *>(&storage);
    }

    void CreateProperties(StorageBase &object) const {
        for (auto property : properties_) {
            PropertyBase const &prop = *property.second;
            if (!prop.IsSystemType()) continue;

            if (!prop.CreateDefaultValue()) continue;

            Object value = object.GetRegistry()->NewFromTypeNumber(
                prop.GetFieldTypeNumber());
            prop.SetObject(object, value);
        }
    }

    void Assign(StorageBase &A, StorageBase const &B) const {
        Traits::Assign::Perform(Deref<T>(A), ConstDeref<T>(B));
    }

    Object Duplicate(StorageBase const &parent) const {
        Storage<T> *result = parent.GetRegistry()->NewStorage<T>();
        Traits::Assign::Perform(result->GetReference(), ConstDeref<T>(parent));
        // foreach (Properties::value_type const &property, properties_)
        for (auto property : properties_) {
            PropertyBase const &prop = *property.second;
            // if it is a system-type property, clone it, else just store the
            // value
            if (prop.IsSystemType()) {
                auto ch = prop.GetObject(parent);
                if (ch.Exists())
                    prop.SetObject(*result, ch.Clone());
                else
                    prop.SetObject(*result, Object());
            } else
                prop.SetValue(*result, prop.GetValue(parent));
        }
        return *result;
    }

    void GetContainedObjects(StorageBase &object, ObjectList &contained) const {
        Traits::ContainerOps::ForEachContained(
            CleanDeref<T>(object), AddContainedFun<T, ObjectList>(contained));
    }

    void SetReferencedObjectsColor(StorageBase &Q, ObjectColor::Color C,
                                   HandleSet &H) const {
        ClassBase::SetReferencedObjectsColor(Q, C, H);
        Traits::ContainerOps::ForEachContained(
            CleanDeref<T>(Q), SetObjectColorRecursive<T>(C, H));
    }

    void SetSwitch(StorageBase &Q, int S, bool M) const {
        Traits::ContainerOps::SetSwitch(CleanDeref<T>(Q), S, M);
    }

    void DetachFromContainer(StorageBase &Q, Object const &K) const {
        Traits::ContainerOps::Erase(Deref<T>(Q), K);
    }

    void SetMarked2(StorageBase &Q, bool M) const {
        Traits::ContainerOps::SetMarked(CleanDeref<T>(Q), M);
    }
    void MakeReachableGrey(StorageBase &base) const {
        ClassBase::MakeReachableGrey(base);
        Traits::ContainerOps::ForEachContained(CleanDeref<T>(base),
                                               MakeReachableGreyFun<T>());
    }

    Object UpCast(StorageBase &Q) const {
        // This is almost always a bad idea. I see no reason to allow it in KAI
        // as well.
        KAI_NOT_IMPLEMENTED();
    }

    Object CrossCast(StorageBase &, Type::Number) const {
        KAI_NOT_IMPLEMENTED();
    }

    Object DownCast(StorageBase &, Type::Number) const {
        KAI_NOT_IMPLEMENTED();
    }

    HashValue GetHashValue(const StorageBase &Q) const {
        return Traits::HashFunction::Calc(ConstDeref<T>(Q));
    }

    Object Absolute(const StorageBase &object) const {
        Object result = object.Clone();
        Traits::Absolute::Perform(Deref<T>(object));
        return result;
    }

    bool Boolean(const StorageBase &A) const {
        return Traits::Boolean::Perform(ConstDeref<T>(A));
    }

    bool Less(const StorageBase &A, const StorageBase &B) const {
        return Traits::Less::Perform(ConstDeref<T>(A), ConstDeref<T>(B));
    }

    bool Equiv(const StorageBase &A, const StorageBase &B) const {
        if (!HasTraitsProperty(Type::Properties::Equiv) &&
            HasTraitsProperty(Type::Properties::Less))
            return !Less(A, B) && !Less(B, A);
        return Traits::Equiv::Perform(ConstDeref<T>(A), ConstDeref<T>(B));
    }

    bool Greater(const StorageBase &A, const StorageBase &B) const {
        return Traits::Greater::Perform(ConstDeref<T>(A), ConstDeref<T>(B));
    }

    StorageBase *Plus(StorageBase const &A, StorageBase const &B) const {
        Storage<T> *R = A.GetRegistry()->NewStorage<T>();
        Traits::Assign::Perform(
            R->GetReference(),
            Traits::Plus::Perform(ConstDeref<T>(A), ConstDeref<T>(B)));
        return R;
    }
    StorageBase *Minus(StorageBase const &A, StorageBase const &B) const {
        Storage<T> *R = A.GetRegistry()->NewStorage<T>();
        Traits::Assign::Perform(
            R->GetReference(),
            Traits::Minus::Perform(ConstDeref<T>(A), ConstDeref<T>(B)));
        return R;
    }

    StorageBase *Multiply(StorageBase const &A, StorageBase const &B) const {
        Storage<T> *R = A.GetRegistry()->NewStorage<T>();
        Traits::Assign::Perform(
            R->GetReference(),
            Traits::Multiply::Perform(ConstDeref<T>(A), ConstDeref<T>(B)));
        return R;
    }

    StorageBase *Divide(StorageBase const &A, StorageBase const &B) const {
        Storage<T> *R = A.GetRegistry()->NewStorage<T>();
        Traits::Assign::Perform(
            R->GetReference(),
            Traits::Divide::Perform(ConstDeref<T>(A), ConstDeref<T>(B)));
        return R;
    }

    void Insert(StringStream &S, const StorageBase &Q) const {
        Traits::StringStreamInsert::Insert(S, ConstDeref<T>(Q));
    }

    StorageBase *Extract(Registry &R, StringStream &S) const {
        Storage<T> *Q = R.NewStorage<T>();
        Traits::StringStreamExtract::Extract(S, Q->GetReference());
        return Q;
    }

    void ExtractValue(Object &object, StringStream &strstream) const {
        Traits::StringStreamExtract::Extract(strstream, Deref<T>(object));
    }

    void Insert(BinaryStream &S, const StorageBase &Q) const {
        Traits::BinaryStreamInsert::Insert(S, ConstDeref<T>(Q));
    }

    StorageBase *Extract(Registry &R, BinaryStream &S) const {
        Value<T> Q = R.New<T>();
        Traits::BinaryPacketExtract::Extract(S, *Q);
        return &Q.GetObject().GetStorageBase();
    }
};

template <class T>
Pointer<ClassBase const *> NewClass(Registry &R, const Label &name) {
    auto klass = new Class<T>(name);
    return R.AddClass(Type::Traits<T>::Number, klass);
}

template <class T>
Storage<T> *Clone(StorageBase const &Q) {
    auto dup = Q.GetRegistry()->NewStorage<T>();
    dup->GetClass()->Clone(*dup, Q);
    return dup;
}

#pragma warning(pop)

KAI_END
