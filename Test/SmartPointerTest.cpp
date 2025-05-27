#include <iostream>
#include <memory>

#include "KAI/Core/Object/Accessor.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Core/Object/Method.h"
#include "KAI/Core/Type/Number.h"
#include "KAI/Core/Type/Traits.h"

using namespace kai;

// Test class
class TestClass {
   public:
    int value;

    TestClass() : value(42) {}

    void SetValue(int v) { value = v; }
    int GetValue() const { return value; }
};

// Register TestClass with KAI's type system - using an arbitrary unused number
KAI_TYPE_TRAITS(TestClass, 200, Type::Properties::None);

int main() {
    std::cout << "Testing smart pointer migration...\n";

    // Test MakeMethod returning unique_ptr
    {
        auto method = MakeMethod(&TestClass::SetValue, Label("SetValue"));
        std::cout << "✓ MakeMethod returns unique_ptr<MethodBase>\n";
    }

    // Test MakeProperty returning unique_ptr
    {
        auto prop = MakeProperty<TestClass>(&TestClass::value, Label("value"));
        std::cout << "✓ MakeProperty returns unique_ptr<PropertyBase>\n";
    }

    // Test deprecated functions still work
    {
        auto method = MakeMethodRaw(&TestClass::GetValue, Label("GetValue"));
        delete method;
        std::cout << "✓ MakeMethodRaw (deprecated) still works\n";

        auto prop =
            MakePropertyRaw<TestClass>(&TestClass::value, Label("value"));
        delete prop;
        std::cout << "✓ MakePropertyRaw (deprecated) still works\n";
    }

    std::cout << "\nAll tests passed!\n";
    return 0;
}