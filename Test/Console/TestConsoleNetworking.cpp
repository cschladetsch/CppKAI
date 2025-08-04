#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "KAI/Console/Console.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestConsoleHelper.h"

using namespace kai;
using namespace std;

// Register translators for testing
REGISTER_TRANSLATOR(Language::Pi, PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

class ConsoleNetworkingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create two console instances
        console1_ = make_unique<Console>();
        console2_ = make_unique<Console>();
        
        // Set up translators for both consoles
        SetupConsoleTranslator(*console1_, Language::Pi);
        SetupConsoleTranslator(*console2_, Language::Pi);
        
        // Message collection for verification
        messages1_.clear();
        messages2_.clear();
        
        // Set up message callbacks to capture network messages
        console1_->SetNetworkMessageCallback([this](const NetworkConsoleMessage& msg) {
            lock_guard<mutex> lock(messagesMutex_);
            messages1_.push_back(msg);
        });
        
        console2_->SetNetworkMessageCallback([this](const NetworkConsoleMessage& msg) {
            lock_guard<mutex> lock(messagesMutex_);
            messages2_.push_back(msg);
        });
    }
    
    void TearDown() override {
        if (console1_) {
            console1_->StopNetworking();
        }
        if (console2_) {
            console2_->StopNetworking();
        }
        
        // Allow time for cleanup
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    void SetupConsoleTranslator(Console& console, Language lang) {
        console.SetLanguage(lang);
        auto translator = TranslatorFactory::Instance().CreateTranslator(
            lang, console.GetRegistry());
        ASSERT_TRUE(translator != nullptr) << "Failed to create translator";
        console.SetTranslator(translator);
    }
    
    // Helper to execute command and get result
    string ExecuteCommand(Console& console, const string& command) {
        console.Execute(String(command.c_str()));
        return console.WriteStack().c_str();
    }
    
    // Helper to execute network command and get response
    string ExecuteNetworkCommand(Console& console, const string& command) {
        String result = console.ProcessNetworkCommand(String(command.c_str()));
        return result.c_str();
    }
    
    // Wait for network messages with timeout
    bool WaitForMessages(int expectedCount1, int expectedCount2, int timeoutMs = 2000) {
        auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
        
        while (chrono::steady_clock::now() < deadline) {
            {
                lock_guard<mutex> lock(messagesMutex_);
                if (messages1_.size() >= expectedCount1 && messages2_.size() >= expectedCount2) {
                    return true;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        return false;
    }
    
protected:
    unique_ptr<Console> console1_;
    unique_ptr<Console> console2_;
    vector<NetworkConsoleMessage> messages1_;
    vector<NetworkConsoleMessage> messages2_;
    mutex messagesMutex_;
};

// Test basic networking setup and connection
TEST_F(ConsoleNetworkingTest, BasicNetworkSetup) {
    // Start networking on console1
    string result = ExecuteNetworkCommand(*console1_, "/network start 14700");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console1_->IsNetworkingEnabled());
    
    // Check network status
    result = ExecuteNetworkCommand(*console1_, "/network status");
    EXPECT_TRUE(result.find("Network enabled") != string::npos);
    EXPECT_TRUE(result.find("14700") != string::npos);
    
    // Start networking on console2 with different port
    result = ExecuteNetworkCommand(*console2_, "/network start 14701");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console2_->IsNetworkingEnabled());
    
    // Connect console2 to console1
    result = ExecuteNetworkCommand(*console2_, "/connect localhost 14700");
    EXPECT_EQ(result, "Connecting...");
    
    // Allow time for connection
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Check peers on console1
    result = ExecuteNetworkCommand(*console1_, "/peers");
    EXPECT_TRUE(result.find("Connected peers (1)") != string::npos);
    
    // Check peers on console2
    result = ExecuteNetworkCommand(*console2_, "/peers");
    EXPECT_TRUE(result.find("Connected peers (1)") != string::npos);
}

// Test sending commands between consoles
TEST_F(ConsoleNetworkingTest, SendCommandToPeer) {
    // Setup network connection
    ExecuteNetworkCommand(*console1_, "/network start 14702");
    ExecuteNetworkCommand(*console2_, "/network start 14703");
    ExecuteNetworkCommand(*console2_, "/connect localhost 14702");
    
    // Allow connection to establish
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Execute a command on console1 to set up stack
    ExecuteCommand(*console1_, "42");
    string stack1 = ExecuteCommand(*console1_, "");  // Get current stack
    EXPECT_TRUE(stack1.find("42") != string::npos);
    
    // Send command from console2 to console1
    ExecuteNetworkCommand(*console2_, "/@0 7 *");
    
    // Wait for command to be processed
    EXPECT_TRUE(WaitForMessages(1, 1, 2000));
    
    // Verify the command was executed on console1
    // The stack should now contain 42 * 7 = 294
    string newStack = console1_->WriteStack().c_str();
    EXPECT_TRUE(newStack.find("294") != string::npos);
    
    // Check message history
    {
        lock_guard<mutex> lock(messagesMutex_);
        EXPECT_GE(messages1_.size(), 1);
        EXPECT_EQ(messages1_[0].command, "7 *");
    }
}

// Test broadcasting commands to multiple peers
TEST_F(ConsoleNetworkingTest, BroadcastCommand) {
    // Setup network - console1 as server, console2 as client
    ExecuteNetworkCommand(*console1_, "/network start 14704");
    ExecuteNetworkCommand(*console2_, "/network start 14705");
    ExecuteNetworkCommand(*console2_, "/connect localhost 14704");
    
    // Allow connection to establish
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Broadcast a command from console2
    ExecuteNetworkCommand(*console2_, "/broadcast 10 5 +");
    
    // Wait for broadcast to be processed
    EXPECT_TRUE(WaitForMessages(1, 0, 2000));
    
    // Verify the command was executed on console1
    string stack1 = console1_->WriteStack().c_str();
    EXPECT_TRUE(stack1.find("15") != string::npos);
    
    // Check message history for broadcast
    {
        lock_guard<mutex> lock(messagesMutex_);
        EXPECT_GE(messages1_.size(), 1);
        EXPECT_TRUE(messages1_[0].senderId.find("BROADCAST") != string::npos);
        EXPECT_EQ(messages1_[0].command, "10 5 +");
        EXPECT_EQ(messages1_[0].result, "15");
    }
}

// Test cross-language communication (Pi to Rho)
TEST_F(ConsoleNetworkingTest, CrossLanguageCommunication) {
    // Setup: console1 in Pi mode, console2 in Rho mode
    console1_->SetLanguage(Language::Pi);
    SetupConsoleTranslator(*console1_, Language::Pi);
    
    console2_->SetLanguage(Language::Rho);
    SetupConsoleTranslator(*console2_, Language::Rho);
    
    ExecuteNetworkCommand(*console1_, "/network start 14706");
    ExecuteNetworkCommand(*console2_, "/network start 14707");
    ExecuteNetworkCommand(*console2_, "/connect localhost 14706");
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Send Pi command from console2 (Rho) to console1 (Pi)
    ExecuteNetworkCommand(*console2_, "/@0 3 4 +");
    
    EXPECT_TRUE(WaitForMessages(1, 1, 2000));
    
    // Verify Pi command executed correctly on console1
    string stack1 = console1_->WriteStack().c_str();
    EXPECT_TRUE(stack1.find("7") != string::npos);
    
    // Send Rho command from console1 (Pi) to console2 (Rho) - this should work
    // because the receiver (console2) will execute in its own language context
    ExecuteNetworkCommand(*console1_, "/@0 x = 25;");
    
    EXPECT_TRUE(WaitForMessages(1, 2, 2000));
}

// Test error handling in network commands
TEST_F(ConsoleNetworkingTest, NetworkErrorHandling) {
    // Test commands without networking enabled
    string result = ExecuteNetworkCommand(*console1_, "/connect localhost 14708");
    EXPECT_EQ(result, "Network not enabled. Use '/network start' first.");
    
    result = ExecuteNetworkCommand(*console1_, "/peers");
    EXPECT_EQ(result, "Network not enabled. Use '/network start' first.");
    
    result = ExecuteNetworkCommand(*console1_, "/broadcast test");
    EXPECT_EQ(result, "Network not enabled. Use '/network start' first.");
    
    // Start networking and test invalid commands
    ExecuteNetworkCommand(*console1_, "/network start 14709");
    
    // Test invalid peer reference
    result = ExecuteNetworkCommand(*console1_, "/@999 test");
    EXPECT_EQ(result, "Failed to send command");
    
    // Test broadcast with no peers
    result = ExecuteNetworkCommand(*console1_, "/broadcast test");
    EXPECT_EQ(result, "");  // Should execute but show warning in output
    
    // Test invalid network subcommands
    result = ExecuteNetworkCommand(*console1_, "/network invalid");
    EXPECT_EQ(result, "Usage: /network {start|stop|status} [port]");
    
    // Test malformed commands
    result = ExecuteNetworkCommand(*console1_, "/connect");
    EXPECT_EQ(result, "Usage: /connect <host> <port>");
    
    result = ExecuteNetworkCommand(*console1_, "/broadcast");
    EXPECT_EQ(result, "Usage: /broadcast <command>");
}

// Test network message history
TEST_F(ConsoleNetworkingTest, MessageHistory) {
    // Setup network connection
    ExecuteNetworkCommand(*console1_, "/network start 14710");
    ExecuteNetworkCommand(*console2_, "/network start 14711");
    ExecuteNetworkCommand(*console2_, "/connect localhost 14710");
    
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Send several commands
    ExecuteNetworkCommand(*console2_, "/@0 1 2 +");
    ExecuteNetworkCommand(*console2_, "/@0 3 4 *");
    ExecuteNetworkCommand(*console2_, "/broadcast 5 6 -");
    
    EXPECT_TRUE(WaitForMessages(3, 2, 3000));
    
    // Check network history on console1
    string history = ExecuteNetworkCommand(*console1_, "/nethistory");
    EXPECT_TRUE(history.find("Network History") != string::npos);
    EXPECT_TRUE(history.find("1 2 +") != string::npos);
    EXPECT_TRUE(history.find("3 4 *") != string::npos);
    EXPECT_TRUE(history.find("5 6 -") != string::npos);
    
    // Verify message details
    {
        lock_guard<mutex> lock(messagesMutex_);
        EXPECT_EQ(messages1_.size(), 3);
        
        // Check first message
        EXPECT_EQ(messages1_[0].command, "1 2 +");
        EXPECT_EQ(messages1_[0].result, "3");
        
        // Check second message  
        EXPECT_EQ(messages1_[1].command, "3 4 *");
        EXPECT_EQ(messages1_[1].result, "12");
        
        // Check broadcast message
        EXPECT_TRUE(messages1_[2].senderId.find("BROADCAST") != string::npos);
        EXPECT_EQ(messages1_[2].command, "5 6 -");
        EXPECT_EQ(messages1_[2].result, "-1");
    }
}

// Test stopping and restarting networking
TEST_F(ConsoleNetworkingTest, NetworkStartStop) {
    // Start networking
    string result = ExecuteNetworkCommand(*console1_, "/network start 14712");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console1_->IsNetworkingEnabled());
    
    // Stop networking
    result = ExecuteNetworkCommand(*console1_, "/network stop");
    EXPECT_EQ(result, "Network stopped");
    EXPECT_FALSE(console1_->IsNetworkingEnabled());
    
    // Verify commands don't work when stopped
    result = ExecuteNetworkCommand(*console1_, "/peers");
    EXPECT_EQ(result, "Network not enabled. Use '/network start' first.");
    
    // Restart networking
    result = ExecuteNetworkCommand(*console1_, "/network start 14713");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console1_->IsNetworkingEnabled());
    
    // Verify it works again
    result = ExecuteNetworkCommand(*console1_, "/peers");
    EXPECT_EQ(result, "No peers connected");
}

// Integration test: Complete console communication workflow
TEST_F(ConsoleNetworkingTest, CompleteWorkflow) {
    // Setup both consoles with networking
    ASSERT_EQ(ExecuteNetworkCommand(*console1_, "/network start 14714"), "Network started");
    ASSERT_EQ(ExecuteNetworkCommand(*console2_, "/network start 14715"), "Network started");
    
    // Connect console2 to console1
    ASSERT_EQ(ExecuteNetworkCommand(*console2_, "/connect localhost 14714"), "Connecting...");
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Verify connection
    string peers1 = ExecuteNetworkCommand(*console1_, "/peers");
    string peers2 = ExecuteNetworkCommand(*console2_, "/peers");
    EXPECT_TRUE(peers1.find("Connected peers (1)") != string::npos);
    EXPECT_TRUE(peers2.find("Connected peers (1)") != string::npos);
    
    // Console1: Set up some initial data
    ExecuteCommand(*console1_, "100");
    ExecuteCommand(*console1_, "200");
    string initialStack = ExecuteCommand(*console1_, "");
    EXPECT_TRUE(initialStack.find("100") != string::npos);
    EXPECT_TRUE(initialStack.find("200") != string::npos);
    
    // Console2: Send commands to manipulate console1's stack
    ExecuteNetworkCommand(*console2_, "/@0 +");           // Add top two numbers: 100 + 200 = 300
    ExecuteNetworkCommand(*console2_, "/@0 50 -");        // Subtract 50: 300 - 50 = 250
    ExecuteNetworkCommand(*console2_, "/@0 2 /");         // Divide by 2: 250 / 2 = 125
    
    // Wait for all commands to complete
    EXPECT_TRUE(WaitForMessages(3, 3, 3000));
    
    // Verify final result on console1
    string finalStack = console1_->WriteStack().c_str();
    EXPECT_TRUE(finalStack.find("125") != string::npos);
    
    // Console1: Broadcast a command to all peers (console2)
    ExecuteCommand(*console2_, "10 20 30");  // Set up console2's stack
    ExecuteNetworkCommand(*console1_, "/broadcast +");     // Add top two: 20 + 30 = 50
    
    EXPECT_TRUE(WaitForMessages(4, 4, 2000));
    
    // Verify console2's stack
    string console2Stack = console2_->WriteStack().c_str();
    EXPECT_TRUE(console2Stack.find("10") != string::npos);
    EXPECT_TRUE(console2Stack.find("50") != string::npos);
    
    // Check complete message history
    auto history1 = console1_->GetNetworkHistory();
    auto history2 = console2_->GetNetworkHistory();
    
    EXPECT_GE(history1.size(), 4);  // 3 received commands + 1 broadcast
    EXPECT_GE(history2.size(), 4);  // 3 sent results + 1 broadcast received
    
    // Verify network status shows active connections
    string status1 = ExecuteNetworkCommand(*console1_, "/network status");
    string status2 = ExecuteNetworkCommand(*console2_, "/network status");
    
    EXPECT_TRUE(status1.find("Network enabled") != string::npos);
    EXPECT_TRUE(status1.find("peers: 1") != string::npos);
    EXPECT_TRUE(status2.find("Network enabled") != string::npos);
    EXPECT_TRUE(status2.find("peers: 1") != string::npos);
}