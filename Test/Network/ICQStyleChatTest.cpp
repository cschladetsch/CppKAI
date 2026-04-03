#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "KAI/Console/Console.h"
#include "KAI/Core/Registry.h"
#include "KAI/Network/Node.h"

using namespace kai;
using namespace std;

// Mock P2P Chat implementation with ICQ-style interface
class ICQStyleChat {
   public:
    struct Channel {
        string name;
        string hostUsername;
        string hostAddress;
        vector<string> users;
        map<string, string> messages;  // timestamp -> message
    };

    ICQStyleChat() : isInitialized_(false), currentUsername_("") {}

    // Core functionality
    bool Initialize(int port = 14589) {
        port_ = port;
        isInitialized_ = true;
        return true;
    }

    bool SetUsername(const string& username) {
        if (username.empty()) return false;
        currentUsername_ = username;
        return true;
    }

    string GetUsername() const { return currentUsername_; }

    bool HasUsername() const { return !currentUsername_.empty(); }

    // Channel operations
    bool Publish(const string& channelName) {
        if (!HasUsername()) return false;

        // Create and host a new channel
        Channel channel;
        channel.name = channelName;
        channel.hostUsername = currentUsername_;
        channel.hostAddress = "127.0.0.1:" + to_string(port_);
        channel.users.push_back(currentUsername_);

        hostedChannels_[channelName] = channel;
        currentChannel_ = channelName;
        isHosting_ = true;

        // Broadcast channel availability
        BroadcastChannelAnnouncement(channelName);
        return true;
    }

    bool Enter(const string& channelName) {
        if (!HasUsername()) return false;

        // Find channel in discovered list
        auto it = discoveredChannels_.find(channelName);
        if (it == discoveredChannels_.end()) return false;

        currentChannel_ = channelName;
        isHosting_ = false;

        // Add self to channel users
        it->second.users.push_back(currentUsername_);

        return true;
    }

    bool Leave() {
        if (currentChannel_.empty()) return false;

        if (isHosting_) {
            hostedChannels_.erase(currentChannel_);
        }

        currentChannel_ = "";
        isHosting_ = false;
        return true;
    }

    vector<string> Discover() {
        vector<string> channels;

        // Add hosted channels
        for (const auto& [name, channel] : hostedChannels_) {
            channels.push_back(name + " (hosted by you)");
        }

        // Add discovered channels
        for (const auto& [name, channel] : discoveredChannels_) {
            channels.push_back(name + " (hosted by " + channel.hostUsername +
                               ")");
        }

        return channels;
    }

    bool SendMessage(const string& message) {
        if (currentChannel_.empty() || !HasUsername()) return false;

        string formattedMsg = "[" + currentUsername_ + "]: " + message;

        if (isHosting_) {
            hostedChannels_[currentChannel_].messages[GetTimestamp()] =
                formattedMsg;
        } else {
            // In real P2P, this would send to channel host
            discoveredChannels_[currentChannel_].messages[GetTimestamp()] =
                formattedMsg;
        }

        return true;
    }

    vector<string> GetUsers() {
        vector<string> users;

        if (!currentChannel_.empty()) {
            if (isHosting_) {
                users = hostedChannels_[currentChannel_].users;
            } else {
                users = discoveredChannels_[currentChannel_].users;
            }
        }

        return users;
    }

    // Simulate channel discovery
    void SimulateChannelDiscovery(const string& channelName,
                                  const string& hostUsername) {
        Channel channel;
        channel.name = channelName;
        channel.hostUsername = hostUsername;
        channel.hostAddress = "192.168.1.100:14589";  // Mock address
        discoveredChannels_[channelName] = channel;
    }

   private:
    void BroadcastChannelAnnouncement(const string& channelName) {
        // In real implementation, this would use UDP broadcast
    }

    string GetTimestamp() { return to_string(time(nullptr)); }

   private:
    bool isInitialized_;
    int port_;
    string currentUsername_;
    string currentChannel_;
    bool isHosting_ = false;

    map<string, Channel> hostedChannels_;
    map<string, Channel> discoveredChannels_;
};

