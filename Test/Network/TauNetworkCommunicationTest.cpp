#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <memory>
#include <future>
#include <regex>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"

using namespace kai;
using namespace std;
namespace fs = std::filesystem;

/**
 * Comprehensive test suite for Tau-based network communication
 * Tests the complete workflow from .tau IDL files to working network communication
 */
class TauNetworkCommunicationTest : public ::testing::Test {
protected:
    string testDir;
    string tauFile;
    string proxyFile;
    string agentFile;
    string networkGenerateExe;

    void SetUp() override {
        // Create test directory
        testDir = "/tmp/tau_network_test";
        fs::create_directories(testDir);
        
        // Set up file paths
        tauFile = testDir + "/TestService.tau";
        proxyFile = testDir + "/TestService.proxy.h";
        agentFile = testDir + "/TestService.agent.h";
        
        // Path to NetworkGenerate executable
        networkGenerateExe = "/home/christian/local/repos/KAI/build/NetworkGenerate";
        
        KAI_LOG_INFO("TauNetworkCommunicationTest setup complete - test directory: " + testDir);
    }

    void TearDown() override {
        // Clean up test files
        try {
            if (fs::exists(testDir)) {
                fs::remove_all(testDir);
            }
        } catch (const fs::filesystem_error& e) {
            KAI_LOG_WARNING("Failed to clean up test directory: " + string(e.what()));
        }
    }

    // Create a comprehensive Tau interface definition
    void CreateTestTauInterface() {
        ofstream file(tauFile);
        ASSERT_TRUE(file.is_open()) << "Failed to create test Tau file";
        
        file << R"(// TestService.tau - Comprehensive test interface for network communication
// Tests all major Tau IDL features: structs, interfaces, methods, events

namespace TestNetwork {
    // Data structures for network communication
    struct RequestData {
        int requestId;
        string operation;
        float value1;
        float value2;
        int64 timestamp;
    }
    
    struct ResponseData {
        int requestId;
        bool success;
        float result;
        string errorMessage;
        int64 processTime;
    }
    
    struct ClientInfo {
        string clientId;
        string name;
        string version;
        int64 connectedAt;
    }
    
    // Core calculator service interface
    interface ICalculatorService {
        // Synchronous operations
        float Add(float a, float b);
        float Subtract(float a, float b);
        float Multiply(float a, float b);
        float Divide(float a, float b);
        
        // Asynchronous batch operations
        ResponseData ProcessRequest(RequestData request);
        ResponseData[] ProcessBatch(RequestData[] requests);
        
        // Service management
        void RegisterClient(ClientInfo client);
        void UnregisterClient(string clientId);
        ClientInfo[] GetActiveClients();
        
        // State management
        void Clear();
        float GetLastResult();
        string GetServiceStatus();
        
        // Events for notifications
        event CalculationCompleted(ResponseData response);
        event ClientRegistered(ClientInfo client);
        event ClientDisconnected(string clientId);
        event ServiceStatusChanged(string status);
        event ErrorOccurred(string error);
    }
    
    // Client callback interface for bidirectional communication
    interface ICalculatorClient {
        // Client-side methods called by server
        void NotifyResult(ResponseData result);
        void RequestHeartbeat();
        void ServiceShuttingDown(string reason);
        
        // Client identification
        ClientInfo GetClientInfo();
        string GetClientId();
        
        // Events for client-side notifications
        event ServerConnected(string serverId);
        event ServerDisconnected(string reason);
        event HeartbeatReceived(int64 timestamp);
    }
}
)";
        
        file.close();
        KAI_LOG_INFO("Created comprehensive Tau interface file: " + tauFile);
    }

    // Generate proxy and agent files using NetworkGenerate
    bool GenerateProxyAndAgent() {
        // Construct NetworkGenerate command
        string command = networkGenerateExe + " " + tauFile + " --out=" + testDir;
        
        KAI_LOG_INFO("Executing NetworkGenerate: " + command);
        
        // Execute NetworkGenerate
        int result = system(command.c_str());
        
        if (result != 0) {
            KAI_LOG_ERROR("NetworkGenerate failed with exit code: " + to_string(result));
            return false;
        }
        
        // Verify generated files exist
        bool proxyExists = fs::exists(proxyFile);
        bool agentExists = fs::exists(agentFile);
        
        KAI_LOG_INFO("Generated files - Proxy: " + string(proxyExists ? "EXISTS" : "MISSING") + 
                    ", Agent: " + string(agentExists ? "EXISTS" : "MISSING"));
        
        return proxyExists && agentExists;
    }

    // Verify generated proxy file contains expected content
    void VerifyProxyGeneration() {
        ASSERT_TRUE(fs::exists(proxyFile)) << "Proxy file was not generated: " << proxyFile;
        
        ifstream file(proxyFile);
        ASSERT_TRUE(file.is_open()) << "Cannot read generated proxy file";
        
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        
        KAI_LOG_INFO("Generated proxy size: " + to_string(content.size()) + " bytes");
        
        // Verify basic structure
        EXPECT_TRUE(content.find("namespace TestNetwork") != string::npos)
            << "Proxy should contain TestNetwork namespace";
        
        // Verify proxy classes were generated
        EXPECT_TRUE(content.find("ICalculatorServiceProxy") != string::npos ||
                   content.find("class ICalculatorService") != string::npos)
            << "Proxy should contain calculator service class";
        
        EXPECT_TRUE(content.find("ICalculatorClientProxy") != string::npos ||
                   content.find("class ICalculatorClient") != string::npos)
            << "Proxy should contain calculator client class";
        
        // Verify methods were generated
        EXPECT_TRUE(content.find("Add") != string::npos)
            << "Proxy should contain Add method";
        EXPECT_TRUE(content.find("ProcessRequest") != string::npos)
            << "Proxy should contain ProcessRequest method";
        
        // Verify network includes
        EXPECT_TRUE(content.find("RakNet/BitStream.h") != string::npos ||
                   content.find("BitStream") != string::npos)
            << "Proxy should include networking headers";
    }

    // Verify generated agent file contains expected content  
    void VerifyAgentGeneration() {
        ASSERT_TRUE(fs::exists(agentFile)) << "Agent file was not generated: " << agentFile;
        
        ifstream file(agentFile);
        ASSERT_TRUE(file.is_open()) << "Cannot read generated agent file";
        
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        
        KAI_LOG_INFO("Generated agent size: " + to_string(content.size()) + " bytes");
        
        // Verify basic structure
        EXPECT_TRUE(content.find("namespace TestNetwork") != string::npos)
            << "Agent should contain TestNetwork namespace";
        
        // Verify agent classes were generated
        EXPECT_TRUE(content.find("ICalculatorServiceAgent") != string::npos ||
                   content.find("class ICalculatorService") != string::npos)
            << "Agent should contain calculator service class";
        
        // Verify handler methods or similar agent patterns
        EXPECT_TRUE(content.find("Handle") != string::npos || 
                   content.find("Add") != string::npos)
            << "Agent should contain method handlers";
        
        // Verify network includes
        EXPECT_TRUE(content.find("RakNet/BitStream.h") != string::npos ||
                   content.find("BitStream") != string::npos)
            << "Agent should include networking headers";
    }

    // Test manual proxy/agent generation using Tau generators directly
    bool TestDirectTauGeneration() {
        // Read the Tau file content
        ifstream file(tauFile);
        if (!file.is_open()) {
            KAI_LOG_ERROR("Cannot read Tau file for direct generation");
            return false;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string tauContent = buffer.str();
        
        try {
            // Test proxy generation
            string proxyOutput;
            tau::Generate::GenerateProxy proxy(tauContent.c_str(), proxyOutput);
            
            bool proxySuccess = !proxy.Failed;
            KAI_LOG_INFO("Direct proxy generation: " + string(proxySuccess ? "SUCCESS" : "FAILED"));
            if (proxy.Failed) {
                KAI_LOG_ERROR("Proxy generation error: " + proxy.Error);
            }
            
            // Test agent generation
            string agentOutput;
            tau::Generate::GenerateAgent agent(tauContent.c_str(), agentOutput);
            
            bool agentSuccess = !agent.Failed;
            KAI_LOG_INFO("Direct agent generation: " + string(agentSuccess ? "SUCCESS" : "FAILED"));
            if (agent.Failed) {
                KAI_LOG_ERROR("Agent generation error: " + agent.Error);
            }
            
            return proxySuccess && agentSuccess;
            
        } catch (const exception& e) {
            KAI_LOG_ERROR("Exception during direct generation: " + string(e.what()));
            return false;
        }
    }

    // Test Tau parsing directly
    bool TestTauParsing() {
        ifstream file(tauFile);
        if (!file.is_open()) {
            KAI_LOG_ERROR("Cannot read Tau file for parsing test");
            return false;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string tauContent = buffer.str();
        
        try {
            Registry registry;
            
            // Create and run lexer
            auto lexer = make_shared<tau::TauLexer>(tauContent.c_str(), registry);
            bool lexResult = lexer->Process();
            
            KAI_LOG_INFO("Tau lexing: " + string(lexResult ? "SUCCESS" : "FAILED"));
            if (lexResult) {
                KAI_LOG_INFO("Token count: " + to_string(lexer->GetTokens().size()));
            }
            
            if (!lexResult) {
                KAI_LOG_ERROR("Lexer failed: " + lexer->Print());
                return false;
            }
            
            // Create and run parser
            auto parser = make_shared<tau::TauParser>(registry);
            bool parseResult = parser->Process(lexer, Structure::Module);
            
            KAI_LOG_INFO("Tau parsing: " + string(parseResult ? "SUCCESS" : "FAILED"));
            if (!parseResult) {
                KAI_LOG_ERROR("Parser failed: " + parser->Error);
            }
            
            return parseResult;
            
        } catch (const exception& e) {
            KAI_LOG_ERROR("Exception during Tau parsing: " + string(e.what()));
            return false;
        }
    }
};

// Test the complete workflow from Tau IDL to generated network code
TEST_F(TauNetworkCommunicationTest, CompleteNetworkCodeGeneration) {
    KAI_LOG_INFO("=== Starting Complete Network Code Generation Test ===");
    
    // Step 1: Create comprehensive Tau interface
    CreateTestTauInterface();
    ASSERT_TRUE(fs::exists(tauFile)) << "Test Tau file was not created";
    
    // Step 2: Test Tau parsing
    bool parseSuccess = TestTauParsing();
    EXPECT_TRUE(parseSuccess) << "Tau parsing should succeed";
    
    // Step 3: Test direct Tau generation (using library directly)
    bool directGenSuccess = TestDirectTauGeneration();
    EXPECT_TRUE(directGenSuccess) << "Direct Tau generation should succeed";
    
    // Step 4: Generate proxy and agent using NetworkGenerate tool
    bool genSuccess = GenerateProxyAndAgent();
    EXPECT_TRUE(genSuccess) << "NetworkGenerate should succeed";
    
    // Step 5: Verify generated code quality
    if (genSuccess) {
        VerifyProxyGeneration();
        VerifyAgentGeneration();
    }
    
    KAI_LOG_INFO("=== Complete Network Code Generation Test COMPLETED ===");
}

// Test NetworkGenerate tool functionality
TEST_F(TauNetworkCommunicationTest, NetworkGenerateTool) {
    KAI_LOG_INFO("=== Testing NetworkGenerate Tool ===");
    
    CreateTestTauInterface();
    
    // Test that NetworkGenerate executable exists
    ASSERT_TRUE(fs::exists(networkGenerateExe)) 
        << "NetworkGenerate executable not found: " << networkGenerateExe;
    
    // Test NetworkGenerate with our interface
    bool success = GenerateProxyAndAgent();
    EXPECT_TRUE(success) << "NetworkGenerate should process Tau file successfully";
    
    KAI_LOG_INFO("=== NetworkGenerate Tool Test COMPLETED ===");
}

// Test Tau IDL parsing and validation
TEST_F(TauNetworkCommunicationTest, TauIDLParsing) {
    KAI_LOG_INFO("=== Testing Tau IDL Parsing ===");
    
    CreateTestTauInterface();
    
    // Test parsing our comprehensive interface
    bool parseSuccess = TestTauParsing();
    EXPECT_TRUE(parseSuccess) << "Complex Tau interface should parse successfully";
    
    KAI_LOG_INFO("=== Tau IDL Parsing Test COMPLETED ===");
}

// Test direct Tau code generation (without NetworkGenerate tool)
TEST_F(TauNetworkCommunicationTest, DirectTauGeneration) {
    KAI_LOG_INFO("=== Testing Direct Tau Generation ===");
    
    CreateTestTauInterface();
    
    // Test direct generation using Tau library
    bool genSuccess = TestDirectTauGeneration();
    EXPECT_TRUE(genSuccess) << "Direct Tau generation should succeed";
    
    KAI_LOG_INFO("=== Direct Tau Generation Test COMPLETED ===");
}

// Test with simple interface to ensure basic functionality works
TEST_F(TauNetworkCommunicationTest, SimpleInterfaceGeneration) {
    KAI_LOG_INFO("=== Testing Simple Interface Generation ===");
    
    // Create a minimal interface
    ofstream file(tauFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create simple Tau file";
    
    file << R"(// Simple test interface
namespace Simple {
    struct Point {
        float x;
        float y;
    }
    
    interface ISimple {
        float Distance(Point a, Point b);
        void Clear();
        
        event CalculationDone(float result);
    }
}
)";
    file.close();
    
    // Test parsing
    bool parseSuccess = TestTauParsing();
    EXPECT_TRUE(parseSuccess) << "Simple interface should parse";
    
    // Test generation
    bool genSuccess = TestDirectTauGeneration();
    EXPECT_TRUE(genSuccess) << "Simple interface should generate code";
    
    KAI_LOG_INFO("=== Simple Interface Generation Test COMPLETED ===");
}

// Test error handling with malformed Tau syntax
TEST_F(TauNetworkCommunicationTest, ErrorHandling) {
    KAI_LOG_INFO("=== Testing Error Handling ===");
    
    // Create invalid Tau file
    ofstream file(tauFile);
    ASSERT_TRUE(file.is_open()) << "Failed to create invalid Tau file";
    
    file << R"(// Invalid Tau syntax
namespace Invalid {
    struct Broken {
        int x  // Missing semicolon
        float y;
    }
    
    interface IBroken {
        float Add(float a float b);  // Missing comma
        void DoSomething(;           // Missing parameter
    }
}
)";
    file.close();
    
    // Test that parsing properly fails
    bool parseSuccess = TestTauParsing();
    EXPECT_FALSE(parseSuccess) << "Invalid Tau syntax should fail parsing";
    
    // Test that generation fails gracefully
    bool genSuccess = TestDirectTauGeneration();
    EXPECT_FALSE(genSuccess) << "Invalid syntax should fail generation";
    
    KAI_LOG_INFO("=== Error Handling Test COMPLETED ===");
}