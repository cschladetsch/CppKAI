#include <KAI/Core/Debug.h>
#include <KAI/Core/Logger.h>
#include <KAI/Network/NetworkLogger.h>

using namespace KAI;
using namespace KAI::Network;

// Simple test program to demonstrate the file and line information in logs
int main() {
    // Initialize loggers
    Logger::Init();
    NetworkLogger::Init();

    // Test standard logging macros
    KAI_LOG_INFO("This is an info message");
    KAI_LOG_WARNING("This is a warning message");
    KAI_LOG_ERROR("This is an error message");
    
    // Test network logging macros
    KAI_NET_LOG_CONNECTION("This is a connection message");
    KAI_NET_LOG_MESSAGE("This is a network message");
    KAI_NET_LOG_DISCOVERY("This is a discovery message");
    KAI_NET_LOG_STATUS("This is a status message");
    
    // Test trace macros
    KAI_TRACE() << "This is a trace message";
    KAI_TRACE_WARN() << "This is a warning trace";
    KAI_TRACE_ERROR() << "This is an error trace";
    
    // Test trace macros with parameters
    int testValue = 42;
    std::string testString = "test string";
    KAI_TRACE_1(testValue) << "Trace with one parameter";
    KAI_TRACE_2(testValue, testString) << "Trace with two parameters";
    
    return 0;
}