#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "KAI/Core/Object.h"
#include "KAI/Core/Registry.h"
#include "KAI/Network/Network.h"
#include "TestCommon.h"

using namespace kai;
using namespace std;
using namespace std::chrono;

// Mock implementation of chat interfaces for testing
namespace kai {
namespace Chat {

// Mock ChatMessage implementation
struct MockChatMessage {
    int type;
    string channelName;
    string senderId;
    string senderName;
    string content;
    long timestamp;

    MockChatMessage() : type(0), timestamp(0) {}
    MockChatMessage(const string& channel, const string& sender,
                    const string& msg)
        : type(0),
          channelName(channel),
          senderId(sender),
          senderName(sender),
          content(msg),
          timestamp(duration_cast<milliseconds>(
                        system_clock::now().time_since_epoch())
                        .count()) {}
};

// Mock ChannelInfo implementation
struct MockChannelInfo {
    string channelName;
    string hostId;
    string hostAddress;
    long createdTime;
    int memberCount;
    bool isPublic;

    MockChannelInfo() : createdTime(0), memberCount(0), isPublic(true) {}
    MockChannelInfo(const string& name, const string& host)
        : channelName(name),
          hostId(host),
          hostAddress("127.0.0.1"),
          createdTime(duration_cast<milliseconds>(
                          system_clock::now().time_since_epoch())
                          .count()),
          memberCount(1),
          isPublic(true) {}
};

// Mock ChannelUser implementation
struct MockChannelUser {
    string username;
    string userId;
    string address;
    long joinedTime;
    bool isHost;

    MockChannelUser() : joinedTime(0), isHost(false) {}
    MockChannelUser(const string& name, bool host = false)
        : username(name),
          userId(name + "_id"),
          address("127.0.0.1"),
          joinedTime(duration_cast<milliseconds>(
                         system_clock::now().time_since_epoch())
                         .count()),
          isHost(host) {}
};

// Mock implementation of IChannelChat
class MockChannelChat {
   private:
    bool initialized;
    bool running;
    string username;
    string currentChannel;
    bool isHost;
    vector<MockChatMessage> messageHistory;
    vector<MockChannelInfo> discoveredChannels;
    vector<MockChannelUser> channelUsers;

   public:
    // Callbacks for testing
    function<void(MockChatMessage)> onMessageReceived;
    function<void(MockChannelInfo)> onChannelDiscovered;
    function<void(MockChannelUser)> onUserJoined;
    function<void(string, string)> onUserLeft;
    function<void(string)> onChannelClosed;

    MockChannelChat() : initialized(false), running(false), isHost(false) {}

    bool Initialize(int port = 14589) {
        initialized = true;
        running = true;
        return true;
    }

    void Shutdown() {
        if (isHost && !currentChannel.empty()) {
            if (onChannelClosed) onChannelClosed(currentChannel);
        }
        running = false;
        initialized = false;
        currentChannel.clear();
        isHost = false;
    }

    bool IsRunning() const { return running; }

    bool SetUsername(const string& name) {
        if (name.empty()) return false;
        username = name;
        return true;
    }

    string GetUsername() const { return username; }
    bool HasUsername() const { return !username.empty(); }

    bool Publish(const string& channelName) {
        if (!HasUsername() || channelName.empty()) return false;
        if (!currentChannel.empty()) Leave();

        currentChannel = channelName;
        isHost = true;

        // Add self as host
        channelUsers.clear();
        channelUsers.push_back(MockChannelUser(username, true));

        // Add to discovered channels
        auto info = MockChannelInfo(channelName, username);
        discoveredChannels.push_back(info);

        if (onChannelDiscovered) onChannelDiscovered(info);

        return true;
    }

    bool Enter(const string& channelName) {
        if (!HasUsername() || channelName.empty()) return false;
        if (!currentChannel.empty()) Leave();

        // Find the channel
        auto it = find_if(discoveredChannels.begin(), discoveredChannels.end(),
                          [&](const MockChannelInfo& info) {
                              return info.channelName == channelName;
                          });

        if (it == discoveredChannels.end()) return false;

        currentChannel = channelName;
        isHost = false;

        // Add self as member
        auto user = MockChannelUser(username, false);
        channelUsers.push_back(user);
        it->memberCount++;

        if (onUserJoined) onUserJoined(user);

        return true;
    }

