#include <iostream>
#include <memory>

// Forward declarations to avoid including full headers
namespace kai {
namespace memory
{
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
} // namespace memory
}  // namespace kai

int main() {
    std::cout << "Creating allocator..." << '\n';
    auto alloc = std::make_shared<kai::memory::StandardAllocator>();
    std::cout << "Allocator created" << '\n';

    std::cout << "Allocating memory..." << '\n';
    void* ptr = alloc->AllocateBytes(100);
    std::cout << "Memory allocated" << '\n';

    alloc->DeAllocateBytes(ptr);
    std::cout << "Memory freed" << '\n';

    return 0;
}