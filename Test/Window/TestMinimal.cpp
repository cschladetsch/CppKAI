#include <iostream>
#include <memory>

// Forward declarations to avoid including full headers
namespace kai {
namespace Memory {
class IAllocator {
   public:
    virtual ~IAllocator() = default;
    virtual void* AllocateBytes(size_t size) = 0;
    virtual void DeAllocateBytes(void* ptr) = 0;
};

class StandardAllocator : public IAllocator {
   public:
    void* AllocateBytes(size_t size) override { return ::operator new(size); }

    void DeAllocateBytes(void* ptr) override { ::operator delete(ptr); }
};
}  // namespace Memory
}  // namespace kai

int main() {
    std::cout << "Creating allocator..." << std::endl;
    auto alloc = std::make_shared<kai::Memory::StandardAllocator>();
    std::cout << "Allocator created" << std::endl;

    std::cout << "Allocating memory..." << std::endl;
    void* ptr = alloc->AllocateBytes(100);
    std::cout << "Memory allocated" << std::endl;

    alloc->DeAllocateBytes(ptr);
    std::cout << "Memory freed" << std::endl;

    return 0;
}