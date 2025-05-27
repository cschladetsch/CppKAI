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

// Fixture for simplified Tau IDL tests
struct TauAdvancedTypeTests : TestLangCommon {
    // Helper method to run a Tau test script
    void RunTauTest(const std::string& script, const std::string& testName,
                    bool expectSuccess = true) {
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);
        bool lexResult = lex->Process();

        // Debug the lexer output
        std::string lexerOutput = lex->Print();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lexerOutput);

        if (!lexResult) {
            KAI_LOG_ERROR("Lexer for " + testName +
                          " failed. Check the lexer output above for details.");
            if (!expectSuccess) {
                SUCCEED() << "Lexer failed as expected for test: " << testName;
                return;
            }
        }

        if (expectSuccess) {
            ASSERT_TRUE(lexResult) << "Lexer for " << testName
                                   << " failed with output: " << lexerOutput;
        }

        auto parser = std::make_shared<tau::TauParser>(r);
        bool success = parser->Process(lex, Structure::Module);

        if (!success) {
            KAI_LOG_WARNING("Parser for " + testName +
                            " reported failure: " + parser->Error);
            if (!expectSuccess) {
                SUCCEED() << "Parser failed as expected for test: " << testName;
                return;
            }
        }

        if (expectSuccess) {
            EXPECT_TRUE(success)
                << "Parser for " << testName << " failed: " << parser->Error;
        } else {
            SUCCEED() << "Test completed for: " << testName
                      << " (known limitation)";
        }
    }

    // Helper method to test proxy code generation
    void TestProxyGen(const std::string& script, const std::string& testName,
                      bool expectSuccess = true) {
        string output;
        tau::Generate::GenerateProxy proxy(script.c_str(), output);

        if (proxy.Failed) {
            KAI_LOG_WARNING("Proxy generation for " + testName +
                            " reported failure: " + proxy.Error);
            if (!expectSuccess) {
                SUCCEED() << "Proxy generation failed as expected for test: "
                          << testName;
                return;
            }
        } else {
            KAI_LOG_INFO(
                "Proxy generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));
        }

        if (expectSuccess) {
            EXPECT_FALSE(proxy.Failed) << "Proxy generation for " << testName
                                       << " failed: " << proxy.Error;
            EXPECT_FALSE(output.empty()) << "Proxy generation for " << testName
                                         << " produced empty output";
        } else {
            SUCCEED() << "Proxy generation test completed for: " << testName;
        }
    }

    // Helper method to test agent code generation
    void TestAgentGen(const std::string& script, const std::string& testName,
                      bool expectSuccess = true) {
        string output;
        tau::Generate::GenerateAgent agent(script.c_str(), output);

        if (agent.Failed) {
            KAI_LOG_WARNING("Agent generation for " + testName +
                            " reported failure: " + agent.Error);
            if (!expectSuccess) {
                SUCCEED() << "Agent generation failed as expected for test: "
                          << testName;
                return;
            }
        } else {
            KAI_LOG_INFO(
                "Agent generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));
        }

        if (expectSuccess) {
            EXPECT_FALSE(agent.Failed) << "Agent generation for " << testName
                                       << " failed: " << agent.Error;
            EXPECT_FALSE(output.empty()) << "Agent generation for " << testName
                                         << " produced empty output";
        } else {
            SUCCEED() << "Agent generation test completed for: " << testName;
        }
    }
};

// Test simple interface definition
TEST_F(TauAdvancedTypeTests, TestSimpleInterface) {
    std::string script = R"(
    namespace Network {
        interface IConnection {
            bool Connect(string host, int port);
            void Disconnect();
            bool IsConnected();
            string GetStatus();
        }
    }
    )";

    RunTauTest(script, "SimpleInterface");
    TestProxyGen(script, "SimpleInterface");
    TestAgentGen(script, "SimpleInterface");
}

// Test interface with events
TEST_F(TauAdvancedTypeTests, TestInterfaceWithEvents) {
    std::string script = R"(
    namespace Network {
        interface IEventHandler {
            event ConnectionEstablished(string host, int port);
            event ConnectionLost(string reason);
            event DataReceived(int bytes);
            
            void RegisterHandler(object handler);
            void UnregisterHandler(object handler);
        }
    }
    )";

    RunTauTest(script, "InterfaceWithEvents");
    TestProxyGen(script, "InterfaceWithEvents");
    TestAgentGen(script, "InterfaceWithEvents");
}

