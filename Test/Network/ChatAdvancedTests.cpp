#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <thread>

#include "KAI/Core/Object.h"
#include "KAI/Core/Registry.h"
#include "TestCommon.h"

using namespace kai;
using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

// Advanced chat functionality with persistence and group features
namespace kai {
namespace Chat {

// Extended message types for advanced features
enum class AdvancedMessageType {
    Text,
    Image,
    File,
    Emoji,
    Reply,
    Edit,
    Delete,
    Reaction,
    SystemNotification
};

// Message metadata for advanced features
struct MessageMetadata {
    string messageId;
    string replyToId;
    vector<string> mentions;
    map<string, string> reactions;  // userId -> emoji
    bool isEdited;
    long editTimestamp;
    bool isDeleted;

    MessageMetadata() : isEdited(false), editTimestamp(0), isDeleted(false) {}
};

// Advanced chat message with metadata
struct AdvancedChatMessage {
    string id;
    AdvancedMessageType type;
    string channelId;
    string senderId;
    string senderName;
    string content;
    long timestamp;
    MessageMetadata metadata;

    AdvancedChatMessage() : type(AdvancedMessageType::Text), timestamp(0) {}

    AdvancedChatMessage(const string& channel, const string& sender,
                        const string& msg)
        : id(GenerateId()),
          type(AdvancedMessageType::Text),
          channelId(channel),
          senderId(sender),
          senderName(sender),
          content(msg),
          timestamp(GetTimestamp()) {}

   private:
    static string GenerateId() {
        static int counter = 0;
        return "msg_" + to_string(++counter);
    }

    static long GetTimestamp() {
        return duration_cast<milliseconds>(
                   system_clock::now().time_since_epoch())
            .count();
    }
};

// Group chat information
struct GroupChatInfo {
    string groupId;
    string groupName;
    string description;
    string creatorId;
    vector<string> adminIds;
    vector<string> memberIds;
    long createdTime;
    map<string, string> settings;
    bool isPrivate;

    GroupChatInfo() : createdTime(0), isPrivate(false) {}

    bool IsAdmin(const string& userId) const {
        return find(adminIds.begin(), adminIds.end(), userId) != adminIds.end();
    }

    bool IsMember(const string& userId) const {
        return find(memberIds.begin(), memberIds.end(), userId) !=
               memberIds.end();
    }
};

// Chat persistence interface
class ChatPersistence {
   private:
    string dataPath;

   public:
    ChatPersistence(const string& path = "./chat_data") : dataPath(path) {
        fs::create_directories(dataPath);
    }

    // Message persistence
    bool SaveMessage(const AdvancedChatMessage& message) {
        string filePath = dataPath + "/" + message.channelId + "_messages.txt";
        ofstream file(filePath, ios::app);
        if (!file) return false;

        file << message.id << "|" << static_cast<int>(message.type) << "|"
             << message.senderId << "|" << message.senderName << "|"
             << message.content << "|" << message.timestamp << "|"
             << message.metadata.isEdited << "|" << message.metadata.isDeleted
             << "\n";

        return true;
    }

    vector<AdvancedChatMessage> LoadMessages(const string& channelId,
                                             int limit = 100) {
        vector<AdvancedChatMessage> messages;
        string filePath = dataPath + "/" + channelId + "_messages.txt";
        ifstream file(filePath);
        if (!file) return messages;

        string line;
        while (getline(file, line) && messages.size() < limit) {
            auto parts = Split(line, '|');
            if (parts.size() >= 8) {
                AdvancedChatMessage msg;
                msg.id = parts[0];
                msg.type = static_cast<AdvancedMessageType>(stoi(parts[1]));
                msg.channelId = channelId;
                msg.senderId = parts[2];
                msg.senderName = parts[3];
                msg.content = parts[4];
                msg.timestamp = stol(parts[5]);
                msg.metadata.isEdited = (parts[6] == "1");
                msg.metadata.isDeleted = (parts[7] == "1");

                if (!msg.metadata.isDeleted) {
                    messages.push_back(msg);
                }
            }
        }

        return messages;
    }

    // Group persistence
    bool SaveGroup(const GroupChatInfo& group) {
        string filePath = dataPath + "/groups.txt";
        ofstream file(filePath, ios::app);
        if (!file) return false;

        file << group.groupId << "|" << group.groupName << "|"
             << group.description << "|" << group.creatorId << "|"
             << Join(group.memberIds, ',') << "|" << group.createdTime << "\n";

        return true;
    }

