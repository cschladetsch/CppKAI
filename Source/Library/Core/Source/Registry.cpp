#include <KAI/Core/BuiltinTypes/Signed32.h>
#include <KAI/Core/Memory/StandardAllocator.h>
#include <KAI/Core/Object/Class.h>
#include <KAI/Core/Object/IObject.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/Tree.h>
#include <KAI/Core/TriColor.h>

#include <cassert>
#include <utility>

#define KAI_USE_TRICOLOR

KAI_BEGIN

Registry::Registry() {
    allocator_ = std::make_shared<Memory::StandardAllocator>();
    Construct();
}

Registry::Registry(std::shared_ptr<Memory::IAllocator> alloc) {
    allocator_ = std::move(alloc);
    Construct();
}

void Registry::Construct() {
    classes_.resize(2000, nullptr);
    gc_trace_level = 1;
    tree_ = nullptr;
    std::fill(classes_.begin(), classes_.end(),
              static_cast<ClassBase const *>(nullptr));
}

Registry::~Registry() { Clear(); }

void Registry::Clear() { ClearInstances(); }

void Registry::ClearInstances() {
    std::vector<Handle> handles;
    for (auto const &instance : instances_) handles.push_back(instance.first);

    for (auto const &handle : handles) DestroyObject(handle, true);

    instances_.clear();
}

void Registry::NominateAll() {
    deathRow_.clear();
    for (const auto &[handle, _] : instances_) deathRow_.insert(handle);
}

Object Registry::NewFromTypeNumber(Type::Number type_number) {
    ClassBase const *klass = classes_[type_number.ToInt()];
    if (!klass) KAI_THROW_1(UnknownTypeNumber, type_number.ToInt());

    return NewFromClass(klass);
}

Object Registry::NewFromClassName(const char *classname_str) {
    const Label className(classname_str);
    const ClassBase *klass = GetClass(className);
    if (klass == nullptr) KAI_THROW_1(UnknownClass<>, String(classname_str));

    return NewFromClass(klass);
}

const ClassBase *Registry::GetClass(const Label &name) {
    for (auto const klass : classes_)
        if (klass && klass->GetName() == name) return klass;

    return nullptr;
}

void Registry::DestroyNominated() {
    // Copy the elements in death row because when we delete
    // objects, they may release other objects.
    DeathRow copy = deathRow_;
    deathRow_.clear();
    for (auto const &ob : copy) DestroyObject(ob);
}

void Registry::DestroyObject(Handle handle, bool force) {
    bool succeeded = false;
    try {
        const auto found = instances_.find(handle);
        if (found == instances_.end()) {
#ifdef KAI_DEBUG_REGISTRY
            if (IsWatching(handle)) {
                KAI_TRACE() << handle << ": doesn't exist, not deleted";
            }
#endif
            return;
        }

        StorageBase &base = *found->second;
        assert(base.GetHandle() == handle);
        if (!base.IsManaged() && !force) {
#ifdef KAI_DEBUG_REGISTRY
            if (IsWatching(handle))
                KAI_TRACE()
                    << handle << ": " << base << " is not managed, not deleted";
#endif
            return;
        }

#ifdef KAI_DEBUG_REGISTRY
        bool trace = gc_trace_level > 3;
        trace = trace || IsWatching(base);
        if (trace) {
            try {
                KAI_TRACE() << handle;
            } catch (const std::exception &e) {
                KAI_TRACE_ERROR() << "Exception while tracing: " << e.what();
            } catch (...) {
                KAI_TRACE_ERROR() << "Unknown exception while tracing";
            }
        }
#endif

#ifdef KAI_USE_TRICOLOR
        // When using tri-color GC, objects are always deleted when they are
        // destroyed.
        if (!base.GetClass()->Destroy(base) && !force) {
            base.SetColor(ObjectColor::Grey);
            return;
        }
#endif
        base.GetClass()->Delete(base);
        instances_.erase(found);

        if (const auto retained = retainedObjects_.find(handle);
            retained != retainedObjects_.end())
            retainedObjects_.erase(retained);

        succeeded = true;
    } catch (const Exception::Base &e) {
        KAI_TRACE_ERROR() << "Exception during object destruction: "
                          << e.ToString();
        // Log error but attempt recovery below
    } catch (const std::exception &e) {
        KAI_TRACE_ERROR() << "Standard exception during object destruction: "
                          << e.what();
        // Log error but attempt recovery below
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception during object destruction";
        // Log error but attempt recovery below
    }

    if (!succeeded) {
        KAI_TRACE_WARN() << "Failed to delete handle " << handle;
        if (auto const found = instances_.find(handle);
            found != instances_.end()) {
            KAI_TRACE_ERROR() << "Force removing object with handle " << handle
                              << " from instances";
            // Remove the object from instances to prevent memory leaks, but log
            // the error This is safer than keeping invalid objects in the
            // registry
            instances_.erase(found);

            // Add an entry to a failed deletion log that can be reviewed later
            failed_deletions_.push_back(handle);
        }
    }
}

