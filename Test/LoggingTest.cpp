#include <KAI/Core/Debug.h>
#include <KAI/Core/Logger.h>

#include <iostream>

using namespace KAI;

int main() {
    std::cout << "Starting logging test..." << std::endl;

    // Test KAI_TRACE() macro
    KAI_TRACE() << "This is a test trace message";

    // Test different log levels
    KAI_TRACE_WARN() << "This is a warning message";
    KAI_TRACE_ERROR() << "This is an error message";

    // Test with arguments
    int testValue = 42;
    std::string testString = "test string";
    KAI_TRACE_1(testValue) << "Testing with a variable";
    KAI_TRACE_2(testValue, testString) << "Testing with multiple variables";

    // Test Logger directly
    Logger::Init();
    Logger::InfoWithLocation("Direct logger test", __FILE__, __LINE__);
    Logger::WarningWithLocation("Direct warning test", __FILE__, __LINE__);

    std::cout << "Logging test complete." << std::endl;
    return 0;
}