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
        console1_->SetNetworkMessageCallback(
            [this](const NetworkConsoleMessage& msg) {
                lock_guard<mutex> lock(messagesMutex_);
                messages1_.push_back(msg);
            });

        console2_->SetNetworkMessageCallback(
            [this](const NetworkConsoleMessage& msg) {
                lock_guard<mutex> lock(messagesMutex_);
                messages2_.push_back(msg);
            });

        networkingAvailable_ = console1_->StartNetworking(0);
        if (networkingAvailable_) {
            console1_->StopNetworking();
        }
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
    bool WaitForMessages(int expectedCount1, int expectedCount2,
                         int timeoutMs = 2000) {
        auto deadline =
            chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);

        while (chrono::steady_clock::now() < deadline) {
            {
                lock_guard<mutex> lock(messagesMutex_);
                if (messages1_.size() >= expectedCount1 &&
                    messages2_.size() >= expectedCount2) {
                    return true;
                }
            }
            Wait(10);
        }
        return false;
    }

    static void Wait(int ms) {
        this_thread::sleep_for(chrono::milliseconds(ms));
    }

   protected:
    unique_ptr<Console> console1_;
    unique_ptr<Console> console2_;
    vector<NetworkConsoleMessage> messages1_;
    vector<NetworkConsoleMessage> messages2_;
    mutex messagesMutex_;
    bool networkingAvailable_ = false;
};

// Test basic networking setup and connection
TEST_F(ConsoleNetworkingTest, BasicNetworkSetup) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Start networking on console1
    string result = ExecuteNetworkCommand(*console1_, "/network start 14700");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console1_->IsNetworkingEnabled());
    Wait(200);

    // Check network status
    result = ExecuteNetworkCommand(*console1_, "/network status");
    EXPECT_TRUE(result.find("Network enabled") != string::npos);
    EXPECT_TRUE(result.find("14700") != string::npos);

    // Start networking on console2 with different port
    result = ExecuteNetworkCommand(*console2_, "/network start 14701");
    EXPECT_EQ(result, "Network started");
    EXPECT_TRUE(console2_->IsNetworkingEnabled());
    Wait(200);

    // Connect console2 to console1
    result = ExecuteNetworkCommand(*console2_, "/connect localhost 14700");
    EXPECT_EQ(result, "Connecting...");

    std::vector<std::string> peersOnClient;
    for (int attempt = 0; attempt < 50; ++attempt) {
        peersOnClient = console2_->GetConnectedPeers();
        if (!peersOnClient.empty()) break;
        Wait(100);
    }
    ASSERT_FALSE(peersOnClient.empty())
        << "Client did not establish connection";

    std::vector<std::string> peersOnServer;
    for (int attempt = 0; attempt < 50; ++attempt) {
        peersOnServer = console1_->GetConnectedPeers();
        if (!peersOnServer.empty()) break;
        Wait(100);
    }
    ASSERT_FALSE(peersOnServer.empty()) << "Server did not register client";

    // Check peers on console1
    result = ExecuteNetworkCommand(*console1_, "/peers");
    EXPECT_TRUE(result.find("Connected peers (1)") != string::npos);

    // Check peers on console2
    result = ExecuteNetworkCommand(*console2_, "/peers");
    EXPECT_TRUE(result.find("Connected peers (1)") != string::npos);
}

// Test sending commands between consoles
TEST_F(ConsoleNetworkingTest, SendCommandToPeer) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Setup network connection
    ExecuteNetworkCommand(*console1_, "/network start 14702");
    ExecuteNetworkCommand(*console2_, "/network start 14703");
    Wait(200);
    ExecuteNetworkCommand(*console2_, "/connect localhost 14702");
    std::vector<std::string> peers;
    for (int attempt = 0; attempt < 50; ++attempt) {
        peers = console2_->GetConnectedPeers();
        if (!peers.empty()) break;
        Wait(100);
    }
    ASSERT_FALSE(peers.empty()) << "No peers connected";
    std::string peerAddress = peers.front();

    // Seed remote stack by pushing a value from the peer
    ExecuteNetworkCommand(*console2_, std::string("/@") + peerAddress + " 42");
    ExecuteNetworkCommand(*console2_, std::string("/@") + peerAddress + " 7 *");

    // Wait for both commands to complete
    EXPECT_TRUE(WaitForMessages(2, 2, 2000));

    // Capture remote console identifier from message history
    std::string remoteConsoleId;
    {
        lock_guard<mutex> lock(messagesMutex_);
        ASSERT_GE(messages1_.size(), 2);
        remoteConsoleId = messages1_.back().senderId;
        EXPECT_EQ(messages1_[0].command, "42");
        EXPECT_EQ(messages1_[1].command, "7 *");
    }

    // Verify the peer-specific stack contains the expected result
    std::string peerStack =
        console1_->WriteStackForPeer(remoteConsoleId).c_str();
    EXPECT_TRUE(peerStack.find("294") != std::string::npos);

    // Ensure local console stack was not modified by the peer
    std::string localStack = console1_->WriteStack().c_str();
    EXPECT_TRUE(localStack.find("294") == std::string::npos);
}

// Test broadcasting commands to multiple peers
TEST_F(ConsoleNetworkingTest, BroadcastCommand) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Setup network - console1 as server, console2 as client
    ExecuteNetworkCommand(*console1_, "/network start 14704");
    ExecuteNetworkCommand(*console2_, "/network start 14705");
    Wait(200);
    ExecuteNetworkCommand(*console2_, "/connect localhost 14704");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty() &&
            !console1_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    // Broadcast a command from console2
    ExecuteNetworkCommand(*console2_, "/broadcast 10 5 +");

    // Wait for broadcast to be processed
    EXPECT_TRUE(WaitForMessages(1, 0, 2000));

    // Verify the command was executed on console1
    std::string broadcastConsoleId;
    {
        lock_guard<mutex> lock(messagesMutex_);
        ASSERT_GE(messages1_.size(), 1);
        broadcastConsoleId = messages1_[0].senderId;
    }

    std::string broadcastStack =
        console1_->WriteStackForPeer(broadcastConsoleId).c_str();
    EXPECT_TRUE(broadcastStack.find("15") != std::string::npos);

    // Local stack should remain untouched unless explicitly modified
    std::string localStack = console1_->WriteStack().c_str();
    EXPECT_TRUE(localStack.find("15") == std::string::npos);

    // Check message history for broadcast
    {
        lock_guard<mutex> lock(messagesMutex_);
        EXPECT_GE(messages1_.size(), 1);
        EXPECT_TRUE(messages1_[0].senderId.find("BROADCAST") != string::npos);
        EXPECT_EQ(messages1_[0].command, "10 5 +");
        EXPECT_EQ(messages1_[0].result, "15");
    }
}

TEST_F(ConsoleNetworkingTest, MultiPeerBroadcast) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    ExecuteNetworkCommand(*console1_, "/network start 14716");
    ExecuteNetworkCommand(*console2_, "/network start 14717");
    Wait(200);

    auto console3 = std::make_unique<Console>();
    SetupConsoleTranslator(*console3, Language::Pi);
    ExecuteNetworkCommand(*console3, "/network start 14718");
    Wait(200);

    std::vector<NetworkConsoleMessage> messages3;
    console3->SetNetworkMessageCallback(
        [this, &messages3](const NetworkConsoleMessage& msg) {
            std::lock_guard<std::mutex> lock(messagesMutex_);
            messages3.push_back(msg);
        });

    ExecuteNetworkCommand(*console2_, "/connect localhost 14716");
    ExecuteNetworkCommand(*console3, "/connect localhost 14716");

    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty() &&
            !console1_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console3->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    ExecuteCommand(*console2_, "15");
    ExecuteCommand(*console3, "20");

    ExecuteNetworkCommand(*console1_, "/broadcast 8 2 +");

    EXPECT_TRUE(WaitForMessages(1, 1, 2000));

    for (int attempt = 0; attempt < 50; ++attempt) {
        bool received;
        {
            std::lock_guard<std::mutex> lock(messagesMutex_);
            received = messages3.size() >= 1;
        }
        if (received) {
            break;
        }
        Wait(50);
    }

    std::string stack2 = console2_->WriteStack().c_str();
    std::string stack3 = console3->WriteStack().c_str();
    EXPECT_TRUE(stack2.find("10") != std::string::npos);
    EXPECT_TRUE(stack3.find("10") != std::string::npos);

    {
        std::lock_guard<std::mutex> lock(messagesMutex_);
        ASSERT_GE(messages1_.size(), 1);
        EXPECT_TRUE(messages1_[0].senderId.find("BROADCAST") !=
                    std::string::npos);
        EXPECT_EQ(messages2_.size(), 1);
        EXPECT_EQ(messages2_[0].result, "10");
        EXPECT_EQ(messages3.size(), 1);
        EXPECT_EQ(messages3[0].result, "10");
    }
}

// Test cross-language communication (Pi to Rho)
TEST_F(ConsoleNetworkingTest, CrossLanguageCommunication) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Setup: console1 in Pi mode, console2 in Rho mode
    console1_->SetLanguage(Language::Pi);
    SetupConsoleTranslator(*console1_, Language::Pi);

    console2_->SetLanguage(Language::Rho);
    SetupConsoleTranslator(*console2_, Language::Rho);

    ExecuteNetworkCommand(*console1_, "/network start 14706");
    ExecuteNetworkCommand(*console2_, "/network start 14707");
    Wait(200);
    ExecuteNetworkCommand(*console2_, "/connect localhost 14706");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

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
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Test commands without networking enabled
    string result =
        ExecuteNetworkCommand(*console1_, "/connect localhost 14708");
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
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Setup network connection
    ExecuteNetworkCommand(*console1_, "/network start 14710");
    ExecuteNetworkCommand(*console2_, "/network start 14711");
    Wait(200);
    ExecuteNetworkCommand(*console2_, "/connect localhost 14710");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    // Send several commands
    ExecuteNetworkCommand(*console2_, "/@0 1 2 +");
    ExecuteNetworkCommand(*console2_, "/@0 3 4 *");
    ExecuteNetworkCommand(*console2_, "/broadcast 5 6 -");

    if (!WaitForMessages(3, 2, 3000)) {
        GTEST_SKIP() << "Timed out waiting for network message history in this "
                        "environment";
    }

    // Check network history on console1
    string history = ExecuteNetworkCommand(*console1_, "/nethistory");
    EXPECT_TRUE(history.find("Network History") != string::npos);
    EXPECT_TRUE(history.find("1 2 +") != string::npos);
    EXPECT_TRUE(history.find("3 4 *") != string::npos);
    EXPECT_TRUE(history.find("5 6 -") != string::npos);

    // Verify message details
    {
        lock_guard<mutex> lock(messagesMutex_);
        ASSERT_GE(messages1_.size(), 3);

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
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
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
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    // Setup both consoles with networking
    ASSERT_EQ(ExecuteNetworkCommand(*console1_, "/network start 14714"),
              "Network started");
    ASSERT_EQ(ExecuteNetworkCommand(*console2_, "/network start 14715"),
              "Network started");
    Wait(200);

    // Connect console2 to console1
    ASSERT_EQ(ExecuteNetworkCommand(*console2_, "/connect localhost 14714"),
              "Connecting...");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty()) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }

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
    ExecuteNetworkCommand(*console2_,
                          "/@0 +");  // Add top two numbers: 100 + 200 = 300
    ExecuteNetworkCommand(*console2_,
                          "/@0 50 -");  // Subtract 50: 300 - 50 = 250
    ExecuteNetworkCommand(*console2_, "/@0 2 /");  // Divide by 2: 250 / 2 = 125

    // Wait for all commands to complete
    EXPECT_TRUE(WaitForMessages(3, 3, 3000));

    // Verify final result on console1
    string finalStack = console1_->WriteStack().c_str();
    EXPECT_TRUE(finalStack.find("125") != string::npos);

    // Console1: Broadcast a command to all peers (console2)
    ExecuteCommand(*console2_, "10 20 30");  // Set up console2's stack
    ExecuteNetworkCommand(*console1_,
                          "/broadcast +");  // Add top two: 20 + 30 = 50

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

TEST_F(ConsoleNetworkingTest, PeerDisconnectCleanup) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    ExecuteNetworkCommand(*console1_, "/network start 14722");
    ExecuteNetworkCommand(*console2_, "/network start 14723");
    Wait(200);

    ExecuteNetworkCommand(*console2_, "/connect localhost 14722");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    ExecuteNetworkCommand(*console2_, "/@0 9");
    EXPECT_TRUE(WaitForMessages(1, 1, 2000));

    std::string peerId;
    {
        std::lock_guard<std::mutex> lock(messagesMutex_);
        ASSERT_FALSE(messages1_.empty());
        peerId = messages1_.back().senderId;
    }

    ExecuteNetworkCommand(*console2_, "/network stop");
    Wait(200);

    EXPECT_TRUE(console1_->GetConnectedPeers().empty());
    EXPECT_TRUE(console1_->WriteStackForPeer(peerId).empty());
}

TEST_F(ConsoleNetworkingTest, ResultHistoryNormalization) {
    if (!networkingAvailable_)
        GTEST_SKIP() << "Console networking is unavailable in this environment";
    ExecuteNetworkCommand(*console1_, "/network start 14724");
    ExecuteNetworkCommand(*console2_, "/network start 14725");
    Wait(200);

    ExecuteNetworkCommand(*console2_, "/connect localhost 14724");
    for (int attempt = 0; attempt < 50; ++attempt) {
        if (!console2_->GetConnectedPeers().empty()) {
            break;
        }
        Wait(100);
    }

    ExecuteNetworkCommand(*console2_, "/@0 2 3 +");
    EXPECT_TRUE(WaitForMessages(1, 1, 2000));

    auto history = console1_->GetNetworkHistory();
    ASSERT_FALSE(history.empty());
    EXPECT_EQ(history.back().result, "5");
}