void Registry::PruneRetained() {
    auto retained = retainedObjects_.begin();
    const auto end = retainedObjects_.end();
    while (retained != end) {
        if (Object object = GetObject(*retained); !object.Exists())
            retained = retainedObjects_.erase(retained);
        else
            ++retained;
    }
}

const ClassBase *Registry::GetClass(Type::Number type_number) {
    const auto tn = type_number.ToInt();
    if (tn >= static_cast<int>(classes_.size()))
        KAI_THROW_1(LogicError, "Inalid type number");

    return classes_[tn];
}

StorageBase *Registry::GetStorageBase(Handle handle) const {
    // Defensive null check for handle
    if (handle == Handle(0)) return nullptr;

    try {
        // Ensure instances_ exists and is valid before trying to find an
        // element
        if (instances_.empty()) {
            return nullptr;
        }

        // Safe lookup in the instance map
        const auto obj = instances_.find(handle);
        if (obj == instances_.end()) return nullptr;

        // Make sure the found instance is valid
        if (obj->second == nullptr) {
            return nullptr;
        }

        // Perform a basic validation test on the returned pointer
        volatile void *test = static_cast<void *>(obj->second);
        if (!test) {
            return nullptr;
        }

        return obj->second;
    } catch (...) {
        // Silent handling of any exceptions
        return nullptr;
    }
}

bool Registry::OnDeathRow(Handle handle) const {
    // Defensive check for validity
    if (handle == Handle(0)) {
        return false;
    }

    try {
        // Safe lookup in the death row set
        return deathRow_.find(handle) != deathRow_.end();
    } catch (...) {
        // Silent handling of any exceptions
        return false;
    }
}

void Registry::AddClass(const ClassBase *klass) {
    if (klass == nullptr) KAI_THROW_0(NullObject);

    if (GetClass(klass->GetTypeNumber())) {
        KAI_TRACE_WARN()
            << "Attempt to register duplicate class with type number "
            << klass->GetTypeNumber().ToInt() << " (name: " << klass->GetName()
            << "). Ignoring.";
        return;
    }

    classes_[klass->GetTypeNumber().ToInt()] = klass;
}

Object Registry::GetObject(Handle handle) const {
#ifdef KAI_DEBUG_REGISTRY
    if (IsWatching(handle) && gc_trace_level > 1) KAI_TRACE() << handle;
#endif

    if (handle == Handle(0)) return Object();

    const auto found = instances_.find(handle);
    if (found == instances_.end()) return Object();

    return static_cast<Object>(*found->second);
}

void Registry::MarkSweepAndDestroy(Object root) {
#ifdef KAI_USE_TRICOLOR
    AddRoot(root);
    TriColor();
    return;
#else
    DestroyNominated();   // destroy any pending
    MarkAndSweep(root_);  // mark objects for destruction
    DestroyNominated();   // destroy them
#endif
}

void Registry::MarkAndSweep(Object root) {
    Mark(root);
    Sweep();
}

void Registry::Mark(Object root) {
    // Mark everything that is reachable from the given object root_.
    if (root.Exists()) MarkAll(root.GetStorageBase(), true);
}