    bool Leave() {
        if (currentChannel.empty()) return false;

        if (onUserLeft) onUserLeft(username + "_id", username);

        if (isHost) {
            // Remove channel from discovered list
            discoveredChannels.erase(
                remove_if(discoveredChannels.begin(), discoveredChannels.end(),
                          [this](const MockChannelInfo& info) {
                              return info.channelName == currentChannel;
                          }),
                discoveredChannels.end());

            if (onChannelClosed) onChannelClosed(currentChannel);
        }

        currentChannel.clear();
        isHost = false;
        channelUsers.clear();
        return true;
    }

    vector<MockChannelInfo> Discover() { return discoveredChannels; }

    bool Send(const string& message) {
        if (currentChannel.empty() || !HasUsername()) return false;

        auto msg = MockChatMessage(currentChannel, username, message);
        messageHistory.push_back(msg);

        if (onMessageReceived) onMessageReceived(msg);

        return true;
    }

    string GetCurrentChannel() const { return currentChannel; }
    bool IsInChannel() const { return !currentChannel.empty(); }
    bool IsHosting() const { return isHost; }

    MockChannelInfo GetChannelInfo(const string& channelName) {
        auto it = find_if(discoveredChannels.begin(), discoveredChannels.end(),
                          [&](const MockChannelInfo& info) {
                              return info.channelName == channelName;
                          });
        return (it != discoveredChannels.end()) ? *it : MockChannelInfo();
    }

    vector<MockChannelUser> GetChannelUsers() { return channelUsers; }

    // Test helper methods
    void SimulateUserJoin(const string& newUser) {
        if (currentChannel.empty()) return;

        auto user = MockChannelUser(newUser, false);
        channelUsers.push_back(user);

        // Update channel info
        auto it = find_if(discoveredChannels.begin(), discoveredChannels.end(),
                          [this](const MockChannelInfo& info) {
                              return info.channelName == currentChannel;
                          });
        if (it != discoveredChannels.end()) {
            it->memberCount++;
        }

        if (onUserJoined) onUserJoined(user);
    }

    void SimulateIncomingMessage(const string& sender, const string& message) {
        if (currentChannel.empty()) return;

        auto msg = MockChatMessage(currentChannel, sender, message);
        messageHistory.push_back(msg);

        if (onMessageReceived) onMessageReceived(msg);
    }

    vector<MockChatMessage> GetMessageHistory() const { return messageHistory; }
};

}  // namespace Chat
}  // namespace kai

// Test fixture for Chat functionality
class ChatFunctionalityTests : public TestCommon {
   protected:
    unique_ptr<Chat::MockChannelChat> chat1;
    unique_ptr<Chat::MockChannelChat> chat2;

    void SetUp() override {
        TestCommon::SetUp();
        chat1 = make_unique<Chat::MockChannelChat>();
        chat2 = make_unique<Chat::MockChannelChat>();
    }

    void TearDown() override {
        if (chat1) chat1->Shutdown();
        if (chat2) chat2->Shutdown();
        TestCommon::TearDown();
    }
};

// Test basic initialization and shutdown
TEST_F(ChatFunctionalityTests, InitializationAndShutdown) {
    EXPECT_FALSE(chat1->IsRunning());

    EXPECT_TRUE(chat1->Initialize());
    EXPECT_TRUE(chat1->IsRunning());

    chat1->Shutdown();
    EXPECT_FALSE(chat1->IsRunning());
}

// Test username management
TEST_F(ChatFunctionalityTests, UsernameManagement) {
    chat1->Initialize();

    // Initially no username
    EXPECT_FALSE(chat1->HasUsername());
    EXPECT_EQ("", chat1->GetUsername());

    // Set username
    EXPECT_TRUE(chat1->SetUsername("Alice"));
    EXPECT_TRUE(chat1->HasUsername());
    EXPECT_EQ("Alice", chat1->GetUsername());

    // Empty username should fail
    EXPECT_FALSE(chat1->SetUsername(""));
    EXPECT_EQ("Alice", chat1->GetUsername());  // Should remain unchanged
}

// Test channel publishing
TEST_F(ChatFunctionalityTests, ChannelPublishing) {
    chat1->Initialize();

    // Cannot publish without username
    EXPECT_FALSE(chat1->Publish("TestChannel"));

    // Set username and publish
    chat1->SetUsername("Alice");
    EXPECT_TRUE(chat1->Publish("TestChannel"));

    EXPECT_TRUE(chat1->IsInChannel());
    EXPECT_TRUE(chat1->IsHosting());
    EXPECT_EQ("TestChannel", chat1->GetCurrentChannel());

    // Check channel info
    auto channels = chat1->Discover();
    EXPECT_EQ(1, channels.size());
    EXPECT_EQ("TestChannel", channels[0].channelName);
    EXPECT_EQ("Alice", channels[0].hostId);
}

