#include <iostream>
#include <KAI/Core/Event.h>
#include <KAI/KAI.h>

using namespace kai;

// Simple function handler
void logMessage(int level, const std::string& message) {
    std::cout << "[Level " << level << "] " << message << std::endl;
}

// Class with method handler
class Logger {
public:
    void log(int level, const std::string& message) {
        std::cout << "[Logger " << level << "] " << message << std::endl;
    }

    void logWithTime(int level, const std::string& message) {
        std::cout << "[Logger " << level << " @ " << time(nullptr) << "] " << message << std::endl;
    }
};

// Demonstrating event usage
int main() {
    // Create a registry
    Registry registry;
    
    // Create an event that takes an int and a string
    Event<int, std::string> logEvent;
    
    // Register a function handler
    logEvent += logMessage;
    
    // Create an object and register its method
    Logger logger;
    logEvent += std::pair(&logger, &Logger::log);
    
    // Fire the event - both handlers will be called
    logEvent(1, "Test message");
    
    // Remove the function handler
    logEvent -= logMessage;
    
    // Now only the logger.log method will be called
    logEvent(2, "Another message");
    
    // Create a more complex event with three parameters
    Event<int, std::string, float> advancedEvent;
    
    // Register a lambda function
    advancedEvent += [](int id, const std::string& name, float value) {
        std::cout << "ID: " << id << ", Name: " << name << ", Value: " << value << std::endl;
    };
    
    // Fire the advanced event
    advancedEvent(42, "Test", 3.14159f);
    
    return 0;
}