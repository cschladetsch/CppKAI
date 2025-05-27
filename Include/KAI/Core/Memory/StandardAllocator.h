#pragma once

#include <KAI/Core/Config/Base.h>

#include <cstdlib>
#include <memory>
#include <new>

#include "BaseAllocator.h"
#include "KAI/Core/Memory/BaseAllocator.h"

KAI_BEGIN

namespace Memory {
/// Standard allocator using modern C++ memory management
struct StandardAllocator : BaseAllocator {
    using BaseAllocator::alloc;
    using BaseAllocator::free;

    StandardAllocator() {
        alloc = &StandardAllocator::sys_malloc;
        free = &StandardAllocator::sys_free;
    }

    static void *sys_malloc(std::size_t N) {
        return std::allocator<std::byte>{}.allocate(N);
    }

    static void sys_free(void *P, std::size_t N) {
        std::allocator<std::byte>{}.deallocate(static_cast<std::byte *>(P), N);
    }
};
}  // namespace Memory

KAI_END