// Test channel joining
TEST_F(ChatFunctionalityTests, ChannelJoining) {
    chat1->Initialize();
    chat2->Initialize();

    // Alice creates a channel
    chat1->SetUsername("Alice");
    EXPECT_TRUE(chat1->Publish("TestChannel"));

    // Bob cannot join without username
    EXPECT_FALSE(chat2->Enter("TestChannel"));

    // Bob sets username but channel not in discovered list
    chat2->SetUsername("Bob");
    EXPECT_FALSE(chat2->Enter("TestChannel"));

    // Simulate channel discovery
    chat2->discoveredChannels = chat1->Discover();

    // Now Bob can join
    EXPECT_TRUE(chat2->Enter("TestChannel"));
    EXPECT_TRUE(chat2->IsInChannel());
    EXPECT_FALSE(chat2->IsHosting());
    EXPECT_EQ("TestChannel", chat2->GetCurrentChannel());
}

// Test message sending
TEST_F(ChatFunctionalityTests, MessageSending) {
    chat1->Initialize();
    chat1->SetUsername("Alice");
    chat1->Publish("TestChannel");

    // Track received messages
    vector<Chat::MockChatMessage> receivedMessages;
    chat1->onMessageReceived = [&](Chat::MockChatMessage msg) {
        receivedMessages.push_back(msg);
    };

    // Send a message
    EXPECT_TRUE(chat1->Send("Hello, World!"));

    // Check message was received
    EXPECT_EQ(1, receivedMessages.size());
    EXPECT_EQ("TestChannel", receivedMessages[0].channelName);
    EXPECT_EQ("Alice", receivedMessages[0].senderName);
    EXPECT_EQ("Hello, World!", receivedMessages[0].content);

    // Cannot send without being in channel
    chat1->Leave();
    EXPECT_FALSE(chat1->Send("This should fail"));
}

// Test user join/leave notifications
TEST_F(ChatFunctionalityTests, UserJoinLeaveNotifications) {
    chat1->Initialize();
    chat1->SetUsername("Alice");
    chat1->Publish("TestChannel");

    // Track user events
    vector<Chat::MockChannelUser> joinedUsers;
    vector<pair<string, string>> leftUsers;

    chat1->onUserJoined = [&](Chat::MockChannelUser user) {
        joinedUsers.push_back(user);
    };

    chat1->onUserLeft = [&](string id, string name) {
        leftUsers.push_back({id, name});
    };

    // Simulate Bob joining
    chat1->SimulateUserJoin("Bob");

    EXPECT_EQ(1, joinedUsers.size());
    EXPECT_EQ("Bob", joinedUsers[0].username);
    EXPECT_FALSE(joinedUsers[0].isHost);

    // Check user list
    auto users = chat1->GetChannelUsers();
    EXPECT_EQ(2, users.size());  // Alice (host) + Bob
}

// Test channel discovery
TEST_F(ChatFunctionalityTests, ChannelDiscovery) {
    chat1->Initialize();
    chat2->Initialize();

    // Initially no channels
    EXPECT_EQ(0, chat1->Discover().size());
    EXPECT_EQ(0, chat2->Discover().size());

    // Alice creates channel 1
    chat1->SetUsername("Alice");
    chat1->Publish("Channel1");

    // Bob creates channel 2
    chat2->SetUsername("Bob");
    chat2->Publish("Channel2");

    // Check discovered channels
    auto aliceChannels = chat1->Discover();
    auto bobChannels = chat2->Discover();

    EXPECT_EQ(1, aliceChannels.size());
    EXPECT_EQ("Channel1", aliceChannels[0].channelName);

    EXPECT_EQ(1, bobChannels.size());
    EXPECT_EQ("Channel2", bobChannels[0].channelName);
}

// Test channel closure
TEST_F(ChatFunctionalityTests, ChannelClosure) {
    chat1->Initialize();
    chat1->SetUsername("Alice");
    chat1->Publish("TestChannel");

    // Track channel closure
    string closedChannel;
    chat1->onChannelClosed = [&](string channel) { closedChannel = channel; };

    // Leave as host should close channel
    chat1->Leave();

    EXPECT_EQ("TestChannel", closedChannel);
    EXPECT_FALSE(chat1->IsInChannel());
    EXPECT_FALSE(chat1->IsHosting());

    // Channel should be removed from discovery
    EXPECT_EQ(0, chat1->Discover().size());
}

