#include <KAI/Core/Debug.h>
#include <KAI/Network/ConnectionManager.h>
#include <KAI/Network/Node.h>
#include <KAI/Network/Serialization.h>
#include <Source/SystemAddress.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace kai;
using namespace kai::net;

void RunServer() {
    std::cout << "Starting server...\n";

    Node node;
    node.Listen(14589);

    std::cout << "Server listening on port 14589\n";
    std::cout << "Starting peer discovery...\n";

    // Start peer discovery
    node.StartDiscovery();

    // Set discovery callback
    node.SetPeerDiscoveryCallback([](const RakNet::SystemAddress& address) {
        char str[64];
        address.ToString(false, str);
        std::cout << "Discovered peer: " << str << std::endl;
    });

    // Main server loop
    int counter = 0;
    while (true) {
        node.Update();

        // Print connection info every few seconds
        if (counter % 50 == 0) {
            std::cout << "Connected peers: " << node.GetConnectionCount()
                      << std::endl;
            auto peers = node.GetConnections();
            for (const auto& peer : peers) {
                char str[64];
                peer.ToString(false, str);
                std::cout << "  Peer: " << str << std::endl;
            }
        }

        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RunClient() {
    std::cout << "Starting client...\n";

    Node node;

    // First find servers through discovery
    node.StartDiscovery();

    std::cout << "Discovering servers...\n";

    // Wait for discovery
    int discoveryAttempts = 0;
    bool connected = false;

    while (!connected && discoveryAttempts < 50) {
        node.Update();

        auto discoveredPeers = node.GetDiscoveredPeers();
        if (!discoveredPeers.empty()) {
            // Try to connect to the first discovered peer
            const auto& serverAddress = discoveredPeers[0];

            // Extract IP and port
            char ipStr[64];
            serverAddress.ToString(false, ipStr);
            std::cout << "Connecting to server: " << ipStr << std::endl;

            IpAddress ip = ipStr;

            node.Connect(ip, serverAddress.GetPort());
            connected = true;
        }

        discoveryAttempts++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!connected) {
        std::cout << "Could not discover any server. Trying to connect to "
                     "localhost...\n";
        node.Connect("127.0.0.1", 14589);
    }

    // Main client loop
    int counter = 0;
    while (true) {
        node.Update();

        // Print connection status every few seconds
        if (counter % 50 == 0) {
            if (node.GetConnectionCount() > 0) {
                std::cout << "Connected to server!\n";
            } else {
                std::cout << "Not connected to server.\n";
            }
        }

        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: NetworkTest [server|client]\n";
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "server") {
            RunServer();
        } else if (mode == "client") {
            RunClient();
        } else {
            std::cout << "Invalid mode. Use 'server' or 'client'.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}