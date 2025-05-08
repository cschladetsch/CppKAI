#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// Include KAI Network and RakNetStub
#include "KAI/Network/RakNetStub.h"

// Implement RakString class for proper string handling
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

// Define custom message types for communication
enum CustomMessageTypes { ID_CUSTOM_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1 };

int main(int argc, char** argv) {
    cout << "KAI Minimal Network Server\n\n";

    // Parse command line arguments
    int port = 14591;  // Default port
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            cerr << "Invalid port number. Using default port " << port << endl;
        }
    }

    // Initialize RakNet
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    if (!peer) {
        cerr << "Failed to create RakNet peer interface\n";
        return 1;
    }

    // Start server
    RakNet::SocketDescriptor sd(port, nullptr);
    RakNet::StartupResult result = peer->Startup(32, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        cerr << "Failed to start server, error code: " << result << "\n";
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    peer->SetMaximumIncomingConnections(32);

    cout << "Server started on port " << port << "\n";
    cout << "Server running. Press Ctrl+C to exit.\n\n";

    // Main server loop
    while (true) {
        // Process messages
        RakNet::Packet* packet;
        while ((packet = peer->Receive()) != nullptr) {
            switch (packet->data[0]) {
                case RakNet::ID_NEW_INCOMING_CONNECTION:
                    cout << "Client connected from "
                         << packet->systemAddress.ToString() << endl;

                    {
                        // Send a welcome message
                        RakNet::BitStream bs;
                        bs.Write((RakNet::MessageID)ID_CUSTOM_MESSAGE);
                        bs.Write(
                            std::string("Welcome to the KAI minimal server!"));
                        peer->Send(&bs, 0, 0, 0, packet->systemAddress, false);
                    }
                    break;

                case RakNet::ID_DISCONNECTION_NOTIFICATION:
                    cout << "Client disconnected: "
                         << packet->systemAddress.ToString() << endl;
                    break;

                case RakNet::ID_CONNECTION_LOST:
                    cout << "Connection lost: "
                         << packet->systemAddress.ToString() << endl;
                    break;

                case ID_CUSTOM_MESSAGE: {
                    RakNet::BitStream bs(packet->data, packet->length, false);
                    bs.IgnoreBytes(sizeof(RakNet::MessageID));

                    std::string message;
                    bs.Read(message);

                    cout << "Message from " << packet->systemAddress.ToString()
                         << ": " << message << endl;

                    // Echo the message back
                    RakNet::BitStream response;
                    response.Write((RakNet::MessageID)ID_CUSTOM_MESSAGE);
                    response.Write(std::string("Server echoed: " + message));
                    peer->Send(&response, 0, 0, 0, packet->systemAddress,
                               false);
                } break;
            }

            peer->DeallocatePacket(packet);
        }

        // Display minimal server status every ~5 seconds
        static int counter = 0;
        if (++counter % 50 == 0) {
            // Get currently connected clients (implementation dependent on
            // RakNetStub)
            unsigned int numConnections =
                0;  // In a full implementation, this would be
                    // peer->NumberOfConnections()

            // In our minimal version, just show we're alive
            cout << "Minimal server is running..." << endl;

            // If we had connections, we'd display them like this:
            if (numConnections > 0) {
                cout << "Connected clients: " << numConnections << endl;
            }
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up - in practice this won't be reached due to the infinite loop
    peer->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer);

    return 0;
}