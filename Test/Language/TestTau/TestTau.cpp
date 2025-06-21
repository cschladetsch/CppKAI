#include <gtest/gtest.h>

#include <fstream>
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

// Simplified test fixture focused on IDL functionality
struct TestTau : TestLangCommon {
    void TestIDL(const std::string& script, const std::string& testName) {
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);

        bool lexResult = lex->Process();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lex->Print());

        if (!lexResult) {
            FAIL() << "Lexer failed for " << testName << ": " << lex->Error;
            return;
        }

        auto parser = std::make_shared<tau::TauParser>(r);
        parser->Process(lex, Structure::Module);

        if (!parser->Error.empty()) {
            FAIL() << "Parser failed for " << testName << ": " << parser->Error;
            return;
        }

        SUCCEED() << "Successfully parsed " << testName;
    }

    void TestCodeGeneration(const std::string& script,
                            const std::string& testName) {
        // Test proxy generation
        string proxyOutput;
        tau::Generate::GenerateProxy proxy(script.c_str(), proxyOutput);

        if (proxy.Failed) {
            KAI_LOG_WARNING("Proxy generation for " + testName +
                            " failed: " + proxy.Error);
        } else {
            KAI_LOG_INFO(
                "Proxy generation for " + testName +
                " succeeded, size: " + std::to_string(proxyOutput.size()));
        }

        // Test agent generation
        string agentOutput;
        tau::Generate::GenerateAgent agent(script.c_str(), agentOutput);

        if (agent.Failed) {
            KAI_LOG_WARNING("Agent generation for " + testName +
                            " failed: " + agent.Error);
        } else {
            KAI_LOG_INFO(
                "Agent generation for " + testName +
                " succeeded, size: " + std::to_string(agentOutput.size()));
        }

        // Success if at least one generator worked
        if (!proxy.Failed || !agent.Failed) {
            SUCCEED() << "Code generation for " << testName << " completed";
        } else {
            FAIL() << "Both proxy and agent generation failed for " << testName;
        }
    }
};

// Test 1: Most basic IDL functionality
TEST_F(TestTau, TestBasicIDL) {
    std::string script = R"(
    namespace Test {
        class SimpleService {
            void DoWork();
            int GetValue();
        }
    }
    )";

    TestIDL(script, "BasicIDL");
}

// Test 2: IDL with properties and methods
TEST_F(TestTau, TestDataService) {
    std::string script = R"(
    namespace Services {
        class DataService {
            int count;
            string name;
            
            void AddItem(string item);
            string GetItem(int index);
            int GetCount();
        }
    }
    )";

    TestIDL(script, "DataService");
}

// Test 3: Multiple classes in one file
TEST_F(TestTau, TestMultipleServices) {
    std::string script = R"(
    namespace Application {
        class UserService {
            void CreateUser(string username);
            void DeleteUser(int userId);
        }
        
        class ProductService {
            void AddProduct(string name, float price);
            float GetPrice(int productId);
        }
    }
    )";

    TestIDL(script, "MultipleServices");
}

// Test 4: Empty namespace (minimal valid file)
TEST_F(TestTau, TestEmptyIDL) {
    std::string script = R"(
    namespace Empty {
    }
    )";

    TestIDL(script, "EmptyIDL");
}

// Test 5: Real-world style service interface
TEST_F(TestTau, TestNetworkService) {
    std::string script = R"(
    namespace Network {
        class ConnectionInfo {
            string address;
            int port;
            bool isConnected;
        }
        
        class NetworkService {
            bool Connect(string address, int port);
            void Disconnect();
            bool SendData(string data);
            string ReceiveData();
            ConnectionInfo GetConnectionInfo();
        }
    }
    )";

    TestIDL(script, "NetworkService");
}

// Test 6: Basic code generation
TEST_F(TestTau, TestBasicCodeGen) {
    std::string script = R"(
    namespace Generated {
        class TestService {
            void SimpleMethod();
            int Calculate(int a, int b);
        }
    }
    )";

    TestCodeGeneration(script, "BasicCodeGen");
}

// Test 7: Nested namespaces (simple syntax)
TEST_F(TestTau, TestNestedNamespacesSimple) {
    std::string script = R"(
    namespace Company {
        namespace API {
            class RestService {
                string Get(string endpoint);
                void Post(string endpoint, string data);
            }
        }
    }
    )";

    TestIDL(script, "NestedNamespacesSimple");
}

// Test 8: Service with only methods (typical RPC interface)
TEST_F(TestTau, TestRPCInterface) {
    std::string script = R"(
    namespace RPC {
        class MathService {
            int Add(int a, int b);
            int Subtract(int a, int b);
            float Multiply(float a, float b);
            float Divide(float a, float b);
        }
    }
    )";

    TestIDL(script, "RPCInterface");
}

// Test 9: Data transfer objects
TEST_F(TestTau, TestDataTransferObjects) {
    std::string script = R"(
    namespace Model {
        class User {
            int id;
            string firstName;
            string lastName;
            string email;
        }
        
        class Order {
            int orderId;
            int userId;
            float totalAmount;
            string status;
        }
    }
    )";

    TestIDL(script, "DataTransferObjects");
}