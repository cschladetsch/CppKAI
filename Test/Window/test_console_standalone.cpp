#include <KAI/Core/Memory/StandardAllocator.h>
#include <KAI/Core/Registry.h>

#include <exception>
#include <iostream>
#include <memory>

using namespace kai;

int main() {
    std::cout << "Test 1: Creating allocator..." << std::endl;

    try {
        // Step 1: Test allocator
        auto alloc = std::make_shared<Memory::StandardAllocator>();
        std::cout << "Allocator created successfully" << std::endl;

        // Step 2: Test Registry creation
        std::cout << "\nTest 2: Creating Registry..." << std::endl;

        // Try to create Registry with allocator
        auto reg_result = alloc->Allocate<Registry>(alloc);
        if (!reg_result.has_value()) {
            std::cerr << "Failed to allocate Registry" << std::endl;
            return 1;
        }

        auto* reg = reg_result.value();
        std::cout << "Registry created successfully" << std::endl;

        // Step 3: Register basic types
        std::cout << "\nTest 3: Registering types..." << std::endl;
        reg->AddClass<void>(Label("Void"));
        reg->AddClass<bool>(Label("Bool"));
        reg->AddClass<int>(Label("Int"));
        std::cout << "Basic types registered" << std::endl;

        // Clean up
        alloc->DeAllocate(reg);
        std::cout << "\nAll tests passed!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }

    return 0;
}