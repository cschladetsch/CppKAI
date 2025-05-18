#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Memory/IAllocator.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/Object/PropertyBase.h>
#include <KAI/Core/Pointer.h>
#include <KAI/Core/Type/Traits.h>
#include <KAI/UnfuckWin32.h>

#include <set>
#include <unordered_map>

KAI_BEGIN

class Tree;

template <class T>
Pointer<ClassBase const *> NewClass(Registry &R, const Label &name);

struct Registry {
   public:
#ifdef KAI_BOOST_UNORDERED_REGISTRY
    typedef std::unordered_map<Handle, StorageBase *, HashHandle> Instances;
#elif defined(KAI_HASH_TABLE_REGISTRY)
    typedef std::hash_map<Handle, StorageBase *, HashHandle> Instances;
#else
    typedef std::map<Handle, StorageBase *> Instances;
#endif
    typedef std::vector<const ClassBase *> Classes;
    typedef HandleSet Handles, DeathRow;
    typedef HandleSet RetainedObjects;
    typedef std::vector<Handle> VectorHandles;
    typedef std::set<Handle> ColoredSet;

    // _pools of objects indexed by type number
    typedef std::vector<std::vector<StorageBase *> > Pools;

    /// A number in range [0..100)
    typedef int Percentage;

    typedef std::list<Object> Roots;
    Roots roots_;

   private:
    friend struct StorageBase;
    ColoredSet white_;  /// condemned set. may be referenced by objects in the
                        /// white_ or grey_ sets
    ColoredSet grey_;   /// may or may not have refernces to objects in the
                        /// white_, black_ or grey_ sets
    ColoredSet black_;  /// has no references to objects in the white_ set

    Classes classes_;
    Instances instances_;
    Handles retained_;
    DeathRow deathRow_;
    Handle nextHandle_{0};
    Tree *tree_{nullptr};
    RetainedObjects retainedObjects_;
    Pools pools_;
    std::shared_ptr<Memory::IAllocator> allocator_;
    bool ownsAllocator_{false};

    // Track handles that couldn't be properly deleted
    std::vector<Handle> failed_deletions_;

   public:
    Registry();
    Registry(std::shared_ptr<Memory::IAllocator>);
    ~Registry();

    Memory::IAllocator &GetMemorySystem() const { return *allocator_; }

    template <class T>
    Pointer<ClassBase const *> AddClass() {
        return NewClass<T>(*this, Label(String(Type::Traits<T>::Name())));
    }

    template <class T>
    Pointer<ClassBase const *> AddClass(const Label &N) {
        return NewClass<T>(*this, N);
    }

    template <class T>
    Object New() {
        const ClassBase *klass = GetClass(Type::Traits<T>::Number);
        if (klass == 0) KAI_THROW_0(UnknownClass<T>);
        return NewFromClass(klass);
    }

    template <class T>
    Object New(const T &value) {
        Pointer<T> pointer = New<T>();
        *pointer = value;
        return pointer;
    }

    static Object New(const Object &object) { return object; }

    template <class T>
    Object NewRetained() {
        Object retained = New<T>();
        retained.SetManaged(false);
        retained.SetBlack();
        retainedObjects_.insert(retained.GetHandle());
        return retained;
    }

    template <class T>
    Storage<T> *NewStorage() {
        const ClassBase *klass = GetClass(Type::Traits<T>::Number);
        if (klass == 0) KAI_THROW_0(UnknownClass<T>);
        Object Q = NewFromClass(klass);
        return static_cast<Storage<T> *>(Q.GetBasePtr());
    }

    template <class T>
    const ClassBase *GetClass() {
        return GetClass(Type::Traits<T>::Number);
    }

    RetainedObjects const &GetRetainedObjects() const {
        return retainedObjects_;
    }
    void PruneRetained();
    const ClassBase *GetClass(Type::Number);
    const ClassBase *GetClass(const Label &);
    StorageBase *GetStorageBase(Handle) const;

    Object GetObject(Handle) const;

    template <class T>
    Pointer<T> GetPointer(Handle handle) const {
        return GetObject(handle);
    }

    template <class T>
    Pointer<const T> GetConstPointer(Handle handle) const {
        return GetObject(handle);
    }

    void Delete(Handle);
    void Delete(Object const &);

    void AddClass(const ClassBase *K);

    template <class T>
    void FreeResources(T *p) {
        allocator_->DeAllocate(p);
    }

    template <class T>
    Storage<T> *CloneStorage(const Object &Q) {
        KAI_NOT_IMPLEMENTED();
    }

