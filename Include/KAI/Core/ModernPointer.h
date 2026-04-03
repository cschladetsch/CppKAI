#pragma once

#include <KAI/Core/Pointer.h>

#include <concepts>
#include <memory>
#include <optional>

KAI_BEGIN

/// Modern C++ concepts for type constraints
template <typename T>
concept KaiObjectType = requires {
    typename Type::Traits<T>;
    Type::Traits<T>::Number;
};

template <typename T>
concept CopyConstructible = std::copy_constructible<T>;

template <typename T>
concept MoveConstructible = std::move_constructible<T>;

/// Modern smart pointer wrapper that can interoperate with KAI's Pointer<T>
/// system
template <typename T>
    requires KaiObjectType<T>
class ModernPointer {
   private:
    std::optional<Pointer<T>> kai_pointer_;
    std::shared_ptr<T> modern_ptr_;

   public:
    using element_type = T;
    using pointer = T*;
    using reference = T&;
    using const_reference = const T&;

    /// Default constructor
    ModernPointer() = default;

    /// Construct from KAI Pointer
    explicit ModernPointer(Pointer<T> ptr) : kai_pointer_(ptr) {}

    /// Construct from std::shared_ptr
    explicit ModernPointer(std::shared_ptr<T> ptr)
        : modern_ptr_(std::move(ptr)) {}

    /// Construct from unique_ptr (takes ownership)
    explicit ModernPointer(std::unique_ptr<T> ptr)
        : modern_ptr_(std::move(ptr)) {}

    /// Copy constructor
    ModernPointer(const ModernPointer& other) = default;

    /// Move constructor
    ModernPointer(ModernPointer&& other) noexcept = default;

    /// Copy assignment
    ModernPointer& operator=(const ModernPointer& other) = default;

    /// Move assignment
    ModernPointer& operator=(ModernPointer&& other) noexcept = default;

    /// Assignment from KAI Pointer
    ModernPointer& operator=(Pointer<T> ptr) {
        kai_pointer_ = ptr;
        modern_ptr_.reset();
        return *this;
    }

    /// Assignment from shared_ptr
    ModernPointer& operator=(std::shared_ptr<T> ptr) {
        modern_ptr_ = std::move(ptr);
        kai_pointer_.reset();
        return *this;
    }

    /// Assignment from unique_ptr
    ModernPointer& operator=(std::unique_ptr<T> ptr) {
        modern_ptr_ = std::move(ptr);
        kai_pointer_.reset();
        return *this;
    }

    /// Dereference operator
    reference operator*() const {
        if (kai_pointer_) {
            return *kai_pointer_.value();
        } else if (modern_ptr_) {
            return *modern_ptr_;
        }
        throw std::runtime_error("Attempted to dereference null ModernPointer");
    }

    /// Arrow operator
    pointer operator->() const {
        if (kai_pointer_) {
            return &(*kai_pointer_.value());
        } else if (modern_ptr_) {
            return modern_ptr_.get();
        }
        return nullptr;
    }

    /// Get raw pointer
    pointer get() const {
        if (kai_pointer_) {
            return &(*kai_pointer_.value());
        } else if (modern_ptr_) {
            return modern_ptr_.get();
        }
        return nullptr;
    }

    /// Boolean conversion
    explicit operator bool() const noexcept {
        return (kai_pointer_ && kai_pointer_->Exists()) ||
               (modern_ptr_ != nullptr);
    }

    /// Check if pointer is valid
    bool valid() const noexcept { return static_cast<bool>(*this); }

    /// Reset the pointer
    void reset() {
        kai_pointer_.reset();
        modern_ptr_.reset();
    }

    /// Get the underlying KAI pointer (if any)
    std::optional<Pointer<T>> GetKaiPointer() const { return kai_pointer_; }

    /// Get the underlying std::shared_ptr (if any)
    std::shared_ptr<T> GetSharedPtr() const { return modern_ptr_; }

    /// Get reference count (if using shared_ptr)
    long use_count() const noexcept {
        return modern_ptr_ ? modern_ptr_.use_count() : 0;
    }

    /// Check if this is the unique owner (for shared_ptr)
    bool unique() const noexcept { return modern_ptr_ && modern_ptr_.unique(); }

    /// Swap with another ModernPointer
    void swap(ModernPointer& other) noexcept {
        std::swap(kai_pointer_, other.kai_pointer_);
        std::swap(modern_ptr_, other.modern_ptr_);
    }

    /// Comparison operators
    bool operator==(const ModernPointer& other) const {
        return get() == other.get();
    }

