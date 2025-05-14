#include <KAI/KAI.h>
#include <KAI/Core/Logger.h>

int main() {
    // Initialize KAI registry
    kai::Registry reg;
    
    // Create a Pi executor
    kai::Pointer<kai::Executor> exec = reg.New<kai::Executor>();
    
    // Run a simple Pi script that should produce the token message
    exec->Execute("pi{ 1 2 + }");
    
    return 0;
}