// Test multiple users in channel
TEST_F(ChatFunctionalityTests, MultipleUsersInChannel) {
    chat1->Initialize();
    chat1->SetUsername("Alice");
    chat1->Publish("ChatRoom");

    // Simulate multiple users joining
    chat1->SimulateUserJoin("Bob");
    chat1->SimulateUserJoin("Charlie");
    chat1->SimulateUserJoin("Diana");

    auto users = chat1->GetChannelUsers();
    EXPECT_EQ(4, users.size());  // Alice + 3 others

    // Check channel info
    auto info = chat1->GetChannelInfo("ChatRoom");
    EXPECT_EQ(4, info.memberCount);
}

// Test message history
TEST_F(ChatFunctionalityTests, MessageHistory) {
    chat1->Initialize();
    chat1->SetUsername("Alice");
    chat1->Publish("TestChannel");

    // Send multiple messages
    chat1->Send("Message 1");
    chat1->Send("Message 2");
    chat1->Send("Message 3");

    // Simulate incoming messages
    chat1->SimulateIncomingMessage("Bob", "Hello Alice!");
    chat1->SimulateIncomingMessage("Charlie", "Hey everyone!");

    auto history = chat1->GetMessageHistory();
    EXPECT_EQ(5, history.size());

    // Verify message order and content
    EXPECT_EQ("Message 1", history[0].content);
    EXPECT_EQ("Alice", history[0].senderName);

    EXPECT_EQ("Hello Alice!", history[3].content);
    EXPECT_EQ("Bob", history[3].senderName);
}

// Test concurrent channel operations
TEST_F(ChatFunctionalityTests, ConcurrentChannelOperations) {
    const int numClients = 5;
    vector<unique_ptr<Chat::MockChannelChat>> clients;

    // Create multiple clients
    for (int i = 0; i < numClients; i++) {
        auto client = make_unique<Chat::MockChannelChat>();
        client->Initialize();
        client->SetUsername("User" + to_string(i));
        clients.push_back(move(client));
    }

    // First client creates channel
    clients[0]->Publish("MainChannel");

    // Simulate all clients discovering the channel
    auto channels = clients[0]->Discover();
    for (int i = 1; i < numClients; i++) {
        clients[i]->discoveredChannels = channels;
    }

    // All other clients join
    for (int i = 1; i < numClients; i++) {
        EXPECT_TRUE(clients[i]->Enter("MainChannel"));
    }

    // Everyone sends a message
    for (int i = 0; i < numClients; i++) {
        EXPECT_TRUE(clients[i]->Send("Hello from User" + to_string(i)));
    }
}

// Test error conditions
TEST_F(ChatFunctionalityTests, ErrorConditions) {
    chat1->Initialize();

    // Cannot perform operations without username
    EXPECT_FALSE(chat1->Publish("Channel"));
    EXPECT_FALSE(chat1->Enter("Channel"));
    EXPECT_FALSE(chat1->Send("Message"));

    chat1->SetUsername("Alice");

    // Cannot enter non-existent channel
    EXPECT_FALSE(chat1->Enter("NonExistentChannel"));

    // Cannot send message without being in channel
    EXPECT_FALSE(chat1->Send("Message"));

    // Empty channel name
    EXPECT_FALSE(chat1->Publish(""));
    EXPECT_FALSE(chat1->Enter(""));
}

// Test channel switching
TEST_F(ChatFunctionalityTests, ChannelSwitching) {
    chat1->Initialize();
    chat1->SetUsername("Alice");

    // Create first channel
    EXPECT_TRUE(chat1->Publish("Channel1"));
    EXPECT_EQ("Channel1", chat1->GetCurrentChannel());
    EXPECT_TRUE(chat1->IsHosting());

    // Switch to another channel (should leave first)
    EXPECT_TRUE(chat1->Publish("Channel2"));
    EXPECT_EQ("Channel2", chat1->GetCurrentChannel());
    EXPECT_TRUE(chat1->IsHosting());

    // First channel should be closed
    auto channels = chat1->Discover();
    EXPECT_EQ(1, channels.size());
    EXPECT_EQ("Channel2", channels[0].channelName);
}