    bool operator!=(const ModernPointer& other) const {
        return !(*this == other);
    }

    bool operator<(const ModernPointer& other) const {
        return get() < other.get();
    }

    /// Compare with nullptr
    bool operator==(std::nullptr_t) const noexcept { return !valid(); }

    bool operator!=(std::nullptr_t) const noexcept { return valid(); }

    /// Type conversion utilities
    template <typename U>
        requires std::convertible_to<T*, U*>
    ModernPointer<U> cast() const {
        if (kai_pointer_) {
            // Cast KAI pointer
            auto kai_cast = kai_pointer_->template Cast<U>();
            return ModernPointer<U>(kai_cast);
        } else if (modern_ptr_) {
            // Cast std::shared_ptr
            auto modern_cast = std::static_pointer_cast<U>(modern_ptr_);
            return ModernPointer<U>(modern_cast);
        }
        return ModernPointer<U>();
    }

    /// Dynamic cast for polymorphic types
    template <typename U>
    ModernPointer<U> dynamic_cast_to() const {
        if (kai_pointer_) {
            // For KAI pointers, use the existing cast mechanism
            if (auto* raw_ptr = dynamic_cast<U*>(get())) {
                // This is a simplified approach - in practice you'd need to
                // create a proper KAI pointer to U
                return ModernPointer<U>();  // TODO: Implement proper KAI cast
            }
        } else if (modern_ptr_) {
            auto modern_cast = std::dynamic_pointer_cast<U>(modern_ptr_);
            if (modern_cast) {
                return ModernPointer<U>(modern_cast);
            }
        }
        return ModernPointer<U>();
    }
};

/// Factory functions
template <typename T, typename... Args>
    requires KaiObjectType<T> && std::constructible_from<T, Args...>
ModernPointer<T> make_modern_shared(Args&&... args) {
    return ModernPointer<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
    requires KaiObjectType<T> && std::constructible_from<T, Args...>
ModernPointer<T> make_modern_unique(Args&&... args) {
    return ModernPointer<T>(std::make_unique<T>(std::forward<Args>(args)...));
}

/// Convert from KAI Pointer to ModernPointer
template <typename T>
ModernPointer<T> to_modern(Pointer<T> ptr) {
    return ModernPointer<T>(ptr);
}

/// Convert from ModernPointer to KAI Pointer (if possible)
template <typename T>
std::optional<Pointer<T>> to_kai(const ModernPointer<T>& ptr) {
    return ptr.GetKaiPointer();
}

/// Utility for RAII management of KAI objects in modern C++ style
template <typename T>
class KaiObjectGuard {
   private:
    Registry* registry_;
    Object object_;
    bool released_;

   public:
    template <typename... Args>
    KaiObjectGuard(Registry& registry, Args&&... args)
        : registry_(&registry), released_(false) {
        object_ = registry_->New<T>(std::forward<Args>(args)...);
    }

    ~KaiObjectGuard() {
        if (!released_ && registry_ && object_.Valid()) {
            // The object will be cleaned up by KAI's GC
            // or we could explicitly delete it here if needed
        }
    }

    // Move-only
    KaiObjectGuard(const KaiObjectGuard&) = delete;
    KaiObjectGuard& operator=(const KaiObjectGuard&) = delete;

    KaiObjectGuard(KaiObjectGuard&& other) noexcept
        : registry_(other.registry_),
          object_(std::move(other.object_)),
          released_(other.released_) {
        other.released_ = true;
    }

    KaiObjectGuard& operator=(KaiObjectGuard&& other) noexcept {
        if (this != &other) {
            registry_ = other.registry_;
            object_ = std::move(other.object_);
            released_ = other.released_;
            other.released_ = true;
        }
        return *this;
    }

    Object& get() { return object_; }
    const Object& get() const { return object_; }

    Pointer<T> GetPointer() const { return object_; }

    void release() { released_ = true; }

    T& operator*() { return Deref<T>(object_); }
    const T& operator*() const { return ConstDeref<T>(object_); }

    Storage<T>* operator->() {
        return static_cast<Storage<T>*>(object_.GetBasePtr());
    }
    const Storage<T>* operator->() const {
        return static_cast<const Storage<T>*>(object_.GetBasePtr());
    }
};

/// Factory function for KaiObjectGuard
template <typename T, typename... Args>
KaiObjectGuard<T> make_kai_guard(Registry& registry, Args&&... args) {
    return KaiObjectGuard<T>(registry, std::forward<Args>(args)...);
}

KAI_END