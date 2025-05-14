#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Debug.h>

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

    template <class T>
    T *Allocate() {
        size_t num_bytes = sizeof(T);
        VoidPtr bytes = AllocateBytes(num_bytes);
        if (!bytes) return 0;
        T *ptr = reinterpret_cast<T *>(bytes);
        try {
            Construct(ptr);
        } catch (...) {
            DeAllocateBytes(bytes, num_bytes);
            ptr = 0;
        }
        return ptr;
    }

    template <class T, class U>
    T *Allocate(U const &val) {
        size_t num_bytes = sizeof(T);
        VoidPtr bytes = AllocateBytes(num_bytes);
        if (!bytes) return 0;
        T *ptr = reinterpret_cast<T *>(bytes);
        try {
            Construct(ptr, val);
        } catch (...) {
            DeAllocateBytes(bytes, num_bytes);
            ptr = 0;
        }
        return ptr;
    }

    template <class T>
    void DeAllocate(T *ptr) {
        if (!ptr) return;
        try {
            Destruct(ptr);
        } catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "exception destructing object at " << ptr << ": " << e.what();
        } catch (...) {
            KAI_TRACE_ERROR() << "unknown exception destructing object at " << ptr;
        }
        DeAllocateBytes(reinterpret_cast<VoidPtr>(ptr), sizeof(T));
    }

    template <class T>
    T **AllocateArray(size_t N) {
        size_t num_bytes = sizeof(T) * N;
        VoidPtr base = AllocateBytes(num_bytes);
        try {
            for (BytePtr ptr = reinterpret_cast<BytePtr>(base); N > 0; --N) {
                Construct(reinterpret_cast<T *>(ptr));
                ptr += sizeof(T);
            }
        } catch (...) {
            DeAllocateBytes(base, num_bytes);
            base = 0;
        }
        return base;
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
        } catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "exception releasing object at " << base << ": " << e.what();
        } catch (...) {
            KAI_TRACE_ERROR() << "unknown exception releasing object at " << base;
        }
        DeAllocateBytes(ptr, sizeof(T) * N);
    }
};
}  // namespace Memory

KAI_END