// Test struct with arrays
TEST_F(TauAdvancedTypeTests, TestStructWithArrays) {
    std::string script = R"(
    namespace Data {
        struct DataPacket {
            string id;
            int[] values;
            string[] names;
            float timestamp;
        }
        
        interface IDataHandler {
            void ProcessPacket(DataPacket packet);
            DataPacket[] GetAllPackets();
        }
    }
    )";

    RunTauTest(script, "StructWithArrays");
    TestProxyGen(script, "StructWithArrays");
    TestAgentGen(script, "StructWithArrays");
}

// Test enum definition
TEST_F(TauAdvancedTypeTests, TestEnumDefinition) {
    std::string script = R"(
    namespace Types {
        enum Status {
            Idle = 0,
            Running = 1,
            Stopped = 2,
            Error = 3
        }
        
        interface IStatusReporter {
            Status GetStatus();
            void SetStatus(Status newStatus);
        }
    }
    )";

    RunTauTest(script, "EnumDefinition");
    TestProxyGen(script, "EnumDefinition");
    TestAgentGen(script, "EnumDefinition");
}

// Test nested structs
TEST_F(TauAdvancedTypeTests, TestNestedStructs) {
    std::string script = R"(
    namespace Data {
        struct Address {
            string street;
            string city;
            int zip;
        }
        
        struct Person {
            string name;
            int age;
            Address address;
        }
        
        interface IPersonService {
            Person GetPerson(string id);
            void UpdatePerson(Person person);
        }
    }
    )";

    RunTauTest(script, "NestedStructs");
    TestProxyGen(script, "NestedStructs");
    TestAgentGen(script, "NestedStructs");
}

// Test multiple interfaces
TEST_F(TauAdvancedTypeTests, TestMultipleInterfaces) {
    std::string script = R"(
    namespace Services {
        interface ILogger {
            void Log(string message);
            void SetLevel(int level);
        }
        
        interface IDatabase {
            void Store(string key, string value);
            string Retrieve(string key);
            bool Contains(string key);
        }
        
        interface IApplication {
            void Initialize();
            void Run();
            void Shutdown();
        }
    }
    )";

    RunTauTest(script, "MultipleInterfaces");
    TestProxyGen(script, "MultipleInterfaces");
    TestAgentGen(script, "MultipleInterfaces");
}

// Test interface with various parameter types
TEST_F(TauAdvancedTypeTests, TestVariousParameterTypes) {
    std::string script = R"(
    namespace Types {
        interface ITypeTest {
            void TestBool(bool value);
            void TestInt(int value);
            void TestFloat(float value);
            void TestString(string value);
            void TestMultiple(bool b, int i, float f, string s);
            int[] GetIntArray();
            void SetStringArray(string[] values);
        }
    }
    )";

    RunTauTest(script, "VariousParameterTypes");
    TestProxyGen(script, "VariousParameterTypes");
    TestAgentGen(script, "VariousParameterTypes");
}

// Test empty interface (should be valid for IDL)
TEST_F(TauAdvancedTypeTests, TestEmptyInterface) {
    std::string script = R"(
    namespace Test {
        interface IEmpty {
        }
    }
    )";

    RunTauTest(script, "EmptyInterface");
    TestProxyGen(script, "EmptyInterface");
    TestAgentGen(script, "EmptyInterface");
}

// Test struct with default values
TEST_F(TauAdvancedTypeTests, TestStructDefaults) {
    std::string script = R"(
    namespace Config {
        struct Settings {
            int timeout = 30;
            bool autoConnect = true;
            string defaultHost = "localhost";
            int maxRetries = 3;
        }
        
        interface IConfigManager {
            Settings GetSettings();
            void UpdateSettings(Settings settings);
        }
    }
    )";

    RunTauTest(script, "StructDefaults");
    TestProxyGen(script, "StructDefaults");
    TestAgentGen(script, "StructDefaults");
}

// Test simple service definition
TEST_F(TauAdvancedTypeTests, TestSimpleService) {
    std::string script = R"(
    namespace Services {
        struct Request {
            string id;
            string method;
            string data;
        }
        
        struct Response {
            string id;
            bool success;
            string result;
            string error;
        }
        
        interface IService {
            Response ProcessRequest(Request request);
            void RegisterCallback(string event, object callback);
            event RequestProcessed(Request request, Response response);
        }
    }
    )";

    RunTauTest(script, "SimpleService");
    TestProxyGen(script, "SimpleService");
    TestAgentGen(script, "SimpleService");
}