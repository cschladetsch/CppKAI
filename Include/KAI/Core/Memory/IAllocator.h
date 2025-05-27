#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Debug.h>

#include <concepts>
#include <memory>
// Using variant instead of expected which may not be available yet
#include <optional>
#include <span>
#include <system_error>
#include <variant>

KAI_BEGIN

namespace Memory {
/// interface for all memory allocation systems
struct IAllocator {
   protected:
    IAllocator() {}
    virtual ~IAllocator() {}

   public:
    typedef void *VoidPtr;
    typedef char *BytePtr;

    typedef std::size_t size_t;

    typedef VoidPtr (*Allocator)(size_t);
    typedef void (*DeAllocator)(VoidPtr, size_t);

    virtual VoidPtr AllocateBytes(size_t) = 0;
    virtual void DeAllocateBytes(VoidPtr, size_t) = 0;

    template <class T>
    void Construct(T *ptr) {
        ::new (ptr) T;
    }

    template <class T, class U>
    void Construct(T *ptr, U const &V) {
        ::new (ptr) T(V);
    }

    template <class T>
    void Destruct(T *ptr) {
        if (ptr) ptr->~T();
    }

    template <typename T>
        requires std::default_initializable<T>
    std::optional<T *> Allocate() {
        size_t num_bytes = sizeof(T);
        VoidPtr bytes = AllocateBytes(num_bytes);
        if (!bytes) return std::nullopt;

        T *ptr = reinterpret_cast<T *>(bytes);
        try {
            Construct(ptr);
        } catch (...) {
            DeAllocateBytes(bytes, num_bytes);
            return std::nullopt;
        }
        return ptr;
    }

    template <typename T, typename U>
        requires std::constructible_from<T, U>
    std::optional<T *> Allocate(U const &val) {
        size_t num_bytes = sizeof(T);
        VoidPtr bytes = AllocateBytes(num_bytes);
        if (!bytes) return std::nullopt;

        T *ptr = reinterpret_cast<T *>(bytes);
        try {
            Construct(ptr, val);
        } catch (...) {
            DeAllocateBytes(bytes, num_bytes);
            return std::nullopt;
        }
        return ptr;
    }

    template <class T>
    void DeAllocate(T *ptr) {
        if (!ptr) return;
        try {
            Destruct(ptr);
        } catch (const std::exception &e) {
            KAI_TRACE_ERROR() << "exception destructing object at " << ptr
                              << ": " << e.what();
        } catch (...) {
            KAI_TRACE_ERROR()
                << "unknown exception destructing object at " << ptr;
        }
        DeAllocateBytes(reinterpret_cast<VoidPtr>(ptr), sizeof(T));
    }

    template <typename T>
        requires std::default_initializable<T>
    std::optional<std::span<T>> AllocateArray(size_t N) {
        size_t num_bytes = sizeof(T) * N;
        VoidPtr base = AllocateBytes(num_bytes);
        if (!base) return std::nullopt;

        T *typed_base = reinterpret_cast<T *>(base);
        try {
            for (size_t i = 0; i < N; ++i) {
                Construct(typed_base + i);
            }
        } catch (...) {
            // Clean up any constructed elements
            for (size_t i = 0; i < N; ++i) {
                try {
                    Destruct(typed_base + i);
                } catch (...) {
                    // Ignore nested exceptions during cleanup
                }
            }
            DeAllocateBytes(base, num_bytes);
            return std::nullopt;
        }
        return std::span<T>(typed_base, N);
    }

    template <class T>
    void DeAllocateArray(T *ptr, size_t N) {
        if (!ptr) return;
        BytePtr base = reinterpret_cast<BytePtr>(ptr);
        try {
            for (; N != 0; --N) {
                Destruct(reinterpret_cast<T *>(base));
                base += sizeof(T);
            }
        } catch (const std::exception &e) {
            KAI_TRACE_ERROR()
                << "exception releasing object at " << base << ": " << e.what();
        } catch (...) {
            KAI_TRACE_ERROR()
                << "unknown exception releasing object at " << base;
        }
        DeAllocateBytes(ptr, sizeof(T) * N);
    }
};
}  // namespace Memory

KAI_END
