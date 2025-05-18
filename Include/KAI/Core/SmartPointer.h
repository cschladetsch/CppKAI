#pragma once

#include <KAI/Core/Config/Base.h>

#include <memory>

KAI_BEGIN

// Modern C++ smart pointer wrappers for KAI
// These provide standard C++ smart pointer functionality while maintaining
// compatibility with the KAI object system

// Forward declarations
class Object;
class StorageBase;

// Smart pointer for KAI objects using std::shared_ptr under the hood
template <typename T>
class SharedPtr {
   private:
    std::shared_ptr<T> ptr;
    Object* object = nullptr;

   public:
    SharedPtr() = default;

    // Construct from raw pointer
    explicit SharedPtr(T* raw_ptr) : ptr(raw_ptr) {}

    // Construct from Object
    explicit SharedPtr(const Object& obj);

    // Move and copy constructors
    SharedPtr(const SharedPtr&) = default;
    SharedPtr(SharedPtr&&) noexcept = default;

    // Assignment operators
    SharedPtr& operator=(const SharedPtr&) = default;
    SharedPtr& operator=(SharedPtr&&) noexcept = default;

    // Dereferencing
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr.get(); }

    // Conversion to std::shared_ptr
    std::shared_ptr<T> get_shared() const { return ptr; }

    // Conversion to raw pointer
    T* get() const { return ptr.get(); }

    // Null check
    explicit operator bool() const { return static_cast<bool>(ptr); }

    // Associated KAI Object
    Object* get_object() const { return object; }
    void set_object(Object* obj) { object = obj; }
};

// Unique ownership smart pointer for KAI objects
template <typename T>
class UniquePtr {
   private:
    std::unique_ptr<T> ptr;
    Object* object = nullptr;

   public:
    UniquePtr() = default;

    // Construct from raw pointer
    explicit UniquePtr(T* raw_ptr) : ptr(raw_ptr) {}

    // Move constructors
    UniquePtr(UniquePtr&&) noexcept = default;

    // Assignment operators
    UniquePtr& operator=(UniquePtr&&) noexcept = default;

    // Delete copy operations to enforce unique ownership
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Dereferencing
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr.get(); }

    // Conversion to std::unique_ptr (releases ownership)
    std::unique_ptr<T> release() { return std::move(ptr); }

    // Conversion to raw pointer (doesn't release ownership)
    T* get() const { return ptr.get(); }

    // Null check
    explicit operator bool() const { return static_cast<bool>(ptr); }

    // Associated KAI Object
    Object* get_object() const { return object; }
    void set_object(Object* obj) { object = obj; }
};

// Non-owning weak reference to a KAI object managed by a SharedPtr
template <typename T>
class WeakPtr {
   private:
    std::weak_ptr<T> ptr;
    Object* object = nullptr;

   public:
    WeakPtr() = default;

    // Construct from SharedPtr
    WeakPtr(const SharedPtr<T>& shared)
        : ptr(shared.get_shared()), object(shared.get_object()) {}

    // Copy and move operations
    WeakPtr(const WeakPtr&) = default;
    WeakPtr(WeakPtr&&) noexcept = default;
    WeakPtr& operator=(const WeakPtr&) = default;
    WeakPtr& operator=(WeakPtr&&) noexcept = default;

    // Conversion to SharedPtr
    SharedPtr<T> lock() const {
        SharedPtr<T> result(ptr.lock());
        result.set_object(object);
        return result;
    }

    // Check if the referenced object still exists
    bool expired() const { return ptr.expired(); }

    // Associated KAI Object
    Object* get_object() const { return object; }
};

// Helper function to create SharedPtr
template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

// Helper function to create UniquePtr
template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

KAI_END