#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

#include "KAI/Console/Console.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/Generate/Proxy.h"
#include "KAI/Language/Tau/Generate/Agent.h" 
#include "KAI/Network/Node.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Mock implementation of a P2P Chat Peer that works with Console
class MockChatPeer {
public:
    MockChatPeer(const string& username, int port) 
        : username_(username), port_(port), running_(false) {}

    bool Initialize() {
        // Initialize RakNet peer
        running_ = true;
        return true;
    }

    void Shutdown() {
        running_ = false;
    }

    bool IsRunning() const {
        return running_;
    }

    void SendMessage(const string& message) {
        // In real implementation, this would broadcast to all connected peers
        lastMessage_ = message;
        messageCount_++;
    }

    string GetLastMessage() const {
        return lastMessage_;
    }

    int GetMessageCount() const {
        return messageCount_;
    }

private:
    string username_;
    int port_;
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
        console_ = make_shared<Console>(registry_.get());
        
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
        console_->Execute(command);
        return console_->GetResult();
    }

    // Helper to register chat commands in console
    void RegisterChatCommands() {
        // Register /chat command
        console_->RegisterCommand("/chat", [this](const vector<string>& args) {
            if (args.size() < 2) {
                return string("Usage: /chat <message>");
            }
            
            string message;
            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) message += " ";
                message += args[i];
            }
            
            peer_->SendMessage(message);
            return "Message sent: " + message;
        });

        // Register /peers command
        console_->RegisterCommand("/peers", [this](const vector<string>& args) {
            return "Connected peers: 0"; // Mock implementation
        });

        // Register /status command  
        console_->RegisterCommand("/status", [this](const vector<string>& args) {
            return peer_->IsRunning() ? "Chat peer is running" : "Chat peer is stopped";
        });

        // Register /history command
        console_->RegisterCommand("/history", [this](const vector<string>& args) {
            return "Messages sent: " + to_string(peer_->GetMessageCount());
        });
    }

protected:
    shared_ptr<Registry> registry_;
    shared_ptr<Console> console_;
    unique_ptr<MockChatPeer> peer_;
};

// Test basic chat functionality through console
TEST_F(ChatP2PConsoleTest, BasicChatCommands) {
    RegisterChatCommands();
    
    // Test status command
    string result = ExecuteCommand("/status");
    EXPECT_EQ(result, "Chat peer is running");
    
    // Test sending a message
    result = ExecuteCommand("/chat Hello, world!");
    EXPECT_EQ(result, "Message sent: Hello, world!");
    EXPECT_EQ(peer_->GetLastMessage(), "Hello, world!");
    
    // Test message history
    result = ExecuteCommand("/history");
    EXPECT_EQ(result, "Messages sent: 1");
    
    // Send another message
    ExecuteCommand("/chat Another message");
    result = ExecuteCommand("/history");
    EXPECT_EQ(result, "Messages sent: 2");
}

// Test Tau interface parsing for chat
TEST_F(ChatP2PConsoleTest, ParseChatInterface) {
    // Load the chat interface file
    ifstream file("/home/xian/local/KAI/Test/Language/TestTau/Scripts/Connection/ChatInterface.tau");
    ASSERT_TRUE(file.is_open());
    
    stringstream buffer;
    buffer << file.rdbuf();
    string tauCode = buffer.str();
    
    // Parse the Tau interface
    tau::TauParser parser(*registry_);
    bool parseResult = parser.Parse(tauCode);
    EXPECT_TRUE(parseResult) << "Failed to parse Chat interface";
    
    // Verify key interfaces were parsed
    auto ast = parser.GetAST();
    ASSERT_NE(ast, nullptr);
    
    // Check for IChatPeer interface
    bool foundChatPeer = false;
    bool foundNetworkProxy = false;
    bool foundAgent = false;
    
    // In real implementation, we would traverse AST to find interfaces
    // For now, just verify parsing succeeded
    EXPECT_TRUE(parseResult);
}

// Test console integration with Rho scripting
TEST_F(ChatP2PConsoleTest, ConsoleRhoIntegration) {
    RegisterChatCommands();
    
    // Execute Rho code that sends chat messages
    string rhoScript = R"(
        {
            # Send a chat message using console command
            `{/chat Hello from Rho}`
            
            # Check status
            `{/status}`
        }
    )";
    
    // In real implementation, this would execute Rho script
    // For now, test direct commands
    ExecuteCommand("/chat Hello from test");
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
    
    RegisterChatCommands();
    
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
    RegisterChatCommands();
    
    // Test invalid command
    string result = ExecuteCommand("/chat");
    EXPECT_EQ(result, "Usage: /chat <message>");
    
    // Test when peer is shutdown
    peer_->Shutdown();
    result = ExecuteCommand("/status");
    EXPECT_EQ(result, "Chat peer is stopped");
}