#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// This demonstrates the advanced chat features from our tests

// Advanced message types
enum class MessageType { Text, Join, Leave, ChannelInfo };

// Channel information
struct ChannelInfo {
    std::string name;
    std::string host;
    int memberCount;

    ChannelInfo() : memberCount(0) {}
    ChannelInfo(const std::string& n, const std::string& h)
        : name(n), host(h), memberCount(1) {}
};

// Chat message with metadata
struct ChatMessage {
    MessageType type;
    std::string channel;
    std::string sender;
    std::string content;

    ChatMessage(MessageType t, const std::string& ch, const std::string& s,
                const std::string& c)
        : type(t), channel(ch), sender(s), content(c) {}
};

// Advanced chat implementation with events
class AdvancedChat {
   private:
    std::string username_;
    std::string currentChannel_;
    bool isHost_ = false;
    std::vector<std::string> channelUsers_;
    std::map<std::string, ChannelInfo> availableChannels_;

   public:
    // Event callbacks
    std::function<void(ChatMessage)> onMessageReceived_;
    std::function<void(std::string)> onUserJoined_;
    std::function<void(std::string)> onUserLeft_;
    std::function<void(ChannelInfo)> onChannelDiscovered_;

    AdvancedChat(const std::string& name) : username_(name) {}

    // Publish (create) a new channel
    bool Publish(const std::string& channelName) {
        if (channelName.empty() || username_.empty()) return false;

        // Leave current channel if any
        if (!currentChannel_.empty()) Leave();

        currentChannel_ = channelName;
        isHost_ = true;
        channelUsers_.clear();
        channelUsers_.push_back(username_);

        // Add to available channels
        ChannelInfo info(channelName, username_);
        availableChannels_[channelName] = info;

        // Trigger discovery event
        if (onChannelDiscovered_) {
            onChannelDiscovered_(info);
        }

        std::cout << "[SYSTEM] " << username_ << " created channel '"
                  << channelName << "'\n";
        return true;
    }

    // Enter (join) an existing channel
    bool Enter(const std::string& channelName) {
        if (channelName.empty() || username_.empty()) return false;

        // Check if channel exists
        auto it = availableChannels_.find(channelName);
        if (it == availableChannels_.end()) {
            std::cout << "[ERROR] Channel '" << channelName << "' not found\n";
            return false;
        }

        // Leave current channel if any
        if (!currentChannel_.empty()) Leave();

        currentChannel_ = channelName;
        isHost_ = false;
        channelUsers_.push_back(username_);
        it->second.memberCount++;

        // Notify others
        ChatMessage joinMsg(MessageType::Join, channelName, username_,
                            username_ + " joined the channel");
        if (onMessageReceived_) onMessageReceived_(joinMsg);
        if (onUserJoined_) onUserJoined_(username_);

        std::cout << "[SYSTEM] " << username_ << " joined channel '"
                  << channelName << "'\n";
        return true;
    }

    // Send a message to current channel
    void Send(const std::string& message) {
        if (currentChannel_.empty()) {
            std::cout << "[ERROR] Not in any channel. Use 'publish' or 'enter' "
                         "first.\n";
            return;
        }

        ChatMessage msg(MessageType::Text, currentChannel_, username_, message);
        if (onMessageReceived_) onMessageReceived_(msg);

        std::cout << "[" << currentChannel_ << "] " << username_ << ": "
                  << message << "\n";
    }

    // Leave current channel
    void Leave() {
        if (currentChannel_.empty()) return;

        std::cout << "[SYSTEM] " << username_ << " left channel '"
                  << currentChannel_ << "'\n";

        // Notify others
        ChatMessage leaveMsg(MessageType::Leave, currentChannel_, username_,
                             username_ + " left the channel");
        if (onMessageReceived_) onMessageReceived_(leaveMsg);
        if (onUserLeft_) onUserLeft_(username_);

        // If host, remove channel
        if (isHost_) {
            availableChannels_.erase(currentChannel_);
            std::cout << "[SYSTEM] Channel '" << currentChannel_
                      << "' closed (host left)\n";
        } else {
            // Update member count
            auto it = availableChannels_.find(currentChannel_);
            if (it != availableChannels_.end()) {
                it->second.memberCount--;
            }
        }

        currentChannel_.clear();
        isHost_ = false;
        channelUsers_.clear();
    }

    // Discover available channels
    std::vector<ChannelInfo> Discover() {
        std::vector<ChannelInfo> channels;
        for (const auto& pair : availableChannels_) {
            channels.push_back(pair.second);
        }
        return channels;
    }

    // Get current channel users
    std::vector<std::string> Users() { return channelUsers_; }

    // Console command handler
    void HandleCommand(const std::string& input) {
        if (input.empty()) return;

        // Parse command
        if (input[0] == '/') {
            std::string cmd = input.substr(1);

            if (cmd == "help") {
                std::cout
                    << "\nAvailable commands:\n"
                    << "  /username <name>  - Set your username\n"
                    << "  /publish <channel> - Create a new channel\n"
                    << "  /enter <channel>   - Join an existing channel\n"
                    << "  /leave            - Leave current channel\n"
                    << "  /discover         - List available channels\n"
                    << "  /users            - List users in current channel\n"
                    << "  /help             - Show this help\n"
                    << "  <message>         - Send message to current "
                       "channel\n\n";
            } else if (cmd.substr(0, 8) == "username") {
                username_ = cmd.substr(9);
                std::cout << "[SYSTEM] Username set to: " << username_ << "\n";
            } else if (cmd.substr(0, 7) == "publish") {
                Publish(cmd.substr(8));
            } else if (cmd.substr(0, 5) == "enter") {
                Enter(cmd.substr(6));
            } else if (cmd == "leave") {
                Leave();
            } else if (cmd == "discover") {
                auto channels = Discover();
                std::cout << "\nAvailable channels:\n";
                for (const auto& ch : channels) {
                    std::cout << "  - " << ch.name << " (host: " << ch.host
                              << ", users: " << ch.memberCount << ")\n";
                }
                std::cout << "\n";
            } else if (cmd == "users") {
                if (currentChannel_.empty()) {
                    std::cout << "[ERROR] Not in any channel\n";
                } else {
                    auto userList = Users();
                    std::cout << "\nUsers in '" << currentChannel_ << "':\n";
                    for (const auto& user : userList) {
                        std::cout << "  - " << user;
                        if (isHost_ && user == username_)
                            std::cout << " (host)";
                        std::cout << "\n";
                    }
                    std::cout << "\n";
                }
            }
        } else {
            // Regular message
            Send(input);
        }
    }
};

// Simulate a chat session
int main() {
    std::cout << "=== Advanced Chat Demo ===\n";
    std::cout << "Type /help for available commands\n\n";

    // Create chat instance
    auto chat = std::make_unique<AdvancedChat>("");

    // Set up event handlers
    chat->onMessageReceived_ = [](ChatMessage msg) {
        if (msg.type == MessageType::Join) {
            std::cout << "[EVENT] User joined: " << msg.sender << "\n";
        } else if (msg.type == MessageType::Leave) {
            std::cout << "[EVENT] User left: " << msg.sender << "\n";
        }
    };

    chat->onChannelDiscovered_ = [](ChannelInfo info) {
        std::cout << "[EVENT] New channel discovered: " << info.name << "\n";
    };

    // Simulate commands
    std::cout << "Simulating chat session...\n\n";

    chat->HandleCommand("/help");
    chat->HandleCommand("/username Alice");
    chat->HandleCommand("/publish general");
    chat->HandleCommand("Welcome to the general channel!");
    chat->HandleCommand("/users");

    // Simulate Bob joining
    auto bob = std::make_unique<AdvancedChat>("Bob");
    bob->Enter("general");
    bob->Send("Hey Alice!");

    chat->HandleCommand("/discover");
    chat->HandleCommand("How's everyone doing?");

    // Simulate Charlie
    auto charlie = std::make_unique<AdvancedChat>("Charlie");
    charlie->Publish("random");

    chat->HandleCommand("/discover");
    chat->HandleCommand("/leave");
    chat->HandleCommand("/enter random");
    chat->HandleCommand("Hi Charlie!");

    return 0;
}

// To compile and run:
// g++ -std=c++20 chat_demo_advanced.cpp -o chat_demo_advanced
// ./chat_demo_advanced