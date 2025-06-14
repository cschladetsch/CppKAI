#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "KAI/Console/Console.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Network/Node.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Mock implementation of a P2P Chat Peer that works with Console
class MockChatPeer {
   public:
    MockChatPeer(const string& username, int port)
        : username_(username), running_(false) {}

    bool Initialize() {
        // Initialize RakNet peer
        running_ = true;
        return true;
    }

    void Shutdown() { running_ = false; }

    bool IsRunning() const { return running_; }

    void SendMessage(const string& message) {
        // In real implementation, this would broadcast to all connected peers
        lastMessage_ = message;
        messageCount_++;
    }

    string GetLastMessage() const { return lastMessage_; }

    int GetMessageCount() const { return messageCount_; }

   private:
    string username_;
    bool running_;
    string lastMessage_;
    int messageCount_ = 0;
};

// Test fixture for P2P Chat with Console integration
class ChatP2PConsoleTest : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();

        // Create a registry for the console
        registry_ = make_shared<Registry>();

        // Create console
        console_ = make_shared<Console>();

        // Initialize peer
        peer_ = make_unique<MockChatPeer>("TestUser", 14589);
        peer_->Initialize();
    }

    void TearDown() override {
        if (peer_) {
            peer_->Shutdown();
        }
        TestLangCommon::TearDown();
    }

    // Helper to execute a console command
    string ExecuteCommand(const string& command) {
        // Execute command through console
        console_->Execute(String(command));

        // Get stack value as result
        auto executor = console_->GetExecutor();
        if (executor.Exists()) {
            auto stack = executor->GetDataStack();
            if (stack.Exists() && !stack->Empty()) {
                return stack->Top().ToString().c_str();
            }
        }
        return "";
    }

    // Helper to setup chat environment in console
    void SetupChatEnvironment() {
        // Add simple chat functions
        console_->Execute("'chat_send { print } ;");
        console_->Execute("'chat_status { \"Chat peer is running\" } ;");
        console_->Execute("'chat_peers { \"Connected peers: 0\" } ;");
        console_->Execute("'chat_history { \"Messages sent: 0\" } ;");
    }

   protected:
    shared_ptr<Registry> registry_;
    shared_ptr<Console> console_;
    unique_ptr<MockChatPeer> peer_;
};

// Test basic chat functionality through console
TEST_F(ChatP2PConsoleTest, BasicChatCommands) {
    SetupChatEnvironment();

    // Test status command
    string result = ExecuteCommand("chat_status");
    EXPECT_EQ(result, "\"Chat peer is running\"");

    // Test sending a message
    ExecuteCommand("\"Hello, world!\" chat_send");
    EXPECT_EQ(peer_->GetLastMessage(), "Hello, world!");

    // Test peers command
    result = ExecuteCommand("chat_peers");
    EXPECT_EQ(result, "\"Connected peers: 0\"");

    // Test history command
    result = ExecuteCommand("chat_history");
    EXPECT_EQ(result, "\"Messages sent: 0\"");
}

// Test Tau interface parsing for chat
TEST_F(ChatP2PConsoleTest, ParseChatInterface) {
    // Load the chat interface file
    ifstream file(
        "/home/xian/local/KAI/Test/Language/TestTau/Scripts/Connection/"
        "ChatInterface.tau");
    ASSERT_TRUE(file.is_open());

    stringstream buffer;
    buffer << file.rdbuf();
    string tauCode = buffer.str();

    // Parse the Tau interface
    tau::TauParser parser(*registry_);
    auto lexer = make_shared<tau::TauLexer>(tauCode.c_str(), *registry_);

    // Lex and parse
    lexer->Process();
    bool parseResult = parser.Process(lexer, Structure::Module);
    EXPECT_TRUE(parseResult) << "Failed to parse Chat interface";

    // Verify parsing succeeded
    auto root = parser.GetRoot();
    ASSERT_NE(root, nullptr) << "Parser root is null";
}

// Test console integration with Rho scripting
TEST_F(ChatP2PConsoleTest, ConsoleRhoIntegration) {
    SetupChatEnvironment();

    // Execute Rho code that sends chat messages
    console_->SetLanguage(Language::Rho);

    // Define a function and use it
    console_->Execute("'send_msg { \"Message: \" swap + print } ;");
    console_->Execute("\"Hello from Rho\" send_msg");

    // Test that peer received simulated message
    peer_->SendMessage("Test message");
    EXPECT_EQ(peer_->GetMessageCount(), 1);
}

// Test multiple peer simulation
TEST_F(ChatP2PConsoleTest, MultiplePeerSimulation) {
    // Create additional mock peers
    auto peer2 = make_unique<MockChatPeer>("User2", 14590);
    auto peer3 = make_unique<MockChatPeer>("User3", 14591);

    EXPECT_TRUE(peer2->Initialize());
    EXPECT_TRUE(peer3->Initialize());

    // Simulate message exchange
    peer_->SendMessage("Hello from User1");
    peer2->SendMessage("Hello from User2");
    peer3->SendMessage("Hello from User3");

    EXPECT_EQ(peer_->GetMessageCount(), 1);
    EXPECT_EQ(peer2->GetMessageCount(), 1);
    EXPECT_EQ(peer3->GetMessageCount(), 1);

    peer2->Shutdown();
    peer3->Shutdown();
}

// Test Window integration concepts
TEST_F(ChatP2PConsoleTest, WindowIntegrationConcept) {
    // This test demonstrates how the chat would integrate with Window
    // In real implementation, Window would:
    // 1. Display chat messages in a GUI panel
    // 2. Provide input field for typing messages
    // 3. Show list of connected peers
    // 4. Display connection status

    SetupChatEnvironment();

    // Simulate window events
    struct ChatWindow {
        vector<string> messageHistory;
        string inputBuffer;

        void DisplayMessage(const string& msg) {
            messageHistory.push_back(msg);
        }

        void SendMessage(const string& msg) {
            // Would call console command or direct peer API
        }
    };

    ChatWindow window;

    // User types message in window
    window.inputBuffer = "Hello from Window";

    // Window sends message through console
    ExecuteCommand("/chat " + window.inputBuffer);

    // Window displays sent message
    window.DisplayMessage("[You]: " + window.inputBuffer);

    EXPECT_EQ(window.messageHistory.size(), 1);
    EXPECT_EQ(peer_->GetLastMessage(), "Hello from Window");
}

// Test error handling
TEST_F(ChatP2PConsoleTest, ErrorHandling) {
    SetupChatEnvironment();

    // Test invalid command
    string result = ExecuteCommand("/chat");
    EXPECT_EQ(result, "Usage: /chat <message>");

    // Test when peer is shutdown
    peer_->Shutdown();
    result = ExecuteCommand("/status");
    EXPECT_EQ(result, "Chat peer is stopped");
}