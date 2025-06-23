#pragma once

#include <KAI/Core/Registry.h>

#include <memory>
#include <unordered_map>
#include <vector>

KAI_BEGIN

/// Modern C++ version of Registry using smart pointers
/// This can be used alongside the existing Registry for gradual migration
class SmartRegistry {
   public:
    // Modern type aliases using smart pointers
    using StoragePtr = std::unique_ptr<StorageBase>;
    using ConstStoragePtr = std::unique_ptr<const StorageBase>;
    using SharedStoragePtr = std::shared_ptr<StorageBase>;
    using WeakStoragePtr = std::weak_ptr<StorageBase>;

    // Smart pointer-based instances map
    using SmartInstances = std::unordered_map<Handle, StoragePtr, HashHandle>;

    // Shared instances for cases where shared ownership is needed
    using SharedInstances =
        std::unordered_map<Handle, SharedStoragePtr, HashHandle>;

   private:
    SmartInstances smart_instances_;
    SharedInstances shared_instances_;
    std::shared_ptr<Memory::IAllocator> allocator_;

   public:
    explicit SmartRegistry(std::shared_ptr<Memory::IAllocator> alloc = nullptr)
        : allocator_(alloc ? alloc
                           : std::make_shared<Memory::StandardAllocator>()) {}

    // Move-only semantics for better performance
    SmartRegistry(const SmartRegistry&) = delete;
    SmartRegistry& operator=(const SmartRegistry&) = delete;
    SmartRegistry(SmartRegistry&&) = default;
    SmartRegistry& operator=(SmartRegistry&&) = default;

    /// Create a new object with unique ownership
    template <typename T>
    std::unique_ptr<Storage<T>> NewUnique() {
        auto result = allocator_->Allocate<Storage<T>>();
        if (!result.has_value()) {
            KAI_THROW_0(OutOfMemory);
        }

        auto storage = std::unique_ptr<Storage<T>>(result.value());
        // Initialize the storage properly
        InitializeStorage(*storage);
        return storage;
    }

    /// Create a new object with shared ownership
    template <typename T>
    std::shared_ptr<Storage<T>> NewShared() {
        auto result = allocator_->Allocate<Storage<T>>();
        if (!result.has_value()) {
            KAI_THROW_0(OutOfMemory);
        }

        auto storage = std::shared_ptr<Storage<T>>(
            result.value(),
            [this](Storage<T>* ptr) { this->allocator_->DeAllocate(ptr); });

        InitializeStorage(*storage);
        return storage;
    }

    /// Register an object with unique ownership
    void RegisterUnique(Handle handle, StoragePtr storage) {
        smart_instances_[handle] = std::move(storage);
    }

    /// Register an object with shared ownership
    void RegisterShared(Handle handle, SharedStoragePtr storage) {
        shared_instances_[handle] = storage;
    }

    /// Get object with unique ownership (transfers ownership)
    StoragePtr ReleaseUnique(Handle handle) {
        auto it = smart_instances_.find(handle);
        if (it == smart_instances_.end()) {
            return nullptr;
        }

        auto result = std::move(it->second);
        smart_instances_.erase(it);
        return result;
    }

    /// Get object with shared ownership
    SharedStoragePtr GetShared(Handle handle) const {
        auto it = shared_instances_.find(handle);
        return it != shared_instances_.end() ? it->second : nullptr;
    }

    /// Get weak reference to shared object
    WeakStoragePtr GetWeak(Handle handle) const {
        auto shared = GetShared(handle);
        return shared ? WeakStoragePtr(shared) : WeakStoragePtr();
    }

    /// Check if handle exists in unique storage
    bool HasUnique(Handle handle) const {
        return smart_instances_.find(handle) != smart_instances_.end();
    }

    /// Check if handle exists in shared storage
    bool HasShared(Handle handle) const {
        return shared_instances_.find(handle) != shared_instances_.end();
    }

    /// Remove object from registry (for unique storage)
    bool RemoveUnique(Handle handle) {
        return smart_instances_.erase(handle) > 0;
    }

    /// Remove object from registry (for shared storage)
    bool RemoveShared(Handle handle) {
        return shared_instances_.erase(handle) > 0;
    }

    /// Get count of unique objects
    size_t GetUniqueCount() const { return smart_instances_.size(); }

    /// Get count of shared objects
    size_t GetSharedCount() const { return shared_instances_.size(); }

    /// Clear all objects (automatic cleanup via smart pointers)
    void Clear() {
        smart_instances_.clear();
        shared_instances_.clear();
    }

    /// Apply function to all unique objects
    template <typename F>
    void ForEachUnique(F&& func) {
        for (auto& [handle, storage] : smart_instances_) {
            func(handle, *storage);
        }
    }

    /// Apply function to all shared objects
    template <typename F>
    void ForEachShared(F&& func) {
        for (auto& [handle, storage] : shared_instances_) {
            if (storage) {  // Check if not expired
                func(handle, *storage);
            }
        }
    }

    /// Modern garbage collection using smart pointer reference counting
    void SmartGarbageCollect() {
        // Remove expired weak references from shared storage
        auto it = shared_instances_.begin();
        while (it != shared_instances_.end()) {
            if (!it->second || it->second.use_count() == 1) {
                // Only the registry holds a reference, safe to remove
                it = shared_instances_.erase(it);
            } else {
                ++it;
            }
        }
    }

    /// Get allocator
    std::shared_ptr<Memory::IAllocator> GetAllocator() const {
        return allocator_;
    }

   private:
    template <typename T>
    void InitializeStorage(Storage<T>& storage) {
        // Initialize storage with proper registry reference, etc.
        // This would be implementation-specific
    }
};

/// Factory function for creating smart registry
inline std::unique_ptr<SmartRegistry> MakeSmartRegistry(
    std::shared_ptr<Memory::IAllocator> allocator = nullptr) {
    return std::make_unique<SmartRegistry>(allocator);
}

/// RAII wrapper for registry objects
template <typename T>
class RegistryObjectGuard {
   private:
    SmartRegistry* registry_;
    Handle handle_;
    bool released_;

   public:
    RegistryObjectGuard(SmartRegistry* registry, Handle handle)
        : registry_(registry), handle_(handle), released_(false) {}

    ~RegistryObjectGuard() {
        if (!released_ && registry_) {
            registry_->RemoveUnique(handle_);
        }
    }

    // Move-only
    RegistryObjectGuard(const RegistryObjectGuard&) = delete;
    RegistryObjectGuard& operator=(const RegistryObjectGuard&) = delete;

    RegistryObjectGuard(RegistryObjectGuard&& other) noexcept
        : registry_(other.registry_),
          handle_(other.handle_),
          released_(other.released_) {
        other.released_ = true;
    }

    RegistryObjectGuard& operator=(RegistryObjectGuard&& other) noexcept {
        if (this != &other) {
            registry_ = other.registry_;
            handle_ = other.handle_;
            released_ = other.released_;
            other.released_ = true;
        }
        return *this;
    }

    void Release() { released_ = true; }

    Handle GetHandle() const { return handle_; }
};

KAI_END