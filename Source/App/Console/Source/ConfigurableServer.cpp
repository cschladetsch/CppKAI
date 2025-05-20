#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

// For JSON parsing
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// Include KAI Network and RakNetStub
#include "KAI/Network/RakNetStub.h"

namespace pt = boost::property_tree;

// Define custom message types for communication
enum CustomMessageTypes {
    ID_CUSTOM_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1,
    ID_CALCULATION_REQUEST = RakNet::ID_USER_PACKET_ENUM + 2,
    ID_CALCULATION_RESPONSE = RakNet::ID_USER_PACKET_ENUM + 3
};

// Simple calculation function
int performCalculation(const std::string& expression) {
    // Very simple parser for "a+b" expressions
    std::regex pattern("(\\d+)\\s*\\+\\s*(\\d+)");
    std::smatch matches;

    if (std::regex_search(expression, matches, pattern) &&
        matches.size() == 3) {
        int a = std::stoi(matches[1].str());
        int b = std::stoi(matches[2].str());
        return a + b;
    }

    // For unrecognized expressions, return -1
    return -1;
}

// Server config
struct ServerConfig {
    int port = 14591;
    int maxClients = 32;
    bool enableCalculation = true;
    bool echoMode = true;

    // Load config from JSON file
    bool loadFromFile(const std::string& filename) {
        try {
            pt::ptree tree;
            pt::read_json(filename, tree);

            port = tree.get<int>("port", port);
            maxClients = tree.get<int>("maxClients", maxClients);
            enableCalculation =
                tree.get<bool>("enableCalculation", enableCalculation);
            echoMode = tree.get<bool>("echoMode", echoMode);

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char** argv) {
    std::cout << "KAI Configurable Network Server\n\n";

    ServerConfig config;

    // Check for config file
    if (argc > 1) {
        config.loadFromFile(argv[1]);
    } else {
        // Try default config locations
        // First check config directory
        std::ifstream configFileInConfigDir("config/server_config.json");
        if (configFileInConfigDir.good()) {
            configFileInConfigDir.close();
            config.loadFromFile("config/server_config.json");
        } else {
            // Then try root directory for backward compatibility
            std::ifstream configFile("server_config.json");
            if (configFile.good()) {
                configFile.close();
                config.loadFromFile("server_config.json");
            }
        }
    }

    // Allow command-line override of port
    if (argc > 2) {
        try {
            config.port = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number. Using configured port "
                      << config.port << std::endl;
        }
    }

    // Display configuration
    std::cout << "Server Configuration:\n";
    std::cout << "  Port: " << config.port << "\n";
    std::cout << "  Max Clients: " << config.maxClients << "\n";
    std::cout << "  Calculation Enabled: "
              << (config.enableCalculation ? "Yes" : "No") << "\n";
    std::cout << "  Echo Mode: " << (config.echoMode ? "Yes" : "No") << "\n\n";

    // Initialize RakNet
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    if (!peer) {
        std::cerr << "Failed to create RakNet peer interface\n";
        return 1;
    }

    // Start server
    RakNet::SocketDescriptor sd(config.port, nullptr);
    RakNet::StartupResult result = peer->Startup(config.maxClients, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::cerr << "Failed to start server, error code: " << result << "\n";
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    peer->SetMaximumIncomingConnections(config.maxClients);

    std::cout << "Server started on port " << config.port << "\n";
    std::cout << "Server running. Press Ctrl+C to exit.\n\n";

    // Main server loop
    while (true) {
        // Process messages
        RakNet::Packet* packet;
        while ((packet = peer->Receive()) != nullptr) {
            switch (packet->data[0]) {
                case RakNet::ID_NEW_INCOMING_CONNECTION:
                    std::cout << "Client connected from "
                              << packet->systemAddress.ToString() << std::endl;

                    {
                        // Send a welcome message
                        RakNet::BitStream bs;
                        bs.Write((RakNet::MessageID)ID_CUSTOM_MESSAGE);
                        bs.Write(std::string(
                            "Welcome to the KAI calculation server!"));
                        peer->Send(&bs, 0, 0, 0, packet->systemAddress, false);
                    }
                    break;

                case RakNet::ID_DISCONNECTION_NOTIFICATION:
                    std::cout << "Client disconnected: "
                              << packet->systemAddress.ToString() << std::endl;
                    break;

                case RakNet::ID_CONNECTION_LOST:
                    std::cout << "Connection lost: "
                              << packet->systemAddress.ToString() << std::endl;
                    break;

                case ID_CUSTOM_MESSAGE: {
                    RakNet::BitStream bs(packet->data, packet->length, false);
                    bs.IgnoreBytes(sizeof(RakNet::MessageID));

                    std::string message;
                    bs.Read(message);

                    std::cout << "Message from "
                              << packet->systemAddress.ToString() << ": "
                              << message << std::endl;

                    // Echo the message back if echo mode is enabled
                    if (config.echoMode) {
                        RakNet::BitStream response;
                        response.Write((RakNet::MessageID)ID_CUSTOM_MESSAGE);
                        response.Write(
                            std::string("Server echoed: " + message));
                        peer->Send(&response, 0, 0, 0, packet->systemAddress,
                                   false);
                    }

                    // Check if this looks like a calculation request
                    if (config.enableCalculation &&
                        message.find("+") != std::string::npos) {
                        int result = performCalculation(message);
                        if (result != -1) {
                            // Send calculation result
                            RakNet::BitStream calcResponse;
                            calcResponse.Write(
                                (RakNet::MessageID)ID_CALCULATION_RESPONSE);
                            calcResponse.Write(std::string(message));
                            calcResponse.Write(result);
                            peer->Send(&calcResponse, 0, 0, 0,
                                       packet->systemAddress, false);

                            std::cout << "Sent calculation result: " << message
                                      << " = " << result << std::endl;
                        }
                    }
                } break;

                case ID_CALCULATION_REQUEST: {
                    if (config.enableCalculation) {
                        RakNet::BitStream bs(packet->data, packet->length,
                                             false);
                        bs.IgnoreBytes(sizeof(RakNet::MessageID));

                        std::string expression;
                        bs.Read(expression);

                        std::cout << "Calculation request from "
                                  << packet->systemAddress.ToString() << ": "
                                  << expression << std::endl;

                        // Perform calculation
                        int result = performCalculation(expression);

                        // Send result back
                        RakNet::BitStream response;
                        response.Write(
                            (RakNet::MessageID)ID_CALCULATION_RESPONSE);
                        response.Write(expression);
                        response.Write(result);
                        peer->Send(&response, 0, 0, 0, packet->systemAddress,
                                   false);

                        std::cout << "Sent calculation result: " << expression
                                  << " = " << result << std::endl;
                    }
                } break;
            }

            peer->DeallocatePacket(packet);
        }

        // Display server status every ~5 seconds
        static int counter = 0;
        if (++counter % 50 == 0) {
            // In a real implementation, this would show connected clients
            std::cout << "Calculation server is running..." << std::endl;
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up - in practice this won't be reached due to the infinite loop
    peer->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer);

    return 0;
}