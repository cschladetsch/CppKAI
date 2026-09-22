#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// This is a working example of the chat system from the tests

// Simple chat message structure
struct ChatMessage {
    std::string channel;
    std::string sender;
    std::string content;
    long timestamp;

    ChatMessage(std::string ch, std::string s, std::string c)
        : channel(std::move(ch)), sender(std::move(s)), content(std::move(c))
    {
        timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    }
};

// Simple chat client implementation
class ChatClient {
   private:
    std::string username_;
    std::string currentChannel_;
    bool isHost_ = false;
    std::vector<ChatMessage> messages_;

   public:
       ChatClient(std::string name) : username_(std::move(name)) {}

       // Create a new channel
       bool CreateChannel(const std::string& channelName)
       {
           if (channelName.empty()) {
               return false;
           }

           currentChannel_ = channelName;
           isHost_ = true;

           std::cout << "[" << username_ << "] Created channel: " << channelName << '\n';
           return true;
       }

    // Join an existing channel
    bool JoinChannel(const std::string& channelName) {
        if (channelName.empty()) {
            return false;
        }

        currentChannel_ = channelName;
        isHost_ = false;

        std::cout << "[" << username_ << "] Joined channel: " << channelName << '\n';
        return true;
    }

    // Send a message
    void SendMessage(const std::string& message) {
        if (currentChannel_.empty()) {
            std::cout << "[" << username_ << "] Error: Not in a channel!" << '\n';
            return;
        }

        ChatMessage msg(currentChannel_, username_, message);
        messages_.push_back(msg);

        std::cout << "[" << currentChannel_ << "] " << username_ << ": " << message << '\n';
    }

    // Leave current channel
    void LeaveChannel() {
        if (!currentChannel_.empty()) {
            std::cout << "[" << username_ << "] Left channel: " << currentChannel_ << '\n';
            currentChannel_.clear();
            isHost_ = false;
        }
    }

    // Get message history
    void ShowHistory() {
        std::cout << "\n=== Message History ===\n";
        for (const auto& msg : messages_) {
            std::cout << "[" << msg.channel << "] " << msg.sender << ": " << msg.content << '\n';
        }
        std::cout << "=====================\n\n";
    }
};

// Example of using the chat system
int main() {
    std::cout << "=== Chat System Example ===\n\n";

    // Create chat clients
    auto alice = std::make_unique<ChatClient>("Alice");
    auto bob = std::make_unique<ChatClient>("Bob");
    auto charlie = std::make_unique<ChatClient>("Charlie");

    // Alice creates a channel
    alice->CreateChannel("general");

    // Alice sends a message
    alice->SendMessage("Welcome to the general channel!");

    // Bob joins the channel
    bob->JoinChannel("general");
    bob->SendMessage("Hi Alice!");

    // Charlie joins too
    charlie->JoinChannel("general");
    charlie->SendMessage("Hey everyone!");

    // More conversation
    alice->SendMessage("How is everyone doing?");
    bob->SendMessage("Great! Just testing the chat system.");
    charlie->SendMessage("This is pretty cool!");

    // Show Alice's message history
    std::cout << "\nAlice's view:\n";
    alice->ShowHistory();

    // Bob creates a private channel
    bob->CreateChannel("private-chat");
    bob->SendMessage("This is a private channel");

    // Charlie leaves general
    charlie->LeaveChannel();

    // Alice sends another message
    alice->SendMessage("Where did Charlie go?");

    return 0;
}

// To compile and run:
// g++ -std=c++20 chat_example.cpp -o chat_example
// ./chat_example