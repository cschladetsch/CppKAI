#include <KAI/Core/Base.h>
#include <KAI/Core/Debug.h>
#include <KAI/Core/Logger.h>
#include <KAI/Network/NetworkLogger.h>
#include <iostream>

using namespace KAI;
using namespace KAI::Network;

// Simple program to test the logger output
int main() {
    try {
        // Initialize loggers
        std::cout << "Initializing loggers..." << std::endl;
        Logger::Init();
        NetworkLogger::Init();
        
        std::cout << "Testing basic Logger methods..." << std::endl;
        // Test basic Logger methods directly
        Logger::Info("Direct info message");
        Logger::Warning("Direct warning message");
        Logger::Error("Direct error message");
        
        std::cout << "Testing location-aware Logger methods..." << std::endl;
        // Test location-aware Logger methods directly
        Logger::InfoWithLocation("Info message with location", __FILE__, __LINE__);
        Logger::WarningWithLocation("Warning message with location", __FILE__, __LINE__);
        Logger::ErrorWithLocation("Error message with location", __FILE__, __LINE__);
        
        std::cout << "Testing NetworkLogger methods..." << std::endl;
        // Test NetworkLogger methods directly
        NetworkLogger::LogConnection("Direct connection message");
        NetworkLogger::LogMessage("Direct network message");
        
        std::cout << "Testing location-aware NetworkLogger methods..." << std::endl;
        // Test location-aware NetworkLogger methods directly
        NetworkLogger::LogConnectionWithLocation("Connection message with location", __FILE__, __LINE__);
        NetworkLogger::LogMessageWithLocation("Network message with location", __FILE__, __LINE__);
        
        std::cout << "Test completed successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}