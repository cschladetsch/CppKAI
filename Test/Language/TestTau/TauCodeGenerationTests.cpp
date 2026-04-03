#include <gtest/gtest.h>

#include <fstream>
#include <regex>
#include <sstream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProcess.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing simplified Tau IDL code generation
struct TauCodeGenerationTests : TestLangCommon {
    // Helper method to test proxy code generation
    void TestProxyGeneration(const std::string& script,
                             const std::string& testName) {
        try {
            // Generate proxy code
            string output;
            tau::Generate::GenerateProxy proxy(script.c_str(), output);

            // Check if generation was successful
            if (proxy.Failed) {
                KAI_LOG_WARNING("Proxy generation for " + testName +
                                " failed: " + proxy.Error);
                FAIL() << "Proxy generation failed: " << proxy.Error;
            }

            KAI_LOG_INFO(
                "Proxy generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));

            // Debug: print the actual output
            KAI_LOG_INFO("Generated proxy output:\n" + output);

            // Verify the generated code contains expected elements
            ASSERT_FALSE(output.empty()) << "Generated proxy code is empty";

            // Using regex to check for minimal expected C++ code structure
            std::regex classRegex("class.*[\\s\\S]*?\\{");
            std::regex methodRegex("\\s+.*\\(.*\\)");
            std::regex namespaceRegex("namespace.*[\\s\\S]*?\\{");

            EXPECT_TRUE(std::regex_search(output, namespaceRegex))
                << "Generated code doesn't contain namespace declarations";
            EXPECT_TRUE(std::regex_search(output, classRegex))
                << "Generated code doesn't contain class declarations";
            EXPECT_TRUE(std::regex_search(output, methodRegex))
                << "Generated code doesn't contain method declarations";

        } catch (const std::exception& e) {
            FAIL() << "Exception during proxy generation test: " << e.what();
        }
    }

    // Helper method to test agent code generation
    void TestAgentGeneration(const std::string& script,
                             const std::string& testName) {
        try {
            // Generate agent code
            string output;
            tau::Generate::GenerateAgent agent(script.c_str(), output);

            // Check if generation was successful
            if (agent.Failed) {
                KAI_LOG_WARNING("Agent generation for " + testName +
                                " failed: " + agent.Error);
                FAIL() << "Agent generation failed: " << agent.Error;
            }

            KAI_LOG_INFO(
                "Agent generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));

            // Verify the generated code contains expected elements
            ASSERT_FALSE(output.empty()) << "Generated agent code is empty";

            // Using regex to check for minimal expected C++ code structure
            std::regex classRegex("class.*[\\s\\S]*?\\{");
            std::regex methodRegex("\\s+.*\\(.*\\)");
            std::regex namespaceRegex("namespace.*[\\s\\S]*?\\{");

            EXPECT_TRUE(std::regex_search(output, namespaceRegex))
                << "Generated code doesn't contain namespace declarations";
            EXPECT_TRUE(std::regex_search(output, classRegex))
                << "Generated code doesn't contain class declarations";
            EXPECT_TRUE(std::regex_search(output, methodRegex))
                << "Generated code doesn't contain method declarations";

        } catch (const std::exception& e) {
            FAIL() << "Exception during agent generation test: " << e.what();
        }
    }
};

// Test basic interface proxy generation
TEST_F(TauCodeGenerationTests, BasicProxyGeneration) {
    std::string script = R"(
    namespace Test {
        interface ISimpleInterface {
            int Add(int a, int b);
            float Multiply(float x, float y);
            string Concatenate(string a, string b);
        }
    }
    )";

    TestProxyGeneration(script, "BasicInterface");
}

// Test interface with events proxy generation
TEST_F(TauCodeGenerationTests, EventProxyGeneration) {
    std::string script = R"(
    namespace Test {
        interface IEventInterface {
            int PerformOperation(int value);
            void RegisterHandler(object handler);
            
            event OperationCompleted(int result);
            event OperationFailed(string error);
        }
    }
    )";

    TestProxyGeneration(script, "EventInterface");
}

// Test interface with structs proxy generation
TEST_F(TauCodeGenerationTests, StructProxyGeneration) {
    std::string script = R"(
    namespace Test {
        struct Point {
            float x;
            float y;
            float z;
        }
        
        struct Color {
            int r;
            int g;
            int b;
            int a;
        }
        
        interface IGraphicsInterface {
            void DrawPoint(Point position, Color color);
            Point[] GetPoints();
            void ClearPoints();
        }
    }
    )";

    TestProxyGeneration(script, "StructInterface");
}

// Test agent generation for basic interface
TEST_F(TauCodeGenerationTests, BasicAgentGeneration) {
    std::string script = R"(
    namespace Test {
        interface ISimpleInterface {
            int Add(int a, int b);
            float Multiply(float x, float y);
            string Concatenate(string a, string b);
        }
    }
    )";

    TestAgentGeneration(script, "BasicInterface");
}

// Test agent generation for interface with events
TEST_F(TauCodeGenerationTests, EventAgentGeneration) {
    std::string script = R"(
    namespace Test {
        interface IEventInterface {
            int PerformOperation(int value);
            void RegisterHandler(object handler);
            
            event OperationCompleted(int result);
            event OperationFailed(string error);
        }
    }
    )";

    TestAgentGeneration(script, "EventInterface");
}

// Test proxy generation with simple namespace
TEST_F(TauCodeGenerationTests, NamespaceProxyGeneration) {
    std::string script = R"(
    namespace Network {
        interface IProtocolHandler {
            void SendMessage(string message);
            string ReceiveMessage();
            bool IsConnected();
        }
    }
    )";

    TestProxyGeneration(script, "SimpleNamespace");
}

// Test generation with array parameters
TEST_F(TauCodeGenerationTests, ArrayParametersGeneration) {
    std::string script = R"(
    namespace Test {
        interface IArrayInterface {
            int Sum(int[] values);
            float Average(float[] values);
            string Join(string[] parts, string separator);
        }
    }
    )";

    TestProxyGeneration(script, "ArrayParameters");
    TestAgentGeneration(script, "ArrayParameters");
}

// Test generation with enum parameters
TEST_F(TauCodeGenerationTests, EnumParametersGeneration) {
    std::string script = R"(
    namespace Test {
        enum LogLevel {
            Debug = 0,
            Info = 1,
            Warning = 2,
            Error = 3
        }
        
        interface ILogInterface {
            void Log(string message, LogLevel level);
            LogLevel GetLogLevel();
            void SetLogLevel(LogLevel level);
        }
    }
    )";

    TestProxyGeneration(script, "EnumParameters");
    TestAgentGeneration(script, "EnumParameters");
}

// Test generation with multiple interfaces
TEST_F(TauCodeGenerationTests, MultipleInterfacesGeneration) {
    std::string script = R"(
    namespace Test {
        interface ILogger {
            void Log(string message);
            void LogError(string error);
        }
        
        interface IConfig {
            string GetSetting(string key);
            void SetSetting(string key, string value);
        }
        
        interface INetwork {
            bool Connect(string host, int port);
            void Disconnect();
        }
    }
    )";

    TestProxyGeneration(script, "MultipleInterfaces");
    TestAgentGeneration(script, "MultipleInterfaces");
}

// Test generation with void return types
TEST_F(TauCodeGenerationTests, VoidReturnTypeGeneration) {
    std::string script = R"(
    namespace Test {
        interface IVoidInterface {
            void Initialize();
            void Process(int data);
            void Cleanup();
            void Log(string message);
        }
    }
    )";

    TestProxyGeneration(script, "VoidReturnType");
    TestAgentGeneration(script, "VoidReturnType");
}