#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "KAI/Console/Console.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/Generate/Agent.h"
#include "KAI/Language/Tau/Generate/Proxy.h"
#include "KAI/Language/Tau/TauParser.h"

using namespace kai;
using namespace std;
namespace fs = std::filesystem;

// Test fixture for Chat Proxy/Agent generation from Tau
class ChatProxyGenerationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry_ = make_shared<Registry>();

        // Set up output directories
        outputDir_ = "/tmp/kai_chat_test";
        fs::create_directories(outputDir_);
        fs::create_directories(outputDir_ + "/proxy");
        fs::create_directories(outputDir_ + "/agent");
    }

    void TearDown() override {
        // Clean up generated files
        if (fs::exists(outputDir_)) {
            fs::remove_all(outputDir_);
        }
    }

    // Load Tau interface file
    string LoadTauInterface(const string& filename) {
        string path =
            "/home/xian/local/KAI/Test/Language/TestTau/Scripts/Connection/" +
            filename;
        ifstream file(path);
        if (!file.is_open()) {
            ADD_FAILURE() << "Failed to open Tau file: " << path;
            return "";
        }

        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Generate proxy code from Tau AST
    bool GenerateProxy(const tau::TauAstNode* ast, const string& outputFile) {
        tau::Generate::ProxyGenerator generator;
        return generator.Generate(ast, outputDir_ + "/proxy/" + outputFile);
    }

    // Generate agent code from Tau AST
    bool GenerateAgent(const tau::TauAstNode* ast, const string& outputFile) {
        tau::Generate::AgentGenerator generator;
        return generator.Generate(ast, outputDir_ + "/agent/" + outputFile);
    }

   protected:
    shared_ptr<Registry> registry_;
    string outputDir_;
};

// Test parsing and code generation for Chat interface
TEST_F(ChatProxyGenerationTest, GenerateChatProxyAndAgent) {
    // Load the Chat interface
    string tauCode = LoadTauInterface("ChatInterface.tau");
    ASSERT_FALSE(tauCode.empty());

    // Parse the Tau code
    tau::TauParser parser(*registry_);
    bool parseResult = parser.Parse(tauCode);
    ASSERT_TRUE(parseResult) << "Failed to parse Chat interface";

    auto ast = parser.GetAST();
    ASSERT_NE(ast, nullptr);

    // Generate proxy code
    bool proxyResult = GenerateProxy(ast, "ChatProxy.h");
    EXPECT_TRUE(proxyResult) << "Failed to generate proxy";

    // Generate agent code
    bool agentResult = GenerateAgent(ast, "ChatAgent.h");
    EXPECT_TRUE(agentResult) << "Failed to generate agent";

    // Verify generated files exist
    EXPECT_TRUE(fs::exists(outputDir_ + "/proxy/ChatProxy.h"));
    EXPECT_TRUE(fs::exists(outputDir_ + "/agent/ChatAgent.h"));
}

// Test console integration with generated code
TEST_F(ChatProxyGenerationTest, ConsoleIntegrationWithGeneratedCode) {
    // This test demonstrates how the generated proxy/agent would work with
    // Console

    // Create a mock console that would use the generated chat system
    auto console = make_shared<Console>(registry_.get());

    // Register chat-related console commands that would use generated code
    console->RegisterCommand("/chat", [](const vector<string>& args) {
        // In real implementation, this would:
        // 1. Get the ChatProxy instance
        // 2. Call proxy->SendMessage(message)
        // 3. Return status

        if (args.size() < 2) {
            return string("Usage: /chat <message>");
        }

        string message = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            message += " " + args[i];
        }

        // Mock: In reality would call generated proxy
        // auto proxy = GetChatProxy();
        // proxy->BroadcastToPeers(message);

        return "Message sent: " + message;
    });

    console->RegisterCommand("/connect", [](const vector<string>& args) {
        if (args.size() != 3) {
            return string("Usage: /connect <address> <port>");
        }

        // Mock: In reality would call generated proxy
        // auto proxy = GetChatProxy();
        // bool success = proxy->ConnectToPeer(args[1], stoi(args[2]));

        return "Connected to " + args[1] + ":" + args[2];
    });

    console->RegisterCommand("/discover", [](const vector<string>& args) {
        // Mock: In reality would call generated proxy
        // auto proxy = GetChatProxy();
        // proxy->StartDiscovery();

        return "Starting peer discovery...";
    });

    // Test the commands
    console->Execute("/discover");
    console->Execute("/connect 192.168.1.100 14589");
    console->Execute("/chat Hello, P2P world!");
}