    vector<GroupChatInfo> LoadGroups() {
        vector<GroupChatInfo> groups;
        string filePath = dataPath + "/groups.txt";
        ifstream file(filePath);
        if (!file) return groups;

        string line;
        while (getline(file, line)) {
            auto parts = Split(line, '|');
            if (parts.size() >= 6) {
                GroupChatInfo group;
                group.groupId = parts[0];
                group.groupName = parts[1];
                group.description = parts[2];
                group.creatorId = parts[3];
                group.memberIds = Split(parts[4], ',');
                group.createdTime = stol(parts[5]);
                groups.push_back(group);
            }
        }

        return groups;
    }

    void ClearData() {
        if (fs::exists(dataPath)) {
            fs::remove_all(dataPath);
            fs::create_directories(dataPath);
        }
    }

   private:
    vector<string> Split(const string& str, char delimiter) {
        vector<string> parts;
        stringstream ss(str);
        string part;
        while (getline(ss, part, delimiter)) {
            parts.push_back(part);
        }
        return parts;
    }

    string Join(const vector<string>& vec, char delimiter) {
        if (vec.empty()) return "";
        return accumulate(
            next(vec.begin()), vec.end(), vec[0],
            [delimiter](string a, string b) { return a + delimiter + b; });
    }
};

// Advanced chat implementation with group and persistence features
class AdvancedChat {
   private:
    string userId;
    string username;
    map<string, GroupChatInfo> groups;
    map<string, vector<AdvancedChatMessage>> messageCache;
    unique_ptr<ChatPersistence> persistence;

   public:
    // Callbacks
    function<void(AdvancedChatMessage)> onMessageReceived;
    function<void(string, string, string)>
        onMessageEdited;  // messageId, channelId, newContent
    function<void(string, string)> onMessageDeleted;  // messageId, channelId
    function<void(string, string, string)>
        onReactionAdded;  // messageId, userId, emoji
    function<void(GroupChatInfo)> onGroupCreated;
    function<void(string, string)> onUserJoinedGroup;  // groupId, userId
    function<void(string, string)> onUserLeftGroup;    // groupId, userId

    AdvancedChat() : persistence(make_unique<ChatPersistence>()) {}

    void SetUser(const string& id, const string& name) {
        userId = id;
        username = name;
    }

    // Group chat management
    string CreateGroup(const string& name, const string& description,
                       bool isPrivate = false) {
        GroupChatInfo group;
        group.groupId = "group_" + to_string(groups.size() + 1);
        group.groupName = name;
        group.description = description;
        group.creatorId = userId;
        group.adminIds.push_back(userId);
        group.memberIds.push_back(userId);
        group.createdTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                .count();
        group.isPrivate = isPrivate;

        groups[group.groupId] = group;
        persistence->SaveGroup(group);

        if (onGroupCreated) onGroupCreated(group);

        return group.groupId;
    }

    bool JoinGroup(const string& groupId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) return false;

        auto& group = it->second;
        if (group.IsMember(userId)) return false;  // Already a member

        group.memberIds.push_back(userId);

        if (onUserJoinedGroup) onUserJoinedGroup(groupId, userId);

        return true;
    }

    bool LeaveGroup(const string& groupId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) return false;

        auto& group = it->second;
        auto memberIt =
            find(group.memberIds.begin(), group.memberIds.end(), userId);
        if (memberIt == group.memberIds.end()) return false;

        group.memberIds.erase(memberIt);

        // Remove from admins if applicable
        auto adminIt =
            find(group.adminIds.begin(), group.adminIds.end(), userId);
        if (adminIt != group.adminIds.end()) {
            group.adminIds.erase(adminIt);
        }

        if (onUserLeftGroup) onUserLeftGroup(groupId, userId);

        return true;
    }

    bool AddGroupMember(const string& groupId, const string& newUserId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) return false;

        auto& group = it->second;
        if (!group.IsAdmin(userId))
            return false;  // Only admins can add members
        if (group.IsMember(newUserId)) return false;  // Already a member

        group.memberIds.push_back(newUserId);

        if (onUserJoinedGroup) onUserJoinedGroup(groupId, newUserId);