    Pointer<ClassBase const *> AddClass(Type::Number, ClassBase const *);
    void Clear();
    void ClearInstances();
    const Instances &GetInstances() const { return instances_; }
    const Classes &GetClasses() const { return classes_; }
    Percentage CalcMemoryUsage() const;
    Percentage CalcMemoryFragmentationPercentage() const;
    void DefragmentMemory();
    Tree *GetTree() const { return tree_; }

    // Check if this registry instance is valid and operational
    bool IsValid() const;
    void SetTree(Tree &);
    bool Pin(Handle);
    bool Unpin(Handle);
    bool Pin(const Object &Q) { return Pin(Q.GetHandle()); }
    bool Unpin(const Object &Q) { return Unpin(Q.GetHandle()); }

    Object NewFromTypeNumber(Type::Number type_number);
    Object NewFromClassName(const char *classname_str);
    Object NewFromClass(const ClassBase *klass);

    void AddRoot(Object const &root);
    void TriColor();
    void ReleaseWhite();
    void Mark(Object root);
    void Sweep();
    void MarkAndSweep(Object root);
    void MarkSweepAndDestroy(Object root);
    void GarbageCollect(Object root);
    void MarkAll(StorageBase &root, bool marked) const;
    void DestroyObject(Handle, bool force = false);
    void GarbageCollect();
    bool SetColor(StorageBase &, ObjectColor::Color);
    bool OnDeathRow(Handle) const;

    int gc_trace_level{};
    void SetGCTraceLevel(int);

    // Get a list of handles that failed deletion
    const std::vector<Handle> &GetFailedDeletions() const {
        return failed_deletions_;
    }

    // Clear the list of failed deletions
    void ClearFailedDeletions() { failed_deletions_.clear(); }

#ifdef KAI_DEBUG_REGISTRY
    void DeleteRetained();
    void UpdateProcesses(float time_delta);

    typedef std::set<Type::Number> ObservedTypes;
    typedef std::set<Handle> Observed;
    ObservedTypes observed_types;
    Observed observed;

    bool IsWatching(Object const &Q) const { return IsWatching(Q.GetHandle()); }
    bool IsWatching(Handle) const;
    void WatchObject(Handle handle, bool = true);
    void WatchObject(Object const &Q, bool W = true) {
        WatchObject(Q.GetHandle(), W);
    }
    void WatchAllObjects();
    void WatchNoObjects() { observed.clear(); }
    void StopWatchingObject(Handle handle) { WatchObject(handle, false); }
    bool IsWatchingObject(Object const &object) const {
        return IsWatchingObject(object.GetHandle());
    }
    bool IsWatchingObject(Handle handle) const {
        return observed.find(handle) != observed.end();
    }

    void WatchAllTypes();
    void WatchNoTypes();
    template <class T>
    void WatchType(bool watch = true) {
        WatchType(Type::Traits<T>::Number, watch);
    }
    void WatchType(Type::Number N, bool = true);
    void StopWatchingType(Type::Number N) { WatchType(N, false); }
    bool IsWatchingType(Type::Number N) const {
        return observed_types.find(N) != observed_types.end();
    }

    void TraceGCCounts() const;
    void TraceGrey() const;
    void TraceWhite() const;

    void TraceSet(Registry::ColoredSet const &set, const char *name_) const;
    void TraceTriColor() const;
    /// @endgroup debugging
    String Trace() const;
#endif

   private:
    void Construct();

    void NominateAll();
    void DestroyNominated();
    int GetDeathrowSize() const { return (int)deathRow_.size(); }
};

template <class T>
Storage<T> *NewStorage(Registry &R) {
    return R.NewStorage<T>();
}

#ifdef KAI_DEBUG_REGISTRY
#define KAI_REGISTRY_TRACE(Q)                \
    do {                                     \
        if (!Q.GetRegistry()) {              \
            KAI_TRACE() << "Empty registry"; \
        } else {                             \
            Q.GetRegistry().TraceObject(Q)
#define KAI_DEBUG_WATCH_OBJECT(object) \
    if (object) object.GetRegistry()->WatchObject(object)
#else
#define KAI_DEBUG_WATCH_OBJECT(object) KAI_UNUSED_1(object)
#endif

template <class T>
void SetProperty(Object const &owner, Label const &name, T const &value) {
    SetPropertyValue(owner, name, owner.GetRegistry()->New(value));
}

KAI_END

#include "KAI/Core/Object/Reflected.h"
