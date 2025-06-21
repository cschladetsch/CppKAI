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

// Simplified fixture for Tau namespace tests - focused on IDL needs
struct TauNamespaceTests : TestLangCommon {
    void TestLexAndParse(const std::string& script,
                         const std::string& testName) {
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);

        bool lexerSuccess = lex->Process();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lex->Print());

        if (!lexerSuccess) {
            KAI_LOG_WARNING("Lexer for " + testName + " failed: " + lex->Error);
            FAIL() << "Lexer failed for " << testName << ": " << lex->Error;
            return;
        }

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

// Test 1: Single namespace - basic IDL structure
TEST_F(TauNamespaceTests, TestSingleNamespace) {
    std::string script = R"(
    namespace Services {
        class UserService {
            void CreateUser(string name);
            bool DeleteUser(int id);
        }
    }
    )";

    TestLexAndParse(script, "SingleNamespace");
}

// Test 2: Empty namespace - minimal valid syntax
TEST_F(TauNamespaceTests, TestEmptyNamespace) {
    std::string script = R"(
    namespace Empty {
    }
    )";

    TestLexAndParse(script, "EmptyNamespace");
}

// Test 3: Multiple separate namespaces - common IDL pattern
TEST_F(TauNamespaceTests, TestMultipleSeparateNamespaces) {
    std::string script = R"(
    namespace Model {
        class User {
            int id;
            string name;
        }
    }
    
    namespace Service {
        class UserService {
            Model.User GetUser(int id);
            void SaveUser(Model.User user);
        }
    }
    )";

    TestLexAndParse(script, "MultipleSeparateNamespaces");
}

// Test 4: Nested namespaces using separate declarations (not ::)
TEST_F(TauNamespaceTests, TestNestedNamespacesSimple) {
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

    TestLexAndParse(script, "NestedNamespacesSimple");
}

// Test 5: Namespace with multiple classes - typical IDL organization
TEST_F(TauNamespaceTests, TestNamespaceWithMultipleClasses) {
    std::string script = R"(
    namespace DataModels {
        class Customer {
            int customerId;
            string name;
            string email;
        }
        
        class Order {
            int orderId;
            int customerId;
            float total;
        }
        
        class Product {
            int productId;
            string name;
            float price;
        }
    }
    )";

    TestLexAndParse(script, "NamespaceWithMultipleClasses");
}

// Test 6: Short namespace names - common in IDL
TEST_F(TauNamespaceTests, TestShortNamespaceNames) {
    std::string script = R"(
    namespace DB {
        class Connection {
            bool Connect(string connectionString);
            void Disconnect();
        }
    }
    
    namespace UI {
        class Window {
            void Show();
            void Hide();
        }
    }
    )";

    TestLexAndParse(script, "ShortNamespaceNames");
}

// Test 7: Namespace with service interfaces - core IDL use case
TEST_F(TauNamespaceTests, TestServiceInterfaces) {
    std::string script = R"(
    namespace NetworkServices {
        class AuthenticationService {
            string Login(string username, string password);
            void Logout(string token);
            bool ValidateToken(string token);
        }
        
        class DataService {
            string GetData(string key);
            void SetData(string key, string value);
            bool DeleteData(string key);
        }
    }
    )";

    TestLexAndParse(script, "ServiceInterfaces");
}

// Test 8: Simple flat namespace structure - easiest for IDL
TEST_F(TauNamespaceTests, TestFlatNamespaceStructure) {
    std::string script = R"(
    namespace MyApplication {
        class Config {
            string serverUrl;
            int timeout;
        }
        
        class Logger {
            void Log(string message);
            void SetLevel(int level);
        }
        
        class Database {
            bool Connect();
            void Disconnect();
            string Query(string sql);
        }
    }
    )";

    TestLexAndParse(script, "FlatNamespaceStructure");
}

// Test 9: Namespace for RPC-style interfaces
TEST_F(TauNamespaceTests, TestRPCInterfaces) {
    std::string script = R"(
    namespace RPC {
        class CalculatorService {
            int Add(int a, int b);
            int Subtract(int a, int b);
            float Divide(float a, float b);
            float Multiply(float a, float b);
        }
        
        class StringService {
            string Concat(string a, string b);
            int Length(string s);
            string Substring(string s, int start, int length);
        }
    }
    )";

    TestLexAndParse(script, "RPCInterfaces");
}

// Test 10: Real-world style IDL namespace
TEST_F(TauNamespaceTests, TestRealWorldIDL) {
    std::string script = R"(
    namespace GameServer {
        class Player {
            int playerId;
            string playerName;
            int level;
            float health;
        }
        
        class GameSession {
            int sessionId;
            string mapName;
            int maxPlayers;
        }
        
        class GameService {
            Player CreatePlayer(string name);
            bool JoinSession(int playerId, int sessionId);
            void LeaveSession(int playerId);
            GameSession CreateSession(string mapName);
            Player GetPlayer(int playerId);
        }
    }
    )";

    TestLexAndParse(script, "RealWorldIDL");
}