// Test generated proxy structure
TEST_F(ChatProxyGenerationTest, VerifyGeneratedProxyStructure) {
    string tauCode = LoadTauInterface("ChatInterface.tau");
    ASSERT_FALSE(tauCode.empty());

    tau::TauParser parser(*registry_);
    ASSERT_TRUE(parser.Parse(tauCode));

    auto ast = parser.GetAST();
    ASSERT_TRUE(GenerateProxy(ast, "ChatProxy.h"));

    // Read generated proxy file
    ifstream proxyFile(outputDir_ + "/proxy/ChatProxy.h");
    ASSERT_TRUE(proxyFile.is_open());

    stringstream buffer;
    buffer << proxyFile.rdbuf();
    string proxyCode = buffer.str();

    // Verify key elements in generated proxy
    EXPECT_NE(proxyCode.find("class ChatPeerProxy"), string::npos)
        << "Generated proxy should contain ChatPeerProxy class";
    EXPECT_NE(proxyCode.find("SendMessage"), string::npos)
        << "Generated proxy should contain SendMessage method";
    EXPECT_NE(proxyCode.find("ConnectToPeer"), string::npos)
        << "Generated proxy should contain ConnectToPeer method";
    EXPECT_NE(proxyCode.find("RakNet"), string::npos)
        << "Generated proxy should include RakNet headers";
}

// Test generated agent structure
TEST_F(ChatProxyGenerationTest, VerifyGeneratedAgentStructure) {
    string tauCode = LoadTauInterface("ChatInterface.tau");
    ASSERT_FALSE(tauCode.empty());

    tau::TauParser parser(*registry_);
    ASSERT_TRUE(parser.Parse(tauCode));

    auto ast = parser.GetAST();
    ASSERT_TRUE(GenerateAgent(ast, "ChatAgent.h"));

    // Read generated agent file
    ifstream agentFile(outputDir_ + "/agent/ChatAgent.h");
    ASSERT_TRUE(agentFile.is_open());

    stringstream buffer;
    buffer << agentFile.rdbuf();
    string agentCode = buffer.str();

    // Verify key elements in generated agent
    EXPECT_NE(agentCode.find("class P2PAgent"), string::npos)
        << "Generated agent should contain P2PAgent class";
    EXPECT_NE(agentCode.find("RouteMessage"), string::npos)
        << "Generated agent should contain RouteMessage method";
    EXPECT_NE(agentCode.find("RegisterPeer"), string::npos)
        << "Generated agent should contain RegisterPeer method";
}

// Test Window integration concept
TEST_F(ChatProxyGenerationTest, WindowIntegrationConcept) {
    // This demonstrates how the generated code would integrate with Window

    struct MockChatWindow {
        // UI elements
        vector<string> chatHistory;
        vector<string> peerList;
        string inputField;

        // Methods that would use generated proxy/agent
        void SendMessage() {
            if (inputField.empty()) return;

            // Would call generated proxy
            // proxy->SendMessage(inputField);

            chatHistory.push_back("[You]: " + inputField);
            inputField.clear();
        }

        void OnMessageReceived(const string& sender, const string& message) {
            chatHistory.push_back("[" + sender + "]: " + message);
        }

        void UpdatePeerList(const vector<string>& peers) { peerList = peers; }

        void Render() {
            // Would render using ImGui or similar
            // - Chat history panel
            // - Peer list panel
            // - Input field
            // - Send button
        }
    };

    MockChatWindow window;

    // Simulate user interaction
    window.inputField = "Hello from Window UI";
    window.SendMessage();

    EXPECT_EQ(window.chatHistory.size(), 1);
    EXPECT_EQ(window.chatHistory[0], "[You]: Hello from Window UI");

    // Simulate receiving message
    window.OnMessageReceived("Peer1", "Hello back!");
    EXPECT_EQ(window.chatHistory.size(), 2);

    // Update peer list
    window.UpdatePeerList({"Peer1", "Peer2", "Peer3"});
    EXPECT_EQ(window.peerList.size(), 3);
}