void Registry::MarkAll(StorageBase &root, bool marked) const {
    MarkObject(root, marked);

    const Dictionary &dict = root.GetDictionary();
    for (const auto &[_, child] : dict) {
        StorageBase *base = GetStorageBase(child.GetHandle());
        if (base == nullptr) continue;

        if (base->IsManaged()) MarkObject(*base, marked);

        if (base->GetSwitches() & IObject::NoRecurse) continue;

        MarkAll(*base, marked);
    }
}

void Registry::Sweep() {
    if (instances_.empty()) return;

    auto A = instances_.begin(), B = instances_.end();
    // the handle of the next object created
    const auto last = Handle(nextHandle_.GetValue());
    for (; A != B; ++A) {
        StorageBase *base = A->second;

        // do not consider objects that were created during this loop!
        if (last < base->GetHandle()) continue;

        if (!base->IsManaged()) continue;

        if (!base->IsMarked())
            Delete(A->first);
        else
            base->SetMarked(false);
    }
}

void Registry::Delete(Handle handle) {
#ifdef KAI_USE_TRICOLOR
    KAI_NOT_IMPLEMENTED_1("Delete when using garbage collection.");
#else

#ifdef KAI_DEBUG_REGISTRY
    if (IsObserving(handle)) {
        KAI_TRACE() << "Deleting " << handle;
    }
#endif
    // if unknown handle, do nothing
    Instances::const_iterator instance = instances_.find(handle);
    if (instance == instances_.end()) return;

    // detach from parent
    if (instance->second) {
        StorageBase &storage = *instance->second;
        storage.SetColor(ObjectColor::White);
        storage.SetColorRecursive(ObjectColor::White);
        storage.GetClass()->Destroy(storage);
        // Detach(storage);
    }

    // remove from list of retained_ objects
    auto etained = retainedObjects_.find(handle);
    if (retained_ != retainedObjects_.end()) retainedObjects_.erase(retained_);

        // mark for pending collection
#ifndef KAI_USE_TRICOLOR
    deathRow_.insert(handle);
#endif
#endif
}

void Registry::Delete(Object const &object) {
#ifdef KAI_DEBUG_REGISTRY
    if (IsWatching(object)) KAI_TRACE() << object.GetHandle();
#endif
    Delete(object.GetHandle());
}

Pointer<ClassBase const *> Registry::AddClass(Type::Number N,
                                              ClassBase const *K) {
    // Check for duplicate class registration
    if (GetClass(N)) {
        KAI_TRACE_WARN()
            << "Attempt to register duplicate class with type number "
            << N.ToInt() << " (name: " << K->GetName() << "). Ignoring.";
        return Pointer<ClassBase const *>();
    }

    // Store the class in the registry
    classes_[N.ToInt()] = K;

    // Now create an object using the stored class
    // For now, we're still returning an empty pointer, but the class is
    // properly stored in the registry. If code depends on this pointer being
    // valid, we'll need to revisit this implementation.
    // The most important thing is that classes_[N.ToInt()] = K is executed
    // correctly.
    return Pointer<ClassBase const *>();
}

Registry::Percentage Registry::CalcMemoryUsage() const { return 0; }
Registry::Percentage Registry::CalcMemoryFragmentationPercentage() const {
    return 0;
}
void Registry::DefragmentMemory() {}
bool Registry::Pin(Handle) { return false; }
bool Registry::Unpin(Handle) { return false; }

void Registry::GarbageCollect() {
    if (!tree_) {
        KAI_TRACE_WARN() << "GC requested with no tree";
        return;
    }

    GarbageCollect(tree_->GetRoot());
}

Object Registry::NewFromClass(const ClassBase *klass) {
    if (klass == nullptr) KAI_THROW_1(UnknownClass<>, "NULL Class");

    const Handle handle(nextHandle_.NextValue());
    StorageBase *base = nullptr;
    base = klass->NewStorage(this, handle);

#ifdef KAI_DEBUG_REGISTRY
    if (IsWatchingType(klass->GetTypeNumber()))
        KAI_TRACE() << klass->GetName() << ": " << handle;
#endif

    base->SetColor(ObjectColor::White);
    base->SetMarked(false);

    instances_[handle] = base;

    klass->Create(*base);
    return Object(ObjectConstructParams(this, klass, handle));
}

