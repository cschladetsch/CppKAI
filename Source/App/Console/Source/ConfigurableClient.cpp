#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
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

// Client configuration
struct ClientConfig {
    std::string serverIp = "127.0.0.1";
    int serverPort = 14591;
    bool autoCalculate = false;
    std::string calculationExpression = "1+2";
    bool waitForResult = false;

    // Load config from JSON file
    bool loadFromFile(const std::string& filename) {
        try {
            pt::ptree tree;
            pt::read_json(filename, tree);

            serverIp = tree.get<std::string>("serverIp", serverIp);
            serverPort = tree.get<int>("serverPort", serverPort);
            autoCalculate = tree.get<bool>("autoCalculate", autoCalculate);
            calculationExpression = tree.get<std::string>(
                "calculationExpression", calculationExpression);
            waitForResult = tree.get<bool>("waitForResult", waitForResult);

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            return false;
        }
    }
};

// Globals for calculation result synchronization
std::mutex resultMutex;
std::condition_variable resultCV;
bool resultReceived = false;
int calculationResult = 0;
std::string resultExpression;

int main(int argc, char** argv) {
    std::cout << "KAI Configurable Network Client\n\n";

    ClientConfig config;

    // Check for config file
    if (argc > 1) {
        config.loadFromFile(argv[1]);
    } else {
        // Try default config locations
        // First check config directory
        std::ifstream configFileInConfigDir("config/client_config.json");
        if (configFileInConfigDir.good()) {
            configFileInConfigDir.close();
            config.loadFromFile("config/client_config.json");
        } else {
            // Then try root directory for backward compatibility
            std::ifstream configFile("client_config.json");
            if (configFile.good()) {
                configFile.close();
                config.loadFromFile("client_config.json");
            }
        }
    }

    // Allow command-line override of server IP
    if (argc > 2) {
        config.serverIp = argv[2];
    }

    // Allow command-line override of port
    if (argc > 3) {
        try {
            config.serverPort = std::stoi(argv[3]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number. Using configured port "
                      << config.serverPort << std::endl;
        }
    }

    // Display configuration
    std::cout << "Client Configuration:\n";
    std::cout << "  Server IP: " << config.serverIp << "\n";
    std::cout << "  Server Port: " << config.serverPort << "\n";
    std::cout << "  Auto Calculate: " << (config.autoCalculate ? "Yes" : "No")
              << "\n";
    if (config.autoCalculate) {
        std::cout << "  Expression: " << config.calculationExpression << "\n";
        std::cout << "  Wait for result: "
                  << (config.waitForResult ? "Yes" : "No") << "\n";
    }
    std::cout << "\n";

    // Initialize RakNet
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    if (!peer) {
        std::cerr << "Failed to create RakNet peer interface\n";
        return 1;
    }

    // Start client
    RakNet::SocketDescriptor sd(0, nullptr);  // Use any available port
    RakNet::StartupResult result = peer->Startup(1, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::cerr << "Failed to start client, error code: " << result << "\n";
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    std::cout << "Client started\n";

    // Connect to server
    std::cout << "Connecting to server at " << config.serverIp << ":"
              << config.serverPort << "...\n";
    RakNet::ConnectionAttemptResult connectResult =
        peer->Connect(config.serverIp.c_str(), config.serverPort, nullptr, 0);

    if (connectResult != RakNet::CONNECTION_ATTEMPT_STARTED) {
        std::cerr << "Failed to start connection attempt, error code: "
                  << connectResult << "\n";
        peer->Shutdown(0);
        RakNet::RakPeerInterface::DestroyInstance(peer);
        return 1;
    }

    std::cout << "Client running. ";
    if (!config.autoCalculate) {
        std::cout << "Enter expressions like '1+2' to calculate, or type "
                     "'exit' to quit.\n\n";
    } else {
        std::cout << "Automatic calculation mode enabled.\n\n";
    }

    // Flag to track if we're connected and sent the auto-calculation
    bool connected = false;
    bool autoCalculationSent = false;
    std::atomic<bool> running(true);

    // Message input thread
    std::thread inputThread([&]() {
        if (config.autoCalculate) {
            // In auto calculate mode, we don't need an input thread
            return;
        }

        while (running) {
            std::string input;
            std::cout << "Enter expression: ";
            std::getline(std::cin, input);

            if (input == "exit") {
                running = false;
                break;
            }

            if (!input.empty()) {
                // Send as calculation request
                RakNet::BitStream bs;
                bs.Write((RakNet::MessageID)ID_CALCULATION_REQUEST);
                bs.Write(input);

                // Send to server
                peer->Send(&bs, 0, 0, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS,
                           true);

                std::cout << "Calculation request sent: " << input << std::endl;
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
                    std::cout << "Connected to server at "
                              << packet->systemAddress.ToString() << std::endl;
                    connected = true;
                    break;

                case RakNet::ID_DISCONNECTION_NOTIFICATION:
                    std::cout << "Disconnected from server" << std::endl;
                    connected = false;
                    break;

                case RakNet::ID_CONNECTION_LOST:
                    std::cout << "Connection to server lost" << std::endl;
                    connected = false;
                    break;

                case RakNet::ID_CONNECTION_ATTEMPT_FAILED:
                    std::cout << "Failed to connect to server" << std::endl;
                    running = false;
                    break;

                case ID_CUSTOM_MESSAGE: {
                    RakNet::BitStream bs(packet->data, packet->length, false);
                    bs.IgnoreBytes(sizeof(RakNet::MessageID));

                    std::string message;
                    bs.Read(message);

                    std::cout << "Message from server: " << message
                              << std::endl;
                } break;

                case ID_CALCULATION_RESPONSE: {
                    RakNet::BitStream bs(packet->data, packet->length, false);
                    bs.IgnoreBytes(sizeof(RakNet::MessageID));

                    std::string expression;
                    int result;
                    bs.Read(expression);
                    bs.Read(result);

                    std::cout << "Calculation result: " << expression << " = "
                              << result << std::endl;

                    // Store result for synchronization
                    {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        resultReceived = true;
                        calculationResult = result;
                        resultExpression = expression;
                    }
                    resultCV.notify_all();

                    // If this was our auto-calculation and we're only waiting
                    // for the result, we can exit now
                    if (config.autoCalculate && config.waitForResult &&
                        expression == config.calculationExpression) {
                        running = false;
                    }
                } break;
            }

            peer->DeallocatePacket(packet);
        }

        // Send automatic calculation once connected
        if (config.autoCalculate && connected && !autoCalculationSent) {
            std::cout << "Sending automatic calculation request: "
                      << config.calculationExpression << std::endl;

            // Send as calculation request
            RakNet::BitStream bs;
            bs.Write((RakNet::MessageID)ID_CALCULATION_REQUEST);
            bs.Write(config.calculationExpression);

            // Send to server
            peer->Send(&bs, 0, 0, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

            autoCalculationSent = true;

            // If not waiting for result, can exit immediately
            if (!config.waitForResult) {
                std::this_thread::sleep_for(std::chrono::seconds(
                    1));  // Small delay to allow packet to be sent
                running = false;
            }
        }

        // Sleep to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for input thread to finish if it was started
    if (inputThread.joinable()) {
        inputThread.join();
    }

    // Clean up
    peer->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer);

    // Return the calculation result if in auto calculate mode
    if (config.autoCalculate && resultReceived) {
        std::cout << "\nFinal result: " << resultExpression << " = "
                  << calculationResult << std::endl;
        return calculationResult == 3
                   ? 0
                   : 1;  // For testing, return success if result is 3
    }

    return 0;
}