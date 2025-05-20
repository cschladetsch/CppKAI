#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// For JSON parsing and configuration
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// Include KAI Network with RakNet adapter (will use either real or stub
// implementation)
#include "KAI/Network/RakNetAdapter.h"

namespace pt = boost::property_tree;

// Define custom message types for P2P communication
enum CustomMessageTypes {
    ID_P2P_COMMAND = RakNet::ID_USER_PACKET_ENUM + 1,
    ID_P2P_RESULT = RakNet::ID_USER_PACKET_ENUM + 2,
    ID_P2P_OBJECT_SYNC = RakNet::ID_USER_PACKET_ENUM + 3
};

// Command processor function type
using CommandProcessor = std::function<std::string(const std::string&)>;

// Simple calculation function (a basic command processor)
std::string performCalculation(const std::string& expression) {
    // Simple parser for "a+b" expressions
    std::regex pattern("(\\d+)\\s*\\+\\s*(\\d+)");
    std::smatch matches;

    if (std::regex_search(expression, matches, pattern) &&
        matches.size() == 3) {
        int a = std::stoi(matches[1].str());
        int b = std::stoi(matches[2].str());
        return std::to_string(a + b);
    }

    // For unrecognized expressions
    return "Error: Unsupported expression";
}

// Peer configuration
struct PeerConfig {
    int listenPort = 14590;
    bool autoListen = true;
    std::vector<std::pair<std::string, int>> peers;
    std::map<std::string, std::string> commands;
    bool interactive = true;

    // Load config from JSON
    bool loadFromFile(const std::string& filename) {
        try {
            pt::ptree tree;
            pt::read_json(filename, tree);

            listenPort = tree.get<int>("listenPort", listenPort);
            autoListen = tree.get<bool>("autoListen", autoListen);
            interactive = tree.get<bool>("interactive", interactive);

            // Load peers to connect to
            if (tree.count("peers") > 0) {
                for (const auto& peer : tree.get_child("peers")) {
                    std::string host = peer.second.get<std::string>("host");
                    int port = peer.second.get<int>("port");
                    peers.push_back(std::make_pair(host, port));
                }
            }

            // Load commands to register
            if (tree.count("commands") > 0) {
                for (const auto& cmd : tree.get_child("commands")) {
                    std::string name = cmd.first;
                    std::string expr = cmd.second.get_value<std::string>();
                    commands[name] = expr;
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            return false;
        }
    }
};

class NetworkPeer {
   private:
    RakNet::RakPeerInterface* peer;
    PeerConfig config;
    bool running = true;
    std::map<std::string, CommandProcessor> commandProcessors;
    std::mutex peersMutex;
    std::vector<RakNet::SystemAddress> connectedPeers;

    // Command history for interactive mode
    std::vector<std::string> commandHistory;

   public:
    NetworkPeer(const PeerConfig& cfg) : config(cfg), peer(nullptr) {
        // Initialize RakNet
        peer = RakNet::RakPeerInterface::GetInstance();
        if (!peer) {
            throw std::runtime_error("Failed to create RakNet peer interface");
        }

        // Register default command processors
        registerCommandProcessor("calc", [](const std::string& cmd) {
            return performCalculation(cmd);
        });

        // Register commands from config
        for (const auto& cmd : config.commands) {
            std::cout << "Registering predefined command: " << cmd.first
                      << " = " << cmd.second << std::endl;
        }
    }

    ~NetworkPeer() {
        Shutdown();
        if (peer) {
            RakNet::RakPeerInterface::DestroyInstance(peer);
        }
    }

    void Start() {
        // Start listening if configured
        if (config.autoListen) {
            StartListening();
        }

        // Connect to configured peers
        for (const auto& peerInfo : config.peers) {
            std::string host = peerInfo.first;
            int port = peerInfo.second;
            std::cout << "Connecting to peer at " << host << ":" << port
                      << std::endl;
            ConnectToPeer(host, port);
        }

        // Start message processing thread
        std::thread messageThread(&NetworkPeer::ProcessMessages, this);

        // Start command input thread if interactive
        if (config.interactive) {
            std::thread commandThread(&NetworkPeer::ProcessUserCommands, this);
            commandThread.join();
        } else {
            // In non-interactive mode, just wait for messages
            messageThread.join();
        }
    }

    void StartListening() {
        std::cout << "Starting to listen on port " << config.listenPort
                  << std::endl;

        RakNet::SocketDescriptor sd(config.listenPort, nullptr);
        RakNet::StartupResult result = peer->Startup(32, &sd, 1);

        if (result != RakNet::RAKNET_STARTED) {
            std::cerr << "Failed to start peer listener, error code: " << result
                      << std::endl;
            return;
        }

        peer->SetMaximumIncomingConnections(32);
        std::cout << "Listening for connections on port " << config.listenPort
                  << std::endl;
    }

    void ConnectToPeer(const std::string& host, int port) {
        if (!peer) return;

        RakNet::ConnectionAttemptResult result =
            peer->Connect(host.c_str(), port, nullptr, 0);

        if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
            std::cerr << "Failed to connect to " << host << ":" << port
                      << ", error code: " << result << std::endl;
        } else {
            std::cout << "Connection attempt to " << host << ":" << port
                      << " started" << std::endl;
        }
    }

    void ProcessMessages() {
        while (running) {
            RakNet::Packet* packet;
            while ((packet = peer->Receive()) != nullptr) {
                HandlePacket(packet);
                peer->DeallocatePacket(packet);
            }

            // Sleep to prevent high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // For testing with stub implementation: Simulate receiving packets
#ifdef KAI_USE_RAKNET_STUB
            // Only simulate for a few seconds after startup to avoid continuous
            // simulation
            static int simulationCount = 0;
            if (simulationCount < 30) {  // Simulate for about 30 iterations
                if (simulationCount == 5) {
                    // Simulate connection acceptance from peer #1
                    std::lock_guard<std::mutex> lock(peersMutex);
                    RakNet::SystemAddress addr("127.0.0.1", 14595);
                    connectedPeers.push_back(addr);

                    // Create a packet to simulate connection
                    RakNet::Packet* simulatedPacket = new RakNet::Packet();
                    simulatedPacket->systemAddress = addr;
                    simulatedPacket->length = 1;
                    simulatedPacket->data = new unsigned char[1];
                    simulatedPacket->data[0] =
                        RakNet::ID_CONNECTION_REQUEST_ACCEPTED;

                    // Handle the packet
                    HandlePacket(simulatedPacket);
                    delete[] simulatedPacket->data;
                    delete simulatedPacket;
                } else if (simulationCount == 25 && !connectedPeers.empty()) {
                    // Simulate receiving a result
                    std::lock_guard<std::mutex> lock(peersMutex);
                    if (!connectedPeers.empty()) {
                        // Create a simulated result packet
                        RakNet::BitStream bs;
                        bs.Write((RakNet::MessageID)ID_P2P_RESULT);
                        std::string command = "add";
                        std::string result = "3";
                        bs.Write(command);
                        bs.Write(result);

                        // Create the packet
                        RakNet::Packet* resultPacket = new RakNet::Packet();
                        resultPacket->systemAddress = connectedPeers[0];
                        resultPacket->length = bs.GetNumberOfBytesUsed();
                        resultPacket->data =
                            new unsigned char[resultPacket->length];
                        memcpy(resultPacket->data, bs.GetData(),
                               resultPacket->length);

                        // Handle the packet
                        HandlePacket(resultPacket);
                        delete[] resultPacket->data;
                        delete resultPacket;
                    }
                }
                simulationCount++;
            }
#endif
        }
    }

    void ProcessUserCommands() {
        std::cout << "P2P Network Console\n";
        std::cout << "Type 'help' for available commands, 'exit' to quit\n";

        std::string line;
        while (running && std::getline(std::cin, line)) {
            if (line.empty()) continue;

            // Add to command history
            commandHistory.push_back(line);

            // Exit command
            if (line == "exit" || line == "quit") {
                running = false;
                break;
            }

            // Help command
            if (line == "help") {
                PrintHelp();
                continue;
            }

            // Connect command - format: connect host port
            if (line.find("connect ") == 0) {
                std::string params = line.substr(8);
                std::istringstream iss(params);
                std::string host;
                int port;

                if (iss >> host >> port) {
                    ConnectToPeer(host, port);
                } else {
                    std::cout << "Usage: connect <host> <port>" << std::endl;
                }
                continue;
            }

            // List connected peers
            if (line == "peers") {
                ListConnectedPeers();
                continue;
            }

            // Remote command execution - format: @peer command args
            if (line.at(0) == '@') {
                size_t spacePos = line.find(' ');
                if (spacePos == std::string::npos) {
                    std::cout << "Usage: @peer command [args]" << std::endl;
                    continue;
                }

                std::string peerName = line.substr(1, spacePos - 1);
                std::string command = line.substr(spacePos + 1);

                SendCommandToPeer(peerName, command);
                continue;
            }

            // Process as local command
            std::string result = ProcessLocalCommand(line);
            std::cout << "Result: " << result << std::endl;
        }
    }

    void HandlePacket(RakNet::Packet* packet) {
        unsigned char packetId = packet->data[0];

        switch (packetId) {
            case RakNet::ID_NEW_INCOMING_CONNECTION:
                std::cout << "Peer connected from "
                          << packet->systemAddress.ToString() << std::endl;
                // Track the new connection
                {
                    std::lock_guard<std::mutex> lock(peersMutex);
                    connectedPeers.push_back(packet->systemAddress);
                }
                break;

            case RakNet::ID_DISCONNECTION_NOTIFICATION:
                std::cout << "Peer disconnected: "
                          << packet->systemAddress.ToString() << std::endl;
                RemovePeer(packet->systemAddress);
                break;

            case RakNet::ID_CONNECTION_LOST:
                std::cout << "Connection lost to peer: "
                          << packet->systemAddress.ToString() << std::endl;
                RemovePeer(packet->systemAddress);
                break;

            case RakNet::ID_CONNECTION_REQUEST_ACCEPTED:
                std::cout << "Connected to peer: "
                          << packet->systemAddress.ToString() << std::endl;
                // Track the new connection
                {
                    std::lock_guard<std::mutex> lock(peersMutex);
                    connectedPeers.push_back(packet->systemAddress);
                }
                break;

            case ID_P2P_COMMAND: {
                // Process command from peer
                RakNet::BitStream bs(packet->data, packet->length, false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));

                std::string command;
                bs.Read(command);

                std::cout << "Command from " << packet->systemAddress.ToString()
                          << ": " << command << std::endl;

                // Process the command
                std::string result = ProcessLocalCommand(command);

                // Send result back
                RakNet::BitStream response;
                response.Write((RakNet::MessageID)ID_P2P_RESULT);
                response.Write(command);
                response.Write(result);
                peer->Send(&response, 0, 0, 0, packet->systemAddress, false);

                std::cout << "Sent result: " << result << std::endl;
                break;
            }

            case ID_P2P_RESULT: {
                // Receive result from command
                RakNet::BitStream bs(packet->data, packet->length, false);
                bs.IgnoreBytes(sizeof(RakNet::MessageID));

                std::string command, result;
                bs.Read(command);
                bs.Read(result);

                std::cout << "Result from " << packet->systemAddress.ToString()
                          << " for command '" << command << "': " << result
                          << std::endl;
                break;
            }
        }
    }

    void registerCommandProcessor(const std::string& commandType,
                                  CommandProcessor processor) {
        commandProcessors[commandType] = processor;
    }

    std::string ProcessLocalCommand(const std::string& command) {
        // Check for predefined commands in config
        for (const auto& cmd : config.commands) {
            if (command == cmd.first) {
                return ProcessLocalCommand(cmd.second);
            }
        }

        // Extract command type and arguments
        size_t spacePos = command.find(' ');
        std::string cmdType = (spacePos == std::string::npos)
                                  ? command
                                  : command.substr(0, spacePos);
        std::string args =
            (spacePos == std::string::npos) ? "" : command.substr(spacePos + 1);

        // Find processor for command type
        auto it = commandProcessors.find(cmdType);
        if (it != commandProcessors.end()) {
            return it->second(args);
        }

        // Default to calculation for expressions
        if (command.find('+') != std::string::npos) {
            return performCalculation(command);
        }

        return "Unknown command: " + command;
    }

    void SendCommandToPeer(const std::string& peerName,
                           const std::string& command) {
        RakNet::SystemAddress targetPeer;
        bool found = false;

        // Lock for thread safety
        {
            std::lock_guard<std::mutex> lock(peersMutex);

            // First check if peerName is an index
            if (std::all_of(peerName.begin(), peerName.end(), ::isdigit)) {
                int index = std::stoi(peerName);
                if (index >= 0 && index < connectedPeers.size()) {
                    targetPeer = connectedPeers[index];
                    found = true;
                }
            } else {
                // Try to match by IP or hostname
                for (const auto& peer : connectedPeers) {
                    if (peer.ToString().find(peerName) != std::string::npos) {
                        targetPeer = peer;
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) {
            std::cout << "No peer found matching '" << peerName << "'"
                      << std::endl;
            return;
        }

        // Send command to the peer
        RakNet::BitStream bs;
        bs.Write((RakNet::MessageID)ID_P2P_COMMAND);
        bs.Write(command);
        peer->Send(&bs, 0, 0, 0, targetPeer, false);

        std::cout << "Sent command '" << command << "' to "
                  << targetPeer.ToString() << std::endl;
    }

    void RemovePeer(const RakNet::SystemAddress& address) {
        std::lock_guard<std::mutex> lock(peersMutex);
        connectedPeers.erase(
            std::remove(connectedPeers.begin(), connectedPeers.end(), address),
            connectedPeers.end());
    }

    void ListConnectedPeers() {
        std::lock_guard<std::mutex> lock(peersMutex);

        if (connectedPeers.empty()) {
            std::cout << "No peers connected." << std::endl;
            return;
        }

        std::cout << "Connected peers:" << std::endl;
        for (size_t i = 0; i < connectedPeers.size(); ++i) {
            std::cout << "  " << i << ": " << connectedPeers[i].ToString()
                      << std::endl;
        }
    }

    void PrintHelp() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  help       - Display this help message" << std::endl;
        std::cout << "  exit/quit  - Exit the program" << std::endl;
        std::cout << "  connect <host> <port> - Connect to a peer" << std::endl;
        std::cout << "  peers      - List connected peers" << std::endl;
        std::cout << "  @<peer> <command> - Execute command on remote peer"
                  << std::endl;
        std::cout << "  calc <expression> - Calculate simple expressions"
                  << std::endl;

        // List predefined commands from config
        if (!config.commands.empty()) {
            std::cout << "Predefined commands:" << std::endl;
            for (const auto& cmd : config.commands) {
                std::cout << "  " << cmd.first << " - " << cmd.second
                          << std::endl;
            }
        }
    }

    void Shutdown() {
        if (peer) {
            peer->Shutdown(300);
        }
    }
};

int main(int argc, char** argv) {
    try {
        PeerConfig config;

        // Check for config file
        if (argc > 1) {
            config.loadFromFile(argv[1]);
        } else {
            // Try default config location
            std::ifstream configFile("peer_config.json");
            if (configFile.good()) {
                configFile.close();
                config.loadFromFile("peer_config.json");
            }
        }

        // Allow command-line override of listen port
        if (argc > 2) {
            try {
                config.listenPort = std::stoi(argv[2]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid port number. Using configured port "
                          << config.listenPort << std::endl;
            }
        }

        // Create and start the peer
        NetworkPeer peer(config);
        peer.Start();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}