        return true;
    }

    bool RemoveGroupMember(const string& groupId, const string& targetUserId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) return false;

        auto& group = it->second;
        if (!group.IsAdmin(userId))
            return false;  // Only admins can remove members
        if (targetUserId == group.creatorId)
            return false;  // Cannot remove creator

        auto memberIt =
            find(group.memberIds.begin(), group.memberIds.end(), targetUserId);
        if (memberIt == group.memberIds.end()) return false;

        group.memberIds.erase(memberIt);

        if (onUserLeftGroup) onUserLeftGroup(groupId, targetUserId);

        return true;
    }

    // Advanced messaging features
    string SendMessage(const string& channelId, const string& content,
                       AdvancedMessageType type = AdvancedMessageType::Text) {
        AdvancedChatMessage msg;
        msg.id =
            "msg_" +
            to_string(chrono::steady_clock::now().time_since_epoch().count());
        msg.type = type;
        msg.channelId = channelId;
        msg.senderId = userId;
        msg.senderName = username;
        msg.content = content;
        msg.timestamp =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                .count();

        messageCache[channelId].push_back(msg);
        persistence->SaveMessage(msg);

        if (onMessageReceived) onMessageReceived(msg);

        return msg.id;
    }

    string ReplyToMessage(const string& channelId, const string& replyToId,
                          const string& content) {
        AdvancedChatMessage msg;
        msg.id =
            "msg_" +
            to_string(chrono::steady_clock::now().time_since_epoch().count());
        msg.type = AdvancedMessageType::Reply;
        msg.channelId = channelId;
        msg.senderId = userId;
        msg.senderName = username;
        msg.content = content;
        msg.timestamp =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                .count();
        msg.metadata.replyToId = replyToId;

        messageCache[channelId].push_back(msg);
        persistence->SaveMessage(msg);

        if (onMessageReceived) onMessageReceived(msg);

        return msg.id;
    }

    bool EditMessage(const string& channelId, const string& messageId,
                     const string& newContent) {
        auto it = messageCache.find(channelId);
        if (it == messageCache.end()) return false;

        auto msgIt = find_if(
            it->second.begin(), it->second.end(),
            [&](const AdvancedChatMessage& m) { return m.id == messageId; });

        if (msgIt == it->second.end()) return false;
        if (msgIt->senderId != userId)
            return false;  // Can only edit own messages

        msgIt->content = newContent;
        msgIt->metadata.isEdited = true;
        msgIt->metadata.editTimestamp =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                .count();

        if (onMessageEdited) onMessageEdited(messageId, channelId, newContent);

        return true;
    }

    bool DeleteMessage(const string& channelId, const string& messageId) {
        auto it = messageCache.find(channelId);
        if (it == messageCache.end()) return false;

        auto msgIt = find_if(
            it->second.begin(), it->second.end(),
            [&](const AdvancedChatMessage& m) { return m.id == messageId; });

        if (msgIt == it->second.end()) return false;

        // Check permissions - can delete own messages or admin in group
        bool canDelete = (msgIt->senderId == userId);
        if (!canDelete) {
            auto groupIt = groups.find(channelId);
            if (groupIt != groups.end()) {
                canDelete = groupIt->second.IsAdmin(userId);
            }
        }

        if (!canDelete) return false;

        msgIt->metadata.isDeleted = true;

        if (onMessageDeleted) onMessageDeleted(messageId, channelId);

        return true;
    }

    bool AddReaction(const string& channelId, const string& messageId,
                     const string& emoji) {
        auto it = messageCache.find(channelId);
        if (it == messageCache.end()) return false;

        auto msgIt = find_if(
            it->second.begin(), it->second.end(),
            [&](const AdvancedChatMessage& m) { return m.id == messageId; });

        if (msgIt == it->second.end()) return false;

        msgIt->metadata.reactions[userId] = emoji;

        if (onReactionAdded) onReactionAdded(messageId, userId, emoji);

        return true;
    }

    // Message search and filtering
    vector<AdvancedChatMessage> SearchMessages(const string& channelId,
                                               const string& query) {
        vector<AdvancedChatMessage> results;
        auto it = messageCache.find(channelId);
        if (it == messageCache.end()) return results;

        for (const auto& msg : it->second) {
            if (!msg.metadata.isDeleted &&
                (msg.content.find(query) != string::npos ||
                 msg.senderName.find(query) != string::npos)) {
                results.push_back(msg);
            }
        }

        return results;
    }

    vector<AdvancedChatMessage> GetMessagesByUser(const string& channelId,
                                                  const string& targetUserId) {
        vector<AdvancedChatMessage> results;
        auto it = messageCache.find(channelId);
        if (it == messageCache.end()) return results;

        for (const auto& msg : it->second) {
            if (!msg.metadata.isDeleted && msg.senderId == targetUserId) {
                results.push_back(msg);
            }
        }

        return results;
    }

    // Persistence operations
    void LoadHistory(const string& channelId) {
        messageCache[channelId] = persistence->LoadMessages(channelId);
    }

    void LoadGroups() {
        auto loadedGroups = persistence->LoadGroups();
        for (const auto& group : loadedGroups) {
            groups[group.groupId] = group;
        }
    }

    void ClearAllData() {
        messageCache.clear();
        groups.clear();
        persistence->ClearData();
    }

    // Getters
    vector<GroupChatInfo> GetGroups() const {
        vector<GroupChatInfo> result;
        for (const auto& pair : groups) {
            result.push_back(pair.second);
        }
        return result;
    }

    GroupChatInfo GetGroup(const string& groupId) const {
        auto it = groups.find(groupId);
        return (it != groups.end()) ? it->second : GroupChatInfo();
    }

    vector<AdvancedChatMessage> GetMessages(const string& channelId) const {
        auto it = messageCache.find(channelId);
        return (it != messageCache.end()) ? it->second
                                          : vector<AdvancedChatMessage>();
    }
};

}  // namespace Chat
}  // namespace kai

