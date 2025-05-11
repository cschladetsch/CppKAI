#define KAI_NAMESPACE_NAME kai

#include <KAI/Core/Logger.h>
#include <iostream>

using namespace kai;

int main() {
    // Ensure Logger is initialized with the Logs directory
    Logger::Init("/home/xian/local/KAI/Logs");
    
    // Log some test messages
    Logger::Debug("Debug test message - warning fixed");
    Logger::Info("Info test message - warning fixed");
    Logger::Warning("Warning test message - warning fixed");
    Logger::Error("Error test message - warning fixed");
    Logger::Fatal("Fatal test message - warning fixed");
    
    std::cout << "Logging test completed with the KAI_NAMESPACE_NAME issue fixed." << std::endl;
    
    return 0;
}