#include <KAI/Core/Memory/StandardAllocator.h>

#include <iostream>

int main() {
    std::cout << "Creating allocator..." << std::endl;

    kai::Memory::StandardAllocator alloc;

    std::cout << "Allocating 100 bytes..." << std::endl;
    void* ptr = alloc.AllocateBytes(100);

    if (ptr) {
        std::cout << "Allocation successful" << std::endl;
        alloc.DeAllocateBytes(ptr);
        std::cout << "Deallocation successful" << std::endl;
    } else {
        std::cout << "Allocation failed" << std::endl;
    }

    return 0;
}