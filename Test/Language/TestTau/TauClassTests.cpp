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

// Simplified fixture for Tau IDL tests - focused on what an IDL actually needs
struct TauClassTests : TestLangCommon {
    void TestLexAndParse(const std::string& script,
                         const std::string& testName) {
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);

        // Process lexer
        bool lexerSuccess = lex->Process();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lex->Print());

        if (!lexerSuccess) {
            KAI_LOG_WARNING("Lexer for " + testName + " failed: " + lex->Error);
            FAIL() << "Lexer failed for " << testName << ": " << lex->Error;
            return;
        }

        // Parse as module
        auto parser = std::make_shared<tau::TauParser>(r);
        parser->Process(lex, Structure::Module);

        if (!parser->Error.empty()) {
            KAI_LOG_WARNING("Parser for " + testName +
                            " reported error: " + parser->Error);
            FAIL() << "Parser failed for " << testName << ": " << parser->Error;
            return;
        }

        SUCCEED() << "Successfully parsed " << testName;
    }
};

// Test 1: Basic IDL class - what Tau is actually designed for
TEST_F(TauClassTests, TestBasicIDLClass) {
    std::string script = R"(
    namespace Test {
        class BasicService {
            int id;
            string name;
            float value;
        }
    }
    )";

    TestLexAndParse(script, "BasicIDLClass");
}

// Test 2: IDL class with methods - core functionality
TEST_F(TauClassTests, TestIDLClassWithMethods) {
    std::string script = R"(
    namespace Test {
        class UserService {
            int userId;
            string userName;
            
            void CreateUser(string name);
            int GetUserId();
            string GetUserName();
            void UpdateUser(int id, string name);
            bool DeleteUser(int id);
        }
    }
    )";

    TestLexAndParse(script, "IDLClassWithMethods");
}

// Test 3: Simple service interface - typical IDL use case
TEST_F(TauClassTests, TestServiceInterface) {
    std::string script = R"(
    namespace Network {
        class DataService {
            void SendData(string data);
            string ReceiveData();
            bool IsConnected();
            void Connect(string host, int port);
            void Disconnect();
        }
    }
    )";

    TestLexAndParse(script, "ServiceInterface");
}

// Test 4: Data transfer object - another common IDL pattern
TEST_F(TauClassTests, TestDataTransferObject) {
    std::string script = R"(
    namespace Model {
        class UserData {
            int id;
            string firstName;
            string lastName;
            string email;
            bool active;
            
            string GetFullName();
            void SetEmail(string newEmail);
        }
    }
    )";

    TestLexAndParse(script, "DataTransferObject");
}

// Test 5: Multiple classes in namespace - realistic IDL file
TEST_F(TauClassTests, TestMultipleClasses) {
    std::string script = R"(
    namespace Application {
        class Request {
            int requestId;
            string method;
            string payload;
        }
        
        class Response {
            int statusCode;
            string message;
            string data;
        }
        
        class ServiceAPI {
            Response ProcessRequest(Request req);
            bool ValidateRequest(Request req);
        }
    }
    )";

    TestLexAndParse(script, "MultipleClasses");
}

// Test 6: Nested namespaces using simple syntax (not ::)
TEST_F(TauClassTests, TestSimpleNestedNamespaces) {
    std::string script = R"(
    namespace Company {
        namespace Services {
            class AuthService {
                bool Authenticate(string username, string password);
                void Logout(string token);
                bool IsValidToken(string token);
            }
        }
    }
    )";

    TestLexAndParse(script, "SimpleNestedNamespaces");
}

// Test 7: Empty class (minimal valid IDL)
TEST_F(TauClassTests, TestEmptyClass) {
    std::string script = R"(
    namespace Test {
        class EmptyClass {
        }
    }
    )";

    TestLexAndParse(script, "EmptyClass");
}

// Test 8: Class with only properties (data-only interface)
TEST_F(TauClassTests, TestDataOnlyClass) {
    std::string script = R"(
    namespace Data {
        class Configuration {
            string serverHost;
            int serverPort;
            int timeout;
            bool useSSL;
            string apiKey;
        }
    }
    )";

    TestLexAndParse(script, "DataOnlyClass");
}

// Test 9: Class with only methods (service-only interface)
TEST_F(TauClassTests, TestMethodOnlyClass) {
    std::string script = R"(
    namespace Services {
        class Calculator {
            int Add(int a, int b);
            int Subtract(int a, int b);
            float Multiply(float a, float b);
            float Divide(float a, float b);
            float SquareRoot(float value);
        }
    }
    )";

    TestLexAndParse(script, "MethodOnlyClass");
}

// Test 10: Realistic network service IDL
TEST_F(TauClassTests, TestRealisticNetworkService) {
    std::string script = R"(
    namespace KAI {
        namespace Network {
            class NodeInfo {
                string nodeId;
                string address;
                int port;
                bool isActive;
            }
            
            class NetworkManager {
                bool Connect(string address, int port);
                void Disconnect();
                bool SendMessage(string nodeId, string message);
                string ReceiveMessage();
                NodeInfo GetNodeInfo(string nodeId);
                bool RegisterNode(NodeInfo info);
            }
        }
    }
    )";

    TestLexAndParse(script, "RealisticNetworkService");
}