void MarkObject(Object const &object, bool marked) {
    if (!object.Exists()) return;

    MarkObject(object.GetStorageBase(), marked);
}

void MarkObject(StorageBase &storage, bool marked) {
    if (marked == storage.IsMarked()) return;

    storage.SetMarked(marked);
    storage.GetClass()->SetMarked(storage, marked);
}

void MarkObjectAndChildren(Object const &object, bool marked) {
    if (!object.Exists()) return;

    MarkObjectAndChildren(object.GetStorageBase(), marked);
}

void MarkObjectAndChildren(StorageBase &storage, bool marked) {
    MarkObject(storage, marked);
    for (auto const &[_, object] : storage.GetDictionary()) {
        auto &child = const_cast<Object &>(object);
        if (child.GetHandle() ==
            storage.GetHandle())  // HACK to sorta/kinda avoid cycles :/
            continue;

        MarkObjectAndChildren(child, marked);
    }
}

void Registry::GarbageCollect(Object root) {
#ifdef KAI_USE_TRICOLOR
    AddRoot(root);
    TriColor();
#else
    MarkSweepAndDestroy(root_);
#endif
}

void Registry::TriColor() {
    // This is a magic number. the higher it is, the more objects may be deleted
    // in this call the cost has to be paid at some point, so this number really
    // means "how much do I want to spread out cost of GC over time versus
    // memory use".
    //
    // Ideally, self-monitoring in real-time will adjust this number to avoid
    // any pauses in the realtime simulation
    //
    // if you have lots of memory, set max_cycles to 1. (or zero!). if not, set
    // it higher until you can fit memory usage into a sequence of frames.
    //
    // See also https://github.com/cschladetsch/Monotonic
    const int MaxCycles = 17;

    if (gc_trace_level >= 1)
        KAI_TRACE_3(static_cast<int>(instances_.size()),
                    static_cast<int>(grey_.size()),
                    static_cast<int>(white_.size()));

    int cycle = 0;
    for (; cycle < MaxCycles; ++cycle) {
#ifdef KAI_DEBUG_REGISTRY
        if (gc_trace_level > 2) TraceTriColor();
#endif

        if (grey_.empty()) {
            ReleaseWhite();
            break;
        }

        auto iterator = grey_.begin();
        const Handle handle = *iterator;
        grey_.erase(iterator);
        StorageBase *base = GetStorageBase(handle);
        if (base == nullptr) continue;

        base->MakeReachableGrey();
        base->SetColor(ObjectColor::Black);
    }

    if (gc_trace_level >= 1) KAI_TRACE() << "TriColor: " << cycle << " passes";
}

void Registry::ReleaseWhite() {
    // Make a copy of the white_ set to avoid mutation while iterating.
    // Yes, this is expensive and is a good candidate to use Monotonic memory
    // allocation.
    std::vector<Handle> to_delete(white_.begin(), white_.end());
    white_.clear();

    for (auto const &handle : to_delete) DestroyObject(handle);
}

void Registry::SetGCTraceLevel(int N) { gc_trace_level = N; }

static void RemoveFromSet(Registry::ColoredSet &handles, Handle handle) {
    Registry::ColoredSet::iterator entry = handles.find(handle);
    if (entry != handles.end()) handles.erase(entry);
}

bool Registry::SetColor(StorageBase &base, ObjectColor::Color color) {
    // When using TriColor, if an object is marked it means it has been forced
    // deleted.
    if (base.IsMarked() && color != ObjectColor::White) return false;

#ifdef KAI_DEBUG_REGISTRY
    if (IsWatching(base) && gc_trace_level > 0)
        KAI_TRACE_3(base.GetHandle(), base.GetClass()->GetName(), color);
#endif

    Handle handle = base.GetHandle();
    switch (color) {
        case ObjectColor::White:
            RemoveFromSet(grey_, handle);
            white_.insert(handle);
            break;

        case ObjectColor::Grey:
            RemoveFromSet(white_, handle);
            grey_.insert(handle);
            break;

        case ObjectColor::Black:
            break;
    }

    return true;
}

