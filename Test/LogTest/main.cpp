#include <KAI/Core/Base.h>
#include <KAI/Core/Debug.h>
#include <KAI/Core/Logger.h>

#include <iostream>

// Simple program to test the logger output
int main() {
    try {
        // Initialize loggers
        std::cout << "Initializing loggers..." << std::endl;
        kai::Logger::Init();

        // Ensure trace settings are set to show file and line info
        kai::debug::Trace::TraceFileLocation = true;
        kai::debug::Trace::StripPath = true;
        kai::debug::Trace::TraceFunction = true;

        std::cout << "Testing basic Logger methods..." << std::endl;
        // Test basic Logger methods directly
        kai::Logger::Info("Direct info message");
        kai::Logger::Warning("Direct warning message");
        kai::Logger::Error("Direct error message");

        std::cout << "Testing location-aware Logger methods..." << std::endl;
        // Test location-aware Logger methods directly
        kai::Logger::InfoWithLocation("Info message with location", __FILE__,
                                      __LINE__);
        kai::Logger::WarningWithLocation("Warning message with location",
                                         __FILE__, __LINE__);
        kai::Logger::ErrorWithLocation("Error message with location", __FILE__,
                                       __LINE__);

        std::cout << "Testing KAI_TRACE macros..." << std::endl;
        // Test basic KAI_TRACE macros
        KAI_TRACE() << "This is a basic trace message";
        KAI_TRACE_WARN() << "This is a warning trace message";
        KAI_TRACE_ERROR() << "This is an error trace message";

        std::cout << "Test completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}