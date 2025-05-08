#include <chrono>
#include <iostream>
#include <thread>

#include "KAI/Network/Network.h"

using namespace std;
using namespace kai;
using namespace kai::net;

int main(int argc, char** argv) {
    cout << "KAI Simple Network Server\n\n";

    // Parse command line arguments
    int port = 14591;  // Default port
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            cerr << "Invalid port number. Using default port " << port << endl;
        }
    }

    // Create a network node
    auto node = std::make_unique<Node>();

    // Start listening on the specified port
    cout << "Starting server on port " << port << "...\n";
    node->Listen(port);

    // Start peer discovery so clients can find this server
    cout << "Starting peer discovery...\n";
    node->StartDiscovery();

    // Main server loop
    cout << "Server running. Press Ctrl+C to exit.\n";
    while (true) {
        // Update the network node
        node->Update();

        // Display connection status
        static int counter = 0;
        if (++counter % 20 == 0) {  // Every ~2 seconds
            auto connectionCount = node->GetConnectionCount();
            if (connectionCount > 0) {
                cout << "Connected clients: " << connectionCount << endl;
                auto connections = node->GetConnections();
                for (const auto& address : connections) {
                    IpAddress ipAddress(address.ToString());
                    int port = address.port;
                    int ping = node->GetPing(ipAddress, port);

                    cout << "  Client: " << address.ToString()
                         << " (ping: " << ping << "ms)" << endl;
                }
            }
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}