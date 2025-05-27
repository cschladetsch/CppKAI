#pragma once

// This is a proposed update to Registry showing smart pointer usage
// for ClassBase and StorageBase management

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/Type/Traits.h>

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

KAI_BEGIN

// Forward declarations
class Tree;
template <class T>
class Class;

class Registry_SmartPtr {
   public:
    // Use shared_ptr for classes since they're shared across the system
    typedef std::vector<std::shared_ptr<const ClassBase>> Classes;

    // Use shared_ptr for storage instances with weak_ptr for back-references
    typedef std::unordered_map<Handle, std::shared_ptr<StorageBase>> Instances;

    // Weak references to prevent cycles
    typedef std::unordered_map<Handle, std::weak_ptr<StorageBase>>
        WeakInstances;

    typedef std::set<Handle> HandleSet;
    typedef std::vector<Object> Roots;

   private:
    Classes classes_;
    Instances instances_;
    WeakInstances weak_instances_;  // For objects that might have circular refs
    HandleSet retained_;
    Handle next_handle_{1};
    Roots roots_;

    // Smart pointer to memory allocator
    std::shared_ptr<Memory::IAllocator> allocator_;

   public:
    Registry_SmartPtr();
    explicit Registry_SmartPtr(std::shared_ptr<Memory::IAllocator> alloc);
    ~Registry_SmartPtr() = default;  // No manual cleanup needed!

    // Add a class using shared_ptr
    template <class T>
    std::shared_ptr<const Class<T>> AddClass(const Label& name) {
        auto klass = std::make_shared<Class<T>>(name);
        AddClass(klass);
        return klass;
    }

    void AddClass(std::shared_ptr<const ClassBase> klass) {
        if (!klass) KAI_THROW_0(NullObject);

        auto type_num = klass->GetTypeNumber().ToInt();
        if (type_num >= classes_.size()) classes_.resize(type_num + 1);

        if (classes_[type_num]) KAI_THROW_1(Base, "Duplicate Class");

        classes_[type_num] = klass;
    }

    // Get class as shared_ptr
    std::shared_ptr<const ClassBase> GetClass(Type::Number type_num) const {
        auto index = type_num.ToInt();
        if (index >= classes_.size()) return nullptr;
        return classes_[index];
    }

    // Create new object with shared ownership
    template <class T>
    Object New() {
        auto klass = GetClass(Type::Traits<T>::Number);
        if (!klass) KAI_THROW_0(UnknownClass<T>);

        return NewFromClass(klass.get());
    }

    // Factory method returning shared_ptr
    template <class T>
    std::shared_ptr<Storage<T>> NewStorage() {
        auto handle = next_handle_++;
        auto klass = GetClass(Type::Traits<T>::Number);
        if (!klass) KAI_THROW_0(UnknownClass<T>);

        // Create storage with shared ownership
        auto storage = std::make_shared<Storage<T>>(
            ObjectConstructParams(this, klass.get(), handle));

        // Register in instances map
        instances_[handle] = storage;

        // Initialize the storage
        klass->Create(*storage);

        return storage;
    }

    // Get object by handle (returns shared_ptr)
    std::shared_ptr<StorageBase> GetStorage(Handle handle) {
        auto it = instances_.find(handle);
        if (it != instances_.end()) return it->second;

        // Check weak references
        auto weak_it = weak_instances_.find(handle);
        if (weak_it != weak_instances_.end()) {
            // Try to lock the weak_ptr
            if (auto locked = weak_it->second.lock()) {
                // Promote back to strong reference
                instances_[handle] = locked;
                weak_instances_.erase(weak_it);
                return locked;
            }
        }

        return nullptr;
    }

    // Downgrade to weak reference (useful for breaking cycles)
    void MakeWeak(Handle handle) {
        auto it = instances_.find(handle);
        if (it != instances_.end()) {
            weak_instances_[handle] = it->second;
            instances_.erase(it);
        }
    }

    // Delete object (removes from registry)
    void Delete(Handle handle) {
        // Remove from both maps
        instances_.erase(handle);
        weak_instances_.erase(handle);
        retained_.erase(handle);

        // The object will be automatically destroyed when
        // all shared_ptr references go out of scope
    }

    // Garbage collection with smart pointers
    void CollectGarbage() {
        // Remove expired weak pointers
        auto it = weak_instances_.begin();
        while (it != weak_instances_.end()) {
            if (it->second.expired()) {
                it = weak_instances_.erase(it);
            } else {
                ++it;
            }
        }

        // Smart pointers handle memory cleanup automatically
        // This method now just cleans up bookkeeping
    }

    // Example of safe object retrieval
    template <class T>
    std::shared_ptr<T> GetObject(Handle handle) {
        auto storage = GetStorage(handle);
        if (!storage) return nullptr;

        // Safe dynamic cast with shared_ptr
        return std::dynamic_pointer_cast<Storage<T>>(storage);
    }
};

// RAII helper for automatic object registration/deregistration
class ScopedObject {
    std::shared_ptr<Registry_SmartPtr> registry_;
    Handle handle_;

   public:
    ScopedObject(std::shared_ptr<Registry_SmartPtr> reg, Handle h)
        : registry_(reg), handle_(h) {}

    ~ScopedObject() {
        if (registry_) registry_->Delete(handle_);
    }

    // Prevent copying
    ScopedObject(const ScopedObject&) = delete;
    ScopedObject& operator=(const ScopedObject&) = delete;

    // Allow moving
    ScopedObject(ScopedObject&& other) noexcept
        : registry_(std::move(other.registry_)), handle_(other.handle_) {
        other.handle_ = Handle(0);
    }
};

KAI_END

// Usage example comment:
/*
// Old way with manual memory management:
Registry reg;
auto klass = new Class<MyType>("MyType");
reg.AddClass(Type::Number(42), klass);  // Registry takes ownership
Storage<MyType>* storage = reg.NewStorage<MyType>();  // Manual delete needed

// New way with smart pointers:
auto reg = std::make_shared<Registry_SmartPtr>();
auto klass = reg->AddClass<MyType>("MyType");  // Returns shared_ptr
auto storage = reg->NewStorage<MyType>();  // Returns shared_ptr, auto cleanup

// Breaking circular references:
reg->MakeWeak(child_handle);  // Downgrade child to weak reference
*/