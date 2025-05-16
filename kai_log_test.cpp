#include <iostream>
#include <string>

// Mock the KAI_NAMESPACE macro
#define KAI_NAMESPACE(X) X

// Mock FileLocation class
class FileLocation {
public:
    std::string file;
    std::string function;
    int line;

    FileLocation(const char* F, int L, const char* G = "")
        : file(F), line(L), function(G) {}

    void AddLocation(std::ostream& str) const {
        // Extract filename from path
        std::string loc = file;
        size_t lastSlash = loc.find_last_of('/');
        if (lastSlash != std::string::npos) {
            loc = loc.substr(lastSlash + 1);
        }

        str << "[" << loc << ":" << line << "] ";
    }
};

// Mock Logger class with location-aware methods
namespace Logger {
    void InfoWithLocation(const std::string& message, const char* file, int line) {
        FileLocation loc(file, line);
        std::cout << "[INFO] ";
        loc.AddLocation(std::cout);
        std::cout << message << std::endl;
    }

    void WarningWithLocation(const std::string& message, const char* file, int line) {
        FileLocation loc(file, line);
        std::cout << "[WARNING] ";
        loc.AddLocation(std::cout);
        std::cout << message << std::endl;
    }

    void ErrorWithLocation(const std::string& message, const char* file, int line) {
        FileLocation loc(file, line);
        std::cout << "[ERROR] ";
        loc.AddLocation(std::cout);
        std::cout << message << std::endl;
    }
}

// Mock debug::Trace class
namespace debug {
    enum Type { Information, Warn, Error, Fatal };
    
    class Trace {
    public:
        FileLocation file_location;
        Type type;
        
        Trace(FileLocation const &F, Type T = Information)
            : file_location(F), type(T) {}
            
        ~Trace() {
            // Simulate output at destruction
            std::string typeStr;
            switch (type) {
                case Information: typeStr = "INFO"; break;
                case Warn: typeStr = "WARN"; break;
                case Error: typeStr = "ERROR"; break;
                case Fatal: typeStr = "FATAL"; break;
            }
            
            std::cout << "[" << typeStr << " TRACE] ";
            file_location.AddLocation(std::cout);
            if (!file_location.function.empty()) {
                std::cout << file_location.function << ": ";
            }
            std::cout << buffer << std::endl;
        }
        
        template <class T>
        Trace& operator<<(const T& value) {
            buffer += std::to_string(value);
            return *this;
        }
        
        Trace& operator<<(const std::string& value) {
            buffer += value;
            return *this;
        }
        
        Trace& operator<<(const char* value) {
            buffer += value;
            return *this;
        }
        
    private:
        std::string buffer;
    };
}

// Define macros similar to those in KAI Debug.h
#define KAI_LOG_INFO(msg) KAI_NAMESPACE(Logger::InfoWithLocation)(msg, __FILE__, __LINE__)
#define KAI_LOG_WARNING(msg) KAI_NAMESPACE(Logger::WarningWithLocation)(msg, __FILE__, __LINE__)
#define KAI_LOG_ERROR(msg) KAI_NAMESPACE(Logger::ErrorWithLocation)(msg, __FILE__, __LINE__)

#define KAI_TRACER(F, L, N, T) \
    KAI_NAMESPACE(debug::Trace)(FileLocation(F, L, N), KAI_NAMESPACE(debug::T))

#define KAI_TRACE() KAI_TRACER(__FILE__, __LINE__, __FUNCTION__, Information)
#define KAI_TRACE_WARN() KAI_TRACER(__FILE__, __LINE__, __FUNCTION__, Warn)
#define KAI_TRACE_ERROR() KAI_TRACER(__FILE__, __LINE__, __FUNCTION__, Error)
#define KAI_TRACE_FATAL() KAI_TRACER(__FILE__, __LINE__, __FUNCTION__, Fatal)

// Test function to demonstrate trace with different file/line
void testFunction() {
    KAI_LOG_INFO("Info message from testFunction");
    KAI_TRACE() << "Trace message from testFunction";
}

int main() {
    std::cout << "Testing KAI logging macros with file and line information...\n" << std::endl;
    
    // Test Logger macros
    KAI_LOG_INFO("This is an info message");
    KAI_LOG_WARNING("This is a warning message");
    KAI_LOG_ERROR("This is an error message");
    
    std::cout << "\nTesting KAI trace macros with file and line information...\n" << std::endl;
    
    // Test Trace macros
    KAI_TRACE() << "This is a trace message";
    KAI_TRACE_WARN() << "This is a warning trace";
    KAI_TRACE_ERROR() << "This is an error trace";
    KAI_TRACE_FATAL() << "This is a fatal trace";
    
    std::cout << "\nTesting from different function...\n" << std::endl;
    
    // Call function to see different file/line
    testFunction();
    
    return 0;
}