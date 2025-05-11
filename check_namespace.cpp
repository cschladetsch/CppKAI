#include <iostream>
#include <KAI/Core/Config/Options.h>

int main() {
    std::cout << "KAI_NAMESPACE_NAME: " << STRINGIFY(KAI_NAMESPACE_NAME) << std::endl;
    return 0;
}

// Helper macros for stringification
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)