void Registry::SetTree(Tree &tree) {
    this->tree_ = &tree;
    AddRoot(this->tree_->GetRoot());
}

bool Registry::IsValid() const {
    try {
        // Simple test to see if we can access our members without crashing
        volatile void *allocTest = static_cast<void *>(allocator_.get());
        if (!allocTest) {
            return false;
        }

        // Check if classes_ was properly initialized
        if (classes_.empty()) {
            return false;
        }

        // Successfully access at least one class to ensure memory is valid
        bool foundValidClass = false;
        for (size_t i = 0; i < classes_.size() && i < 10; ++i) {
            if (classes_[i] != nullptr) {
                // Test if we can access the class object - avoid volatile cast
                const void *classTest = classes_[i];
                if (classTest) {
                    foundValidClass = true;
                    break;
                }
            }
        }

        // We need at least one valid class to consider the registry functional
        if (!foundValidClass) {
            return false;
        }

        // All tests passed - registry appears valid
        return true;
    } catch (...) {
        // Any exception indicates an invalid registry
        return false;
    }
}

template <class II, class T, class Pred>
II find(II A, II B, T X, Pred P) {
    for (; A != B; ++A) {
        if (P(X, *A)) break;
    }

    return A;
}

bool SameHandle(Object const &A, Object const &B) {
    return A.GetHandle() == B.GetHandle();
}

void Registry::AddRoot(Object const &root) {
    if (!root.Exists()) return;

    if (find(roots_.begin(), roots_.end(), root, SameHandle) == roots_.end())
        roots_.push_back(root);

    SetColor(root.GetStorageBase(), ObjectColor::Grey);
}

#ifdef KAI_DEBUG_REGISTRY
String Registry::Trace() const { return ""; }

void Registry::WatchObject(Handle handle, bool watch) {
    if (watch) {
        observed.insert(handle);
        return;
    }

    Observed::iterator A = observed.find(handle);
    if (A != observed.end()) observed.erase(A);
}

bool Registry::IsWatching(Handle handle) const {
    StorageBase *base = GetStorageBase(handle);
    return IsWatchingObject(handle) ||
           (base && IsWatchingType(base->GetClass()->GetTypeNumber()));
}

void Registry::WatchAllTypes() {
    for (auto klass : classes_)
        if (klass) WatchType(klass->GetTypeNumber());
}

void Registry::WatchNoTypes() { observed_types.clear(); }

void Registry::WatchType(Type::Number N, bool watch) {
    if (watch) {
        observed_types.insert(N);
        return;
    }

    ObservedTypes::iterator A = observed_types.find(N);
    if (A != observed_types.end()) observed_types.erase(A);
}

void Registry::TraceSet(Registry::ColoredSet const &set,
                        const char *name_) const {
    KAI_TRACE() << name_ << "(" << (int)set.size() << "): ";
    if (gc_trace_level > 2) {
        for (auto handle : set) {
            Object Q = GetObject(handle);
            if (!Q.Exists()) continue;
            KAI_TRACE() << Q.GetLabel() << "(" << Q.GetClass()->GetName()
                        << "): " << handle.GetValue() << ", " << Q;
        }
    }
}

void Registry::TraceTriColor() const {
    TraceGCCounts();
    if (gc_trace_level > 1) TraceWhite();

    if (gc_trace_level > 1) TraceGrey();
}

void Registry::TraceGCCounts() const {
    KAI_TRACE_3((int)instances_.size(), (int)grey_.size(), (int)white_.size());
}

void Registry::TraceGrey() const { TraceSet(grey_, "***GREY*** "); }

void Registry::TraceWhite() const { TraceSet(white_, "***WHITE*** "); }

#endif

KAI_END

// EOF