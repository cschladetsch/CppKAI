#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// Include KAI Network and RakNetStub
#include "KAI/Network/RakNetStub.h"

// Implement RakString class
namespace RakNet {
class RakString {
   private:
    std::string str;

   public:
    RakString() {}
    RakString(const char* s) : str(s) {}

    const char* C_String() const { return str.c_str(); }

    // Non-static member operator for concatenation
    RakString operator+(const RakString& b) const {
        RakString result;
        result.str = this->str + b.str;
        return result;
    }
};
}  // namespace RakNet

using namespace std;

enum CustomMessageTypes { ID_CUSTOM_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1 };

int main(int argc, char** argv) {
    cout << "KAI Minimal Network Client Example\n\n";

    // Parse command line arguments
    string serverIp = "127.0.0.1";  // Default to localhost
    int serverPort = 14591;         // Default port

    if (argc > 1) {
        serverIp = argv[1];
    }

    if (argc > 2) {
        try {
            serverPort = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            cerr << "Invalid port number. Using default port " << serverPort
                 << endl;
        }
    }

    // Initialize RakNet
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    if (!peer) {
        cerr << "Failed to create RakNet peer interface\n";
        return 1;
    }

    // Start client
    RakNet::SocketDescriptor sd(0, nullptr);  // Use any available port
    RakNet::StartupResult result = peer->Startup(1, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        cerr << "Failed to start client, error code: " << result << "\n";
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    cout << "Client started on port " << sd.port << "\n";

    // Connect to server
    cout << "Connecting to server at " << serverIp << ":" << serverPort
         << "...\n";
    RakNet::ConnectionAttemptResult connectResult =
        peer->Connect(serverIp.c_str(), serverPort, nullptr, 0);

    if (connectResult != RakNet::CONNECTION_ATTEMPT_STARTED) {
        cerr << "Failed to start connection attempt, error code: "
             << connectResult << "\n";
        peer->Shutdown(0);
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    cout
        << "Client running. Enter messages to send or type 'exit' to quit.\n\n";

    // Message input thread
    bool running = true;
    thread inputThread([&]() {
        while (running) {
            string input;
            cout << "Enter message: ";
            getline(cin, input);

            if (input == "exit") {
                running = false;
                break;
            }

            if (!input.empty()) {
                RakNet::BitStream bs;
                bs.Write((RakNet::MessageID)ID_CUSTOM_MESSAGE);
                // Convert to std::string which BitStream can handle
                bs.Write(std::string(input));

                // Use constants defined in RakNetStub.h for packet priority and
                // reliability Note: These constants aren't defined in the stub
                // currently, using a channel 0
                peer->Send(&bs, 0, 0, 0, RakNet::SystemAddress(), true);

                cout << "Message sent: " << input << endl;
            }
        }
    });

    // Main client loop
    while (running) {
        // Process messages
        RakNet::Packet* packet;
        while ((packet = peer->Receive()) != nullptr) {
            switch (packet->data[0]) {
                case RakNet::ID_CONNECTION_REQUEST_ACCEPTED:
                    cout << "Connected to server at "
                         << packet->systemAddress.ToString() << endl;
                    break;

                case RakNet::ID_DISCONNECTION_NOTIFICATION:
                    cout << "Disconnected from server" << endl;
                    break;

                case RakNet::ID_CONNECTION_LOST:
                    cout << "Connection to server lost" << endl;
                    break;

                case RakNet::ID_CONNECTION_ATTEMPT_FAILED:
                    cout << "Failed to connect to server" << endl;
                    break;

                case ID_CUSTOM_MESSAGE: {
                    RakNet::BitStream bs(packet->data, packet->length, false);
                    bs.IgnoreBytes(sizeof(RakNet::MessageID));

                    std::string message;
                    bs.Read(message);

                    cout << "Message from server: " << message << endl;
                } break;
            }

            peer->DeallocatePacket(packet);
        }

        // Show connection status
        static int counter = 0;
        if (++counter % 50 == 0) {
            // Add connection status check compatible with RakNetStub
            // Note that our stub implementation doesn't have
            // NumberOfConnections or GetSystemAddressFromIndex
            bool isConnected =
                peer->GetConnectionState(RakNet::SystemAddress()) ==
                RakNet::IS_CONNECTED;
            cout << "Connected to server: " << (isConnected ? "Yes" : "No")
                 << endl;

            if (isConnected) {
                // In a real implementation we would have:
                // RakNet::SystemAddress serverAddr =
                // peer->GetSystemAddressFromIndex(0);
                RakNet::SystemAddress
                    serverAddr;  // Use default address since we can't get it
                                 // from the stub
                cout << "  Server address: " << serverAddr.ToString() << endl;
                cout << "  Ping: " << peer->GetAveragePing(serverAddr) << "ms"
                     << endl;
            }
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for input thread to finish
    if (inputThread.joinable()) {
        inputThread.join();
    }

    // Clean up
    peer->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer);

    return 0;
}