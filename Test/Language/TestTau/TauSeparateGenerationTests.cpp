#include <KAI/Language/Tau/Generate/GenerateAgent.h>
#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/Generate/GenerateStruct.h>

#include <regex>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

TAU_BEGIN

struct TauSeparateGenerationTests : TestLangCommon {};

TEST_F(TauSeparateGenerationTests, TestProxyOnlyGeneratesProxy) {
    string input = R"(
    namespace Test {
        class Service {
            void DoWork();
            int Calculate(int a, int b);
        }
    }
    )";

    string output;
    Generate::GenerateProxy generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should contain proxy class
    EXPECT_TRUE(output.find("class ServiceProxy: public ProxyBase") !=
                string::npos);

    // Should NOT contain agent class
    EXPECT_TRUE(output.find("class ServiceAgent") == string::npos);
    EXPECT_TRUE(output.find("AgentBase") == string::npos);

    // Should contain proxy methods
    EXPECT_TRUE(output.find("DoWork()") != string::npos);
    EXPECT_TRUE(output.find("Calculate(") != string::npos);

    // Should NOT contain handler methods
    EXPECT_TRUE(output.find("Handle_DoWork") == string::npos);
    EXPECT_TRUE(output.find("Handle_Calculate") == string::npos);
}

TEST_F(TauSeparateGenerationTests, TestAgentOnlyGeneratesAgent) {
    string input = R"(
    namespace Test {
        class Service {
            void DoWork();
            int Calculate(int a, int b);
        }
    }
    )";

    string output;
    Generate::GenerateAgent generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should contain agent class
    EXPECT_TRUE(output.find("class ServiceAgent: public AgentBase<Service>") !=
                string::npos);

    // Should NOT contain proxy class
    EXPECT_TRUE(output.find("class ServiceProxy") == string::npos);
    EXPECT_TRUE(output.find("ProxyBase") == string::npos);

    // Should contain handler methods
    EXPECT_TRUE(output.find("Handle_DoWork") != string::npos);
    EXPECT_TRUE(output.find("Handle_Calculate") != string::npos);

    // Should NOT contain proxy methods like Fetch or Store
    EXPECT_TRUE(output.find("Fetch<") == string::npos);
    EXPECT_TRUE(output.find("Store(") == string::npos);
}

TEST_F(TauSeparateGenerationTests, TestProxyHandlesInterfaces) {
    string input = R"(
    namespace Test {
        interface ILogger {
            void Log(string message);
            void LogError(string error);
        }
    }
    )";

    string output;
    Generate::GenerateProxy generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should generate proxy for interface
    EXPECT_TRUE(output.find("class ILoggerProxy: public ProxyBase") !=
                string::npos);
    EXPECT_TRUE(output.find("Log(") != string::npos);
    EXPECT_TRUE(output.find("LogError(") != string::npos);
}

TEST_F(TauSeparateGenerationTests, TestAgentHandlesInterfaces) {
    string input = R"(
    namespace Test {
        interface ILogger {
            void Log(string message);
            void LogError(string error);
        }
    }
    )";

    string output;
    Generate::GenerateAgent generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should generate agent for interface
    EXPECT_TRUE(output.find("class ILoggerAgent: public AgentBase<ILogger>") !=
                string::npos);
    EXPECT_TRUE(output.find("Handle_Log") != string::npos);
    EXPECT_TRUE(output.find("Handle_LogError") != string::npos);
}

TEST_F(TauSeparateGenerationTests, TestProxyIgnoresStructs) {
    string input = R"(
    namespace Test {
        struct Point {
            int x;
            int y;
        }
        
        class Service {
            Point GetPoint();
        }
    }
    )";

    string output;
    Generate::GenerateProxy generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should generate proxy for class
    EXPECT_TRUE(output.find("class ServiceProxy") != string::npos);

    // Should NOT generate proxy for struct
    EXPECT_TRUE(output.find("class PointProxy") == string::npos);
}

TEST_F(TauSeparateGenerationTests, TestAgentIgnoresStructs) {
    string input = R"(
    namespace Test {
        struct Point {
            int x;
            int y;
        }
        
        class Service {
            Point GetPoint();
        }
    }
    )";

    string output;
    Generate::GenerateAgent generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should generate agent for class
    EXPECT_TRUE(output.find("class ServiceAgent") != string::npos);

    // Should NOT generate agent for struct
    EXPECT_TRUE(output.find("class PointAgent") == string::npos);
}

TEST_F(TauSeparateGenerationTests, TestProxyWithEvents) {
    string input = R"(
    namespace Test {
        class EventService {
            event OnDataReceived(string data);
            event OnError(int code, string message);
        }
    }
    )";

    string output;
    Generate::GenerateProxy generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should generate event registration methods
    EXPECT_TRUE(output.find("RegisterOnDataReceivedHandler") != string::npos);
    EXPECT_TRUE(output.find("UnregisterOnDataReceivedHandler") != string::npos);
    EXPECT_TRUE(output.find("RegisterOnErrorHandler") != string::npos);
    EXPECT_TRUE(output.find("UnregisterOnErrorHandler") != string::npos);
}

TEST_F(TauSeparateGenerationTests, TestComplexScenario) {
    string input = R"(
    namespace Application {
        struct UserData {
            int id;
            string name;
        }
        
        interface IUserService {
            UserData GetUser(int id);
            void UpdateUser(UserData user);
        }
        
        class UserManager {
            void AddUser(UserData user);
            UserData FindUser(string name);
            event OnUserAdded(UserData user);
        }
    }
    )";

    // Test proxy generation
    {
        string proxyOutput;
        Generate::GenerateProxy generator(input.c_str(), proxyOutput);

        ASSERT_FALSE(proxyOutput.empty());

        // Should have proxies for interface and class
        EXPECT_TRUE(proxyOutput.find("class IUserServiceProxy") !=
                    string::npos);
        EXPECT_TRUE(proxyOutput.find("class UserManagerProxy") != string::npos);

        // Should NOT have proxy for struct
        EXPECT_TRUE(proxyOutput.find("class UserDataProxy") == string::npos);

        // Should have event handlers
        EXPECT_TRUE(proxyOutput.find("RegisterOnUserAddedHandler") !=
                    string::npos);
    }

    // Test agent generation
    {
        string agentOutput;
        Generate::GenerateAgent generator(input.c_str(), agentOutput);

        ASSERT_FALSE(agentOutput.empty());

        // Should have agents for interface and class
        EXPECT_TRUE(agentOutput.find("class IUserServiceAgent") !=
                    string::npos);
        EXPECT_TRUE(agentOutput.find("class UserManagerAgent") != string::npos);

        // Should NOT have agent for struct
        EXPECT_TRUE(agentOutput.find("class UserDataAgent") == string::npos);

        // Should have handler methods
        EXPECT_TRUE(agentOutput.find("Handle_GetUser") != string::npos);
        EXPECT_TRUE(agentOutput.find("Handle_UpdateUser") != string::npos);
        EXPECT_TRUE(agentOutput.find("Handle_AddUser") != string::npos);
        EXPECT_TRUE(agentOutput.find("Handle_FindUser") != string::npos);
    }

    // Test struct generation
    {
        string structOutput;
        Generate::GenerateStruct generator(input.c_str(), structOutput);

        ASSERT_FALSE(structOutput.empty());

        // Should have struct definition
        EXPECT_TRUE(structOutput.find("struct UserData") != string::npos);
        EXPECT_TRUE(structOutput.find("int id;") != string::npos);
        EXPECT_TRUE(structOutput.find("string name;") != string::npos);

        // Should treat class as struct (but not interface)
        EXPECT_TRUE(structOutput.find("struct UserManager") != string::npos);
        EXPECT_TRUE(structOutput.find("IUserService") == string::npos);
    }
}

TAU_END