// Test fixture for advanced chat features
class ChatAdvancedTests : public TestCommon {
   protected:
    unique_ptr<Chat::AdvancedChat> chat;

    void SetUp() override {
        TestCommon::SetUp();
        chat = make_unique<Chat::AdvancedChat>();
        chat->ClearAllData();  // Start with clean data
    }

    void TearDown() override {
        if (chat) chat->ClearAllData();
        TestCommon::TearDown();
    }
};

// Test group creation and management
TEST_F(ChatAdvancedTests, GroupCreation) {
    chat->SetUser("user1", "Alice");

    // Track group creation
    Chat::GroupChatInfo createdGroup;
    chat->onGroupCreated = [&](Chat::GroupChatInfo group) {
        createdGroup = group;
    };

    // Create a group
    string groupId =
        chat->CreateGroup("Test Group", "A test group for unit tests");

    EXPECT_FALSE(groupId.empty());
    EXPECT_EQ("Test Group", createdGroup.groupName);
    EXPECT_EQ("A test group for unit tests", createdGroup.description);
    EXPECT_EQ("user1", createdGroup.creatorId);
    EXPECT_EQ(1, createdGroup.memberIds.size());
    EXPECT_EQ(1, createdGroup.adminIds.size());
}

// Test group member management
TEST_F(ChatAdvancedTests, GroupMemberManagement) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Test Group", "Test");

    // Track join/leave events
    vector<pair<string, string>> joinEvents;
    vector<pair<string, string>> leaveEvents;

    chat->onUserJoinedGroup = [&](string gId, string uId) {
        joinEvents.push_back({gId, uId});
    };

    chat->onUserLeftGroup = [&](string gId, string uId) {
        leaveEvents.push_back({gId, uId});
    };

    // Simulate another user joining
    chat->SetUser("user2", "Bob");
    EXPECT_TRUE(chat->JoinGroup(groupId));

    EXPECT_EQ(1, joinEvents.size());
    EXPECT_EQ(groupId, joinEvents[0].first);
    EXPECT_EQ("user2", joinEvents[0].second);

    // Cannot join again
    EXPECT_FALSE(chat->JoinGroup(groupId));

    // Leave group
    EXPECT_TRUE(chat->LeaveGroup(groupId));

    EXPECT_EQ(1, leaveEvents.size());
    EXPECT_EQ(groupId, leaveEvents[0].first);
    EXPECT_EQ("user2", leaveEvents[0].second);
}

// Test admin operations
TEST_F(ChatAdvancedTests, GroupAdminOperations) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Admin Test", "Test admin features");

    // Alice (admin) adds Bob
    EXPECT_TRUE(chat->AddGroupMember(groupId, "user2"));

    // Switch to Bob (non-admin)
    chat->SetUser("user2", "Bob");

    // Bob cannot add Charlie
    EXPECT_FALSE(chat->AddGroupMember(groupId, "user3"));

    // Switch back to Alice
    chat->SetUser("user1", "Alice");

    // Alice can remove Bob
    EXPECT_TRUE(chat->RemoveGroupMember(groupId, "user2"));

    // Cannot remove creator
    EXPECT_FALSE(chat->RemoveGroupMember(groupId, "user1"));
}

// Test message sending and receiving
TEST_F(ChatAdvancedTests, AdvancedMessaging) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Chat Group", "Test messaging");

    // Track messages
    vector<Chat::AdvancedChatMessage> receivedMessages;
    chat->onMessageReceived = [&](Chat::AdvancedChatMessage msg) {
        receivedMessages.push_back(msg);
    };

    // Send various message types
    string textId = chat->SendMessage(groupId, "Hello, World!");
    string emojiId =
        chat->SendMessage(groupId, "😀", Chat::AdvancedMessageType::Emoji);

    EXPECT_EQ(2, receivedMessages.size());
    EXPECT_EQ("Hello, World!", receivedMessages[0].content);
    EXPECT_EQ(Chat::AdvancedMessageType::Text, receivedMessages[0].type);
    EXPECT_EQ(Chat::AdvancedMessageType::Emoji, receivedMessages[1].type);
}

// Test reply functionality
TEST_F(ChatAdvancedTests, ReplyToMessage) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Reply Test", "Test replies");

    // Send original message
    string originalId = chat->SendMessage(groupId, "Original message");

    // Reply to it
    string replyId =
        chat->ReplyToMessage(groupId, originalId, "This is a reply");

    auto messages = chat->GetMessages(groupId);
    EXPECT_EQ(2, messages.size());
    EXPECT_EQ(Chat::AdvancedMessageType::Reply, messages[1].type);
    EXPECT_EQ(originalId, messages[1].metadata.replyToId);
}

// Test message editing
TEST_F(ChatAdvancedTests, MessageEditing) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Edit Test", "Test editing");

    // Track edits
    string editedMessageId;
    string newContent;
    chat->onMessageEdited = [&](string msgId, string channelId,
                                string content) {
        editedMessageId = msgId;
        newContent = content;
    };

    // Send and edit message
    string msgId = chat->SendMessage(groupId, "Original content");
    EXPECT_TRUE(chat->EditMessage(groupId, msgId, "Edited content"));

    EXPECT_EQ(msgId, editedMessageId);
    EXPECT_EQ("Edited content", newContent);

    // Check message is marked as edited
    auto messages = chat->GetMessages(groupId);
    EXPECT_TRUE(messages[0].metadata.isEdited);
    EXPECT_EQ("Edited content", messages[0].content);
}

// Test message deletion
TEST_F(ChatAdvancedTests, MessageDeletion) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Delete Test", "Test deletion");

    // Track deletions
    string deletedMessageId;
    chat->onMessageDeleted = [&](string msgId, string channelId) {
        deletedMessageId = msgId;
    };

    // Send messages
    string msgId1 = chat->SendMessage(groupId, "Message 1");
    string msgId2 = chat->SendMessage(groupId, "Message 2");

    // Delete first message
    EXPECT_TRUE(chat->DeleteMessage(groupId, msgId1));
    EXPECT_EQ(msgId1, deletedMessageId);

    // Check message is marked as deleted
    auto messages = chat->GetMessages(groupId);
    EXPECT_TRUE(messages[0].metadata.isDeleted);
}

// Test reactions
TEST_F(ChatAdvancedTests, MessageReactions) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Reaction Test", "Test reactions");

    // Track reactions
    vector<tuple<string, string, string>> reactions;
    chat->onReactionAdded = [&](string msgId, string userId, string emoji) {
        reactions.push_back({msgId, userId, emoji});
    };

    // Send message and add reactions
    string msgId = chat->SendMessage(groupId, "React to this!");

    EXPECT_TRUE(chat->AddReaction(groupId, msgId, "👍"));

    // Switch user and add another reaction
    chat->SetUser("user2", "Bob");
    EXPECT_TRUE(chat->AddReaction(groupId, msgId, "❤️"));

    EXPECT_EQ(2, reactions.size());

    // Check message reactions
    auto messages = chat->GetMessages(groupId);
    EXPECT_EQ(2, messages[0].metadata.reactions.size());
    EXPECT_EQ("👍", messages[0].metadata.reactions["user1"]);
    EXPECT_EQ("❤️", messages[0].metadata.reactions["user2"]);
}

// Test message search
TEST_F(ChatAdvancedTests, MessageSearch) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Search Test", "Test search");

    // Send various messages
    chat->SendMessage(groupId, "Hello world");
    chat->SendMessage(groupId, "Good morning");
    chat->SendMessage(groupId, "Hello again");
    chat->SendMessage(groupId, "Goodbye");

    // Search for "Hello"
    auto results = chat->SearchMessages(groupId, "Hello");
    EXPECT_EQ(2, results.size());
    EXPECT_EQ("Hello world", results[0].content);
    EXPECT_EQ("Hello again", results[1].content);
}

// Test message filtering by user
TEST_F(ChatAdvancedTests, MessageFilterByUser) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Filter Test", "Test filtering");

    // Alice sends messages
    chat->SendMessage(groupId, "Alice message 1");
    chat->SendMessage(groupId, "Alice message 2");

    // Bob sends messages
    chat->SetUser("user2", "Bob");
    chat->JoinGroup(groupId);
    chat->SendMessage(groupId, "Bob message 1");
    chat->SendMessage(groupId, "Bob message 2");
    chat->SendMessage(groupId, "Bob message 3");

    // Filter Alice's messages
    auto aliceMessages = chat->GetMessagesByUser(groupId, "user1");
    EXPECT_EQ(2, aliceMessages.size());

    // Filter Bob's messages
    auto bobMessages = chat->GetMessagesByUser(groupId, "user2");
    EXPECT_EQ(3, bobMessages.size());
}

// Test persistence
TEST_F(ChatAdvancedTests, MessagePersistence) {
    chat->SetUser("user1", "Alice");
    string groupId = chat->CreateGroup("Persistence Test", "Test persistence");

    // Send messages
    chat->SendMessage(groupId, "Persistent message 1");
    chat->SendMessage(groupId, "Persistent message 2");
    chat->SendMessage(groupId, "Persistent message 3");

    // Create new chat instance
    auto newChat = make_unique<Chat::AdvancedChat>();
    newChat->SetUser("user1", "Alice");

    // Load history
    newChat->LoadHistory(groupId);
    auto loadedMessages = newChat->GetMessages(groupId);

    EXPECT_EQ(3, loadedMessages.size());
    EXPECT_EQ("Persistent message 1", loadedMessages[0].content);
    EXPECT_EQ("Persistent message 2", loadedMessages[1].content);
    EXPECT_EQ("Persistent message 3", loadedMessages[2].content);
}

// Test group persistence
TEST_F(ChatAdvancedTests, GroupPersistence) {
    chat->SetUser("user1", "Alice");

    // Create multiple groups
    chat->CreateGroup("Group 1", "First group");
    chat->CreateGroup("Group 2", "Second group");
    chat->CreateGroup("Group 3", "Third group", true);  // Private group

    // Create new chat instance
    auto newChat = make_unique<Chat::AdvancedChat>();
    newChat->LoadGroups();

    auto loadedGroups = newChat->GetGroups();
    EXPECT_EQ(3, loadedGroups.size());

    // Verify group details
    auto group1 = find_if(
        loadedGroups.begin(), loadedGroups.end(),
        [](const Chat::GroupChatInfo& g) { return g.groupName == "Group 1"; });
    EXPECT_NE(loadedGroups.end(), group1);
    EXPECT_EQ("First group", group1->description);
}

// Test complex scenario with multiple users
TEST_F(ChatAdvancedTests, ComplexMultiUserScenario) {
    // Create main group
    chat->SetUser("alice_id", "Alice");
    string groupId = chat->CreateGroup("Team Chat", "Company team chat");

    // Alice sends initial message
    chat->SendMessage(groupId, "Welcome to the team chat!");

    // Bob joins and sends messages
    chat->SetUser("bob_id", "Bob");
    chat->JoinGroup(groupId);
    string bobMsg = chat->SendMessage(groupId, "Thanks for adding me!");
    chat->AddReaction(groupId, bobMsg, "👋");

    // Charlie joins and replies
    chat->SetUser("charlie_id", "Charlie");
    chat->JoinGroup(groupId);
    chat->ReplyToMessage(groupId, bobMsg, "Welcome Bob!");

    // Diana joins and searches
    chat->SetUser("diana_id", "Diana");
    chat->JoinGroup(groupId);
    auto welcomeMessages = chat->SearchMessages(groupId, "Welcome");
    EXPECT_EQ(2, welcomeMessages.size());

    // Back to Alice - check group status
    chat->SetUser("alice_id", "Alice");
    auto group = chat->GetGroup(groupId);
    EXPECT_EQ(4, group.memberIds.size());

    // Alice adds Charlie as admin
    chat->AddGroupMember(groupId, "charlie_id");

    // Charlie can now remove users
    chat->SetUser("charlie_id", "Charlie");
    // Note: In real implementation, would need to promote Charlie to admin
    // first
}