#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Memory/IAllocator.h>

KAI_BEGIN

namespace Memory {
/// Commonality for all allocators
struct BaseAllocator : IAllocator {
    typedef char Byte;
    typedef char *BytePtr;

    typename IAllocator::Allocator alloc;
    typename IAllocator::DeAllocator free;

   protected:
    BaseAllocator() : alloc(0), free(0) {}

   public:
    VoidPtr AllocateBytes(size_t N) {
        if (!alloc) return 0;
        return alloc(N);
    }
    void DeAllocateBytes(VoidPtr ptr, size_t num_bytes) {
        if (!free) return;
        free(ptr, num_bytes);
    }
};
}  // namespace Memory

KAI_END