// Test fixture
class ICQStyleChatTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry_ = make_shared<Registry>();
        console_ = make_shared<Console>(registry_.get());
        chat_ = make_unique<ICQStyleChat>();

        RegisterChatCommands();
    }

    void RegisterChatCommands() {
        // Register username command
        console_->RegisterCommand(
            "username", [this](const vector<string>& args) {
                if (args.size() != 2) {
                    return string("Usage: username <name>");
                }

                if (chat_->SetUsername(args[1])) {
                    return "Username set to: " + args[1];
                }
                return "Failed to set username";
            });

        // Register publish command
        console_->RegisterCommand(
            "publish", [this](const vector<string>& args) {
                if (args.size() != 2) {
                    return string("Usage: publish <channel>");
                }

                if (!chat_->HasUsername()) {
                    return string(
                        "Error: Set username first with 'username <name>'");
                }

                if (chat_->Publish(args[1])) {
                    return "Publishing channel: " + args[1];
                }
                return "Failed to publish channel";
            });

        // Register enter command
        console_->RegisterCommand("enter", [this](const vector<string>& args) {
            if (args.size() != 2) {
                return string("Usage: enter <channel>");
            }

            if (!chat_->HasUsername()) {
                return string(
                    "Error: Set username first with 'username <name>'");
            }

            if (chat_->Enter(args[1])) {
                return "Entered channel: " + args[1];
            }
            return "Channel not found or unable to join";
        });

        // Register leave command
        console_->RegisterCommand("leave", [this](const vector<string>& args) {
            if (chat_->Leave()) {
                return string("Left channel");
            }
            return "Not in any channel";
        });

        // Register discover command
        console_->RegisterCommand("discover",
                                  [this](const vector<string>& args) {
                                      auto channels = chat_->Discover();
                                      if (channels.empty()) {
                                          return string("No channels found");
                                      }

                                      string result = "Available channels:\n";
                                      for (const auto& channel : channels) {
                                          result += "  " + channel + "\n";
                                      }
                                      return result;
                                  });

        // Register users command
        console_->RegisterCommand("users", [this](const vector<string>& args) {
            auto users = chat_->GetUsers();
            if (users.empty()) {
                return string("No users in current channel");
            }

            string result = "Users in channel:\n";
            for (const auto& user : users) {
                result += "  " + user + "\n";
            }
            return result;
        });

        // Register help command
        console_->RegisterCommand("help", [this](const vector<string>& args) {
            return string("ICQ-style P2P Chat Commands:\n") +
                   "  username <name>  - Set your username\n" +
                   "  publish <channel> - Create and host a channel\n" +
                   "  enter <channel>   - Join an existing channel\n" +
                   "  leave            - Leave current channel\n" +
                   "  discover         - List available channels\n" +
                   "  users            - List users in current channel\n" +
                   "  <message>        - Send message to current channel\n" +
                   "  help             - Show this help";
        });

        // Default handler for messages
        console_->SetDefaultHandler([this](const string& input) {
            if (chat_->SendMessage(input)) {
                return "";  // Don't echo back the message
            }
            return "Not in a channel or username not set";
        });
    }

   protected:
    shared_ptr<Registry> registry_;
    shared_ptr<Console> console_;
    unique_ptr<ICQStyleChat> chat_;
};

// Tests
TEST_F(ICQStyleChatTest, RequireUsernameBeforeChannelOperations) {
    chat_->Initialize();

    // Try to publish without username
    auto result = console_->Execute("publish test-channel");
    EXPECT_EQ(result, "Error: Set username first with 'username <name>'");

    // Set username
    result = console_->Execute("username Alice");
    EXPECT_EQ(result, "Username set to: Alice");

    // Now publish should work
    result = console_->Execute("publish test-channel");
    EXPECT_EQ(result, "Publishing channel: test-channel");
}

TEST_F(ICQStyleChatTest, ChannelDiscoveryAndJoining) {
    chat_->Initialize();

    // Set username
    console_->Execute("username Bob");

    // Simulate discovered channels
    chat_->SimulateChannelDiscovery("general", "Alice");
    chat_->SimulateChannelDiscovery("dev-talk", "Charlie");

    // Discover channels
    auto result = console_->Execute("discover");
    EXPECT_NE(result.find("general"), string::npos);
    EXPECT_NE(result.find("dev-talk"), string::npos);
    EXPECT_NE(result.find("Alice"), string::npos);
    EXPECT_NE(result.find("Charlie"), string::npos);

    // Join a channel
    result = console_->Execute("enter general");
    EXPECT_EQ(result, "Entered channel: general");
}

TEST_F(ICQStyleChatTest, MessageSending) {
    chat_->Initialize();

    // Setup
    console_->Execute("username Alice");
    console_->Execute("publish test-chat");

    // Send message
    auto result = console_->Execute("Hello, world!");
    EXPECT_EQ(result, "");  // Message sent successfully, no echo

    // Try sending without being in channel
    console_->Execute("leave");
    result = console_->Execute("This should fail");
    EXPECT_EQ(result, "Not in a channel or username not set");
}

TEST_F(ICQStyleChatTest, MultipleUsersSimulation) {
    // Simulate multiple chat instances
    auto chat1 = make_unique<ICQStyleChat>();
    auto chat2 = make_unique<ICQStyleChat>();

    chat1->Initialize(14589);
    chat2->Initialize(14590);

    // User 1 creates channel
    chat1->SetUsername("Alice");
    EXPECT_TRUE(chat1->Publish("tech-talk"));

    // User 2 discovers and joins
    chat2->SetUsername("Bob");
    chat2->SimulateChannelDiscovery("tech-talk", "Alice");
    EXPECT_TRUE(chat2->Enter("tech-talk"));

    // Both users can send messages
    EXPECT_TRUE(chat1->SendMessage("Welcome to tech-talk!"));
    EXPECT_TRUE(chat2->SendMessage("Thanks for hosting!"));
}

TEST_F(ICQStyleChatTest, HelpCommand) {
    chat_->Initialize();

    auto result = console_->Execute("help");
    EXPECT_NE(result.find("username"), string::npos);
    EXPECT_NE(result.find("publish"), string::npos);
    EXPECT_NE(result.find("enter"), string::npos);
    EXPECT_NE(result.find("discover"), string::npos);
}