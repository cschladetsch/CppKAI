#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "KAI/Network/Network.h"

using namespace std;
using namespace kai;
using namespace kai::net;

int main(int argc, char** argv) {
    cout << "KAI Simple Network Client\n\n";

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

    // Create a network node
    auto node = std::make_unique<Node>();

    // Start listening on any available port
    cout << "Starting client...\n";
    node->Listen(0);  // 0 means any available port
    // Note: There's no direct way to get the listening port currently
    cout << "Client initialized on random available port" << endl;

    // Connect to the server
    cout << "Connecting to server at " << serverIp << ":" << serverPort
         << "...\n";
    node->Connect(IpAddress(serverIp), serverPort);

    // Main client loop
    cout << "Client running. Press Ctrl+C to exit.\n";
    while (true) {
        // Update the network node
        node->Update();

        // Display connection status
        static int counter = 0;
        if (++counter % 20 == 0) {  // Every ~2 seconds
            if (node->GetConnectionCount() > 0) {
                cout << "Connected to server: Yes\n";

                // Display the first connection (the server)
                auto connections = node->GetConnections();
                if (!connections.empty()) {
                    IpAddress serverAddr(connections[0].ToString());
                    int port = connections[0].port;
                    int ping = node->GetPing(serverAddr, port);

                    cout << "  Server: " << connections[0].ToString()
                         << " (ping: " << ping << "ms)" << endl;
                }
            } else {
                cout << "Connected to server: No\n";
            }
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}