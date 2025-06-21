#include <KAI/Core/SmartRegistry.h>
#include <KAI/Core/ModernPointer.h>
#include <KAI/Core/BuiltinTypes.h>
#include <iostream>
#include <memory>

USING_NAMESPACE_KAI

/// Demonstration of modern C++ improvements for KAI
void DemonstrateModernCpp() {
    std::cout << "=== Modern C++ Features for KAI ===\n\n";
    
    // 1. Smart Registry with RAII and smart pointers
    {
        std::cout << "1. Smart Registry with automatic memory management:\n";
        
        auto smart_registry = MakeSmartRegistry();
        
        // Create objects with unique ownership
        auto unique_int = smart_registry->NewUnique<int>();
        auto unique_string = smart_registry->NewUnique<String>();
        
        std::cout << "   - Created unique objects: " 
                  << smart_registry->GetUniqueCount() << " unique objects\n";
        
        // Create objects with shared ownership
        auto shared_float = smart_registry->NewShared<float>();
        auto shared_array = smart_registry->NewShared<Array>();
        
        std::cout << "   - Created shared objects: " 
                  << smart_registry->GetSharedCount() << " shared objects\n";
        
        // Register objects with handles
        Handle h1{1}, h2{2}, h3{3}, h4{4};
        smart_registry->RegisterUnique(h1, std::move(unique_int));
        smart_registry->RegisterUnique(h2, std::move(unique_string));
        smart_registry->RegisterShared(h3, shared_float);
        smart_registry->RegisterShared(h4, shared_array);
        
        std::cout << "   - Registered objects with handles\n";
        
        // Access objects safely
        if (auto shared_obj = smart_registry->GetShared(h3)) {
            std::cout << "   - Successfully retrieved shared object (use_count: " 
                      << shared_obj.use_count() << ")\n";
        }
        
        // Automatic cleanup when smart_registry goes out of scope
        std::cout << "   - Objects will be automatically cleaned up\n\n";
    }
    
    // 2. Modern Pointer with concepts and type safety
    {
        std::cout << "2. Modern Pointer with C++20 concepts:\n";
        
        // Create using standard smart pointers
        auto std_shared = std::make_shared<String>("Hello, Modern C++!");
        auto std_unique = std::make_unique<int>(42);
        
        // Wrap in ModernPointer
        ModernPointer<String> modern_str(std_shared);
        ModernPointer<int> modern_int(std::move(std_unique));
        
        std::cout << "   - Created ModernPointer from std::shared_ptr and std::unique_ptr\n";
        
        // Use like regular pointers
        if (modern_str && modern_int) {
            std::cout << "   - String value: " << modern_str->ToString() << "\n";
            std::cout << "   - Int value: " << *modern_int << "\n";
        }
        
        // Factory functions with perfect forwarding
        auto factory_str = make_modern_shared<String>("Factory created!");
        auto factory_int = make_modern_unique<int>(123);
        
        std::cout << "   - Used factory functions with perfect forwarding\n";
        std::cout << "   - Factory string: " << factory_str->ToString() << "\n";
        std::cout << "   - Factory int: " << *factory_int << "\n\n";
    }
    
    // 3. RAII guards for automatic cleanup
    {
        std::cout << "3. RAII guards for automatic resource management:\n";
        
        Registry registry;
        registry.AddClass<String>();
        registry.AddClass<int>();
        registry.AddClass<Array>();
        
        {
            // Create objects with RAII guards
            auto str_guard = make_kai_guard<String>(registry, "RAII Protected");
            auto int_guard = make_kai_guard<int>(registry, 999);
            
            std::cout << "   - Created RAII-protected KAI objects\n";
            std::cout << "   - String: " << str_guard->ToString() << "\n";
            std::cout << "   - Int: " << **int_guard << "\n";
            
            // Objects are automatically managed when guards go out of scope
        }
        
        std::cout << "   - RAII guards automatically cleaned up\n\n";
    }
    
    // 4. Exception-safe operations with std::optional
    {
        std::cout << "4. Exception-safe operations with std::optional:\n";
        
        auto allocator = std::make_shared<Memory::StandardAllocator>();
        
        // Safe allocation that returns std::optional
        auto result = allocator->Allocate<String>();
        if (result.has_value()) {
            std::cout << "   - Safe allocation succeeded\n";
            
            // Initialize the allocated string
            new (result.value()) String("Safely allocated");
            std::cout << "   - String initialized: " << result.value()->ToString() << "\n";
            
            // Clean up
            result.value()->~String();
            allocator->DeAllocate(result.value());
            std::cout << "   - Memory safely deallocated\n";
        } else {
            std::cout << "   - Allocation failed gracefully\n";
        }
        
        std::cout << "\n";
    }
    
    // 5. Modern C++20 concepts for type safety
    {
        std::cout << "5. C++20 Concepts for compile-time type safety:\n";
        
        // These will compile because String is a KAI object type
        static_assert(KaiObjectType<String>, "String should be a KAI object type");
        static_assert(KaiObjectType<int>, "int should be a KAI object type");
        static_assert(KaiObjectType<Array>, "Array should be a KAI object type");
        
        std::cout << "   - Compile-time type checking with concepts passed\n";
        std::cout << "   - Types are verified to be valid KAI object types\n\n";
    }
    
    std::cout << "=== Modern C++ Demo Complete ===\n";
}

/// Example of migrating legacy code to modern C++
void LegacyToModernMigration() {
    std::cout << "\n=== Legacy to Modern Migration Example ===\n\n";
    
    Registry legacy_registry;
    legacy_registry.AddClass<String>();
    legacy_registry.AddClass<Array>();
    
    // Legacy approach (what we're improving)
    std::cout << "Legacy approach:\n";
    Object legacy_obj = legacy_registry.New<String>("Legacy String");
    Pointer<String> legacy_ptr = legacy_obj;
    std::cout << "   - Legacy string: " << legacy_ptr->ToString() << "\n";
    
    // Modern approach (using our new features)
    std::cout << "\nModern approach:\n";
    {
        auto str_guard = make_kai_guard<String>(legacy_registry, "Modern String");
        ModernPointer<String> modern_ptr(str_guard.GetPointer());
        
        std::cout << "   - Modern string: " << modern_ptr->ToString() << "\n";
        std::cout << "   - Automatic cleanup with RAII\n";
        std::cout << "   - Exception safety built-in\n";
        std::cout << "   - Type safety with concepts\n";
    }
    
    std::cout << "\n=== Migration Example Complete ===\n";
}

int main() {
    try {
        DemonstrateModernCpp();
        LegacyToModernMigration();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}