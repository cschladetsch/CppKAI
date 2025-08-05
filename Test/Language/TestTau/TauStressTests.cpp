#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>
#include <future>
#include <atomic>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"

using namespace kai;
using namespace std;
using namespace std::chrono;

/**
 * Stress tests for Tau IDL proxy/agent generation
 * Tests performance, memory usage, and scalability limits
 */
class TauStressTests : public ::testing::Test {
protected:
    Registry registry;
    
    void SetUp() override {
        registry.Clear();
    }
    
    // Generate a large interface with specified number of methods and events
    string GenerateLargeInterface(int numMethods, int numEvents, const string& namePrefix = "Stress") {
        stringstream ss;
        ss << "namespace " << namePrefix << " {\n";
        
        // Add some structs for complexity
        ss << "    struct DataStruct {\n";
        ss << "        int id;\n";
        ss << "        string name;\n";
        ss << "        float value;\n";
        ss << "        bool active;\n";
        ss << "        int64 timestamp;\n";
        ss << "    }\n\n";
        
        ss << "    struct ComplexStruct {\n";
        ss << "        DataStruct[] data;\n";
        ss << "        string[] tags;\n";
        ss << "        int[] numbers;\n";
        ss << "        float[] values;\n";
        ss << "    }\n\n";
        
        ss << "    interface I" << namePrefix << "Interface {\n";
        
        // Generate methods with varying complexity
        for (int i = 0; i < numMethods; ++i) {
            if (i % 4 == 0) {
                // Simple method
                ss << "        void Method" << i << "();\n";
            } else if (i % 4 == 1) {
                // Method with basic parameters
                ss << "        int Method" << i << "(int param1, string param2);\n";
            } else if (i % 4 == 2) {
                // Method with complex parameters
                ss << "        DataStruct Method" << i << "(DataStruct input, int[] filters);\n";
            } else {
                // Method with very complex parameters
                ss << "        ComplexStruct[] Method" << i << "(ComplexStruct[] inputs, DataStruct filter, bool sorted);\n";
            }
        }
        
        // Generate events with varying complexity
        for (int i = 0; i < numEvents; ++i) {
            if (i % 3 == 0) {
                ss << "        event Event" << i << "();\n";
            } else if (i % 3 == 1) {
                ss << "        event Event" << i << "(string message, int code);\n";
            } else {
                ss << "        event Event" << i << "(DataStruct data, ComplexStruct context);\n";
            }
        }
        
        ss << "    }\n";
        ss << "}\n";
        
        return ss.str();
    }
    
    // Generate multiple interfaces in one namespace
    string GenerateMultipleInterfaces(int numInterfaces, int methodsPerInterface) {
        stringstream ss;
        ss << "namespace MultiStress {\n";
        
        // Common structs
        ss << "    struct SharedStruct {\n";
        ss << "        int id;\n";
        ss << "        string data;\n";
        ss << "    }\n\n";
        
        for (int i = 0; i < numInterfaces; ++i) {
            ss << "    interface IInterface" << i << " {\n";
            
            for (int j = 0; j < methodsPerInterface; ++j) {
                ss << "        SharedStruct Method" << j << "(SharedStruct input" << j << ");\n";
            }
            
            ss << "        event InterfaceEvent" << i << "(int eventData);\n";
            ss << "    }\n\n";
        }
        
        ss << "}\n";
        return ss.str();
    }
    
    bool GenerateAndMeasure(const string& tauContent, string& proxyOutput, string& agentOutput, 
                           double& parseTime, double& proxyTime, double& agentTime) {
        auto start = high_resolution_clock::now();
        
        // Parse
        try {
            auto lexer = make_shared<tau::TauLexer>(tauContent.c_str(), registry);
            if (!lexer->Process()) {
                return false;
            }
            
            auto parser = make_shared<tau::TauParser>(registry);
            if (!parser->Process(lexer, Structure::Module)) {
                return false;
            }
        } catch (const exception&) {
            return false;
        }
        
        auto parseEnd = high_resolution_clock::now();
        parseTime = duration_cast<microseconds>(parseEnd - start).count() / 1000.0;
        
        // Generate proxy
        try {
            tau::Generate::GenerateProxy proxyGen(tauContent.c_str(), proxyOutput);
            if (proxyGen.Failed) {
                return false;
            }
        } catch (const exception&) {
            return false;
        }
        
        auto proxyEnd = high_resolution_clock::now();
        proxyTime = duration_cast<microseconds>(proxyEnd - parseEnd).count() / 1000.0;
        
        // Generate agent
        try {
            tau::Generate::GenerateAgent agentGen(tauContent.c_str(), agentOutput);
            if (agentGen.Failed) {
                return false;
            }
        } catch (const exception&) {
            return false;
        }
        
        auto agentEnd = high_resolution_clock::now();
        agentTime = duration_cast<microseconds>(agentEnd - proxyEnd).count() / 1000.0;
        
        return true;
    }
};

// Test generation performance with increasing interface sizes
TEST_F(TauStressTests, ScalabilityTest) {
    vector<int> methodCounts = {10, 50, 100, 200, 500};
    vector<int> eventCounts = {5, 25, 50, 100, 250};
    
    cout << "\nScalability Test Results:\n";
    cout << "Methods\tEvents\tParse(ms)\tProxy(ms)\tAgent(ms)\tProxy Size\tAgent Size\n";
    cout << "-------\t------\t---------\t---------\t---------\t----------\t----------\n";
    
    for (size_t i = 0; i < methodCounts.size(); ++i) {
        int methods = methodCounts[i];
        int events = eventCounts[i];
        
        string tauContent = GenerateLargeInterface(methods, events, "Scale" + to_string(i));
        string proxyOutput, agentOutput;
        double parseTime, proxyTime, agentTime;
        
        bool success = GenerateAndMeasure(tauContent, proxyOutput, agentOutput, 
                                         parseTime, proxyTime, agentTime);
        
        EXPECT_TRUE(success) << "Failed to generate for " << methods << " methods, " << events << " events";
        
        if (success) {
            cout << methods << "\t" << events << "\t" 
                 << fixed << setprecision(2) << parseTime << "\t\t"
                 << proxyTime << "\t\t" << agentTime << "\t\t"
                 << proxyOutput.size() << "\t\t" << agentOutput.size() << "\n";
            
            // Verify output contains expected content
            EXPECT_TRUE(proxyOutput.find("Scale" + to_string(i)) != string::npos);
            EXPECT_TRUE(agentOutput.find("Scale" + to_string(i)) != string::npos);
            
            // Performance expectations (generous to account for system variance)
            EXPECT_LT(parseTime, 1000.0) << "Parse time too slow for " << methods << " methods";
            EXPECT_LT(proxyTime, 2000.0) << "Proxy generation too slow for " << methods << " methods";
            EXPECT_LT(agentTime, 2000.0) << "Agent generation too slow for " << methods << " methods";
        }
    }
}

// Test memory usage with large interfaces
TEST_F(TauStressTests, MemoryUsageTest) {
    const int LARGE_METHOD_COUNT = 1000;
    const int LARGE_EVENT_COUNT = 500;
    
    string tauContent = GenerateLargeInterface(LARGE_METHOD_COUNT, LARGE_EVENT_COUNT, "Memory");
    string proxyOutput, agentOutput;
    double parseTime, proxyTime, agentTime;
    
    auto start = high_resolution_clock::now();
    bool success = GenerateAndMeasure(tauContent, proxyOutput, agentOutput, 
                                     parseTime, proxyTime, agentTime);
    auto end = high_resolution_clock::now();
    
    EXPECT_TRUE(success) << "Failed to generate large interface";
    
    if (success) {
        double totalTime = duration_cast<milliseconds>(end - start).count();
        
        cout << "\nMemory Usage Test Results:\n";
        cout << "Input size: " << tauContent.size() / 1024 << " KB\n";
        cout << "Proxy size: " << proxyOutput.size() / 1024 << " KB\n";
        cout << "Agent size: " << agentOutput.size() / 1024 << " KB\n";
        cout << "Total time: " << totalTime << " ms\n";
        cout << "Methods: " << LARGE_METHOD_COUNT << ", Events: " << LARGE_EVENT_COUNT << "\n";
        
        // Memory usage should be reasonable
        EXPECT_LT(proxyOutput.size(), 10 * 1024 * 1024); // Less than 10MB
        EXPECT_LT(agentOutput.size(), 10 * 1024 * 1024);  // Less than 10MB
        
        // Should complete within reasonable time
        EXPECT_LT(totalTime, 30000); // Less than 30 seconds
        
        // Verify content quality
        EXPECT_TRUE(proxyOutput.find("namespace Memory") != string::npos);
        EXPECT_TRUE(agentOutput.find("namespace Memory") != string::npos);
        
        // Count generated methods (approximate)
        size_t proxyMethodCount = 0;
        size_t pos = 0;
        while ((pos = proxyOutput.find("Method", pos)) != string::npos) {
            proxyMethodCount++;
            pos++;
        }
        EXPECT_GE(proxyMethodCount, LARGE_METHOD_COUNT * 0.9);
    }
}

// Test concurrent generation
TEST_F(TauStressTests, ConcurrentGenerationTest) {
    const int NUM_THREADS = 4;
    const int METHODS_PER_INTERFACE = 100;
    const int EVENTS_PER_INTERFACE = 50;
    
    atomic<int> successfulGenerations{0};
    atomic<int> totalAttempts{0};
    vector<future<bool>> futures;
    
    // Launch concurrent generation tasks
    for (int i = 0; i < NUM_THREADS; ++i) {
        futures.push_back(async(launch::async, [i, &successfulGenerations, &totalAttempts, this]() {
            Registry localRegistry;
            string tauContent = GenerateLargeInterface(
                100, 50, "Concurrent" + to_string(i));
            
            totalAttempts++;
            
            try {
                string proxyOutput, agentOutput;
                
                tau::Generate::GenerateProxy proxyGen(tauContent.c_str(), proxyOutput);
                if (proxyGen.Failed) return false;
                
                tau::Generate::GenerateAgent agentGen(tauContent.c_str(), agentOutput);
                if (agentGen.Failed) return false;
                
                // Verify outputs are reasonable
                if (proxyOutput.size() > 1000 && agentOutput.size() > 1000 &&
                    proxyOutput.find("Concurrent" + to_string(i)) != string::npos &&
                    agentOutput.find("Concurrent" + to_string(i)) != string::npos) {
                    successfulGenerations++;
                    return true;
                }
            } catch (const exception&) {
                return false;
            }
            
            return false;
        }));
    }
    
    // Wait for all tasks to complete
    vector<bool> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }
    
    EXPECT_EQ(totalAttempts.load(), NUM_THREADS);
    EXPECT_GE(successfulGenerations.load(), NUM_THREADS * 0.8); // Allow some failures due to concurrency
    
    cout << "\nConcurrent Generation Test Results:\n";
    cout << "Threads: " << NUM_THREADS << "\n";
    cout << "Successful: " << successfulGenerations.load() << "/" << totalAttempts.load() << "\n";
}

// Test generation with multiple interfaces
TEST_F(TauStressTests, MultipleInterfacesTest) {
    vector<int> interfaceCounts = {5, 10, 25, 50};
    const int METHODS_PER_INTERFACE = 20;
    
    cout << "\nMultiple Interfaces Test Results:\n";
    cout << "Interfaces\tParse(ms)\tProxy(ms)\tAgent(ms)\tTotal Size(KB)\n";
    cout << "----------\t---------\t---------\t---------\t--------------\n";
    
    for (int interfaceCount : interfaceCounts) {
        string tauContent = GenerateMultipleInterfaces(interfaceCount, METHODS_PER_INTERFACE);
        string proxyOutput, agentOutput;
        double parseTime, proxyTime, agentTime;
        
        bool success = GenerateAndMeasure(tauContent, proxyOutput, agentOutput, 
                                         parseTime, proxyTime, agentTime);
        
        EXPECT_TRUE(success) << "Failed to generate " << interfaceCount << " interfaces";
        
        if (success) {
            size_t totalSize = (proxyOutput.size() + agentOutput.size()) / 1024;
            
            cout << interfaceCount << "\t\t" 
                 << fixed << setprecision(2) << parseTime << "\t\t"
                 << proxyTime << "\t\t" << agentTime << "\t\t"
                 << totalSize << "\n";
            
            // Verify all interfaces are present
            for (int i = 0; i < interfaceCount; ++i) {
                string interfaceName = "IInterface" + to_string(i);
                EXPECT_TRUE(proxyOutput.find(interfaceName) != string::npos);
                EXPECT_TRUE(agentOutput.find(interfaceName) != string::npos);
            }
            
            // Performance should scale reasonably
            double expectedTime = interfaceCount * 10.0; // 10ms per interface baseline
            EXPECT_LT(parseTime + proxyTime + agentTime, expectedTime * 3); // Allow 3x overhead
        }
    }
}

// Test generation with complex inheritance hierarchies
TEST_F(TauStressTests, ComplexInheritanceTest) {
    stringstream ss;
    ss << "namespace InheritanceStress {\n";
    
    // Base interfaces
    ss << "    interface IBase1 {\n";
    ss << "        void BaseMethod1();\n";
    ss << "        event BaseEvent1();\n";
    ss << "    }\n\n";
    
    ss << "    interface IBase2 {\n";
    ss << "        void BaseMethod2();\n";
    ss << "        event BaseEvent2();\n";
    ss << "    }\n\n";
    
    // Multiple inheritance levels
    for (int level = 1; level <= 5; ++level) {
        ss << "    interface ILevel" << level;
        if (level == 1) {
            ss << " : IBase1, IBase2";
        } else {
            ss << " : ILevel" << (level - 1);
        }
        ss << " {\n";
        
        for (int method = 0; method < 10; ++method) {
            ss << "        void Level" << level << "Method" << method << "();\n";
        }
        
        ss << "        event Level" << level << "Event();\n";
        ss << "    }\n\n";
    }
    
    ss << "}\n";
    
    string tauContent = ss.str();
    string proxyOutput, agentOutput;
    double parseTime, proxyTime, agentTime;
    
    bool success = GenerateAndMeasure(tauContent, proxyOutput, agentOutput, 
                                     parseTime, proxyTime, agentTime);
    
    EXPECT_TRUE(success) << "Failed to generate complex inheritance hierarchy";
    
    if (success) {
        cout << "\nComplex Inheritance Test Results:\n";
        cout << "Parse time: " << parseTime << " ms\n";
        cout << "Proxy time: " << proxyTime << " ms\n";
        cout << "Agent time: " << agentTime << " ms\n";
        cout << "Proxy size: " << proxyOutput.size() / 1024 << " KB\n";
        cout << "Agent size: " << agentOutput.size() / 1024 << " KB\n";
        
        // Verify inheritance is handled correctly
        for (int level = 1; level <= 5; ++level) {
            string interfaceName = "ILevel" + to_string(level);
            EXPECT_TRUE(proxyOutput.find(interfaceName) != string::npos);
            EXPECT_TRUE(agentOutput.find(interfaceName) != string::npos);
        }
        
        // Should complete within reasonable time even with complex inheritance
        EXPECT_LT(parseTime + proxyTime + agentTime, 5000.0); // Less than 5 seconds
    }
}

// Test with malformed content stress (error handling)
TEST_F(TauStressTests, ErrorHandlingStressTest) {
    vector<string> malformedInputs = {
        // Missing semicolons in large interface
        "namespace Bad { interface IBad { " + string(1000, 'v') + "oid method() } }",
        
        // Deeply nested but broken structure
        "namespace A { namespace B { namespace C { interface I { void m(int int int); } } }",
        
        // Very long identifier names
        "namespace Test { interface I" + string(1000, 'X') + " { void method(); } }",
        
        // Circular references
        "namespace Circular { struct A { B b; } struct B { A a; } }",
        
        // Invalid characters in various places
        "namespace Test { interface ITest { void method@#$%(); } }"
    };
    
    int handledErrorsCount = 0;
    
    for (size_t i = 0; i < malformedInputs.size(); ++i) {
        string proxyOutput, agentOutput;
        double parseTime, proxyTime, agentTime;
        
        // Should fail gracefully without crashing
        bool success = GenerateAndMeasure(malformedInputs[i], proxyOutput, agentOutput, 
                                         parseTime, proxyTime, agentTime);
        
        if (!success) {
            handledErrorsCount++;
        }
        
        // Even if it fails, it shouldn't crash or take too long
        EXPECT_LT(parseTime, 1000.0) << "Error handling took too long for input " << i;
    }
    
    // Most malformed inputs should be rejected
    EXPECT_GE(handledErrorsCount, malformedInputs.size() * 0.8);
    
    cout << "\nError Handling Stress Test Results:\n";
    cout << "Malformed inputs tested: " << malformedInputs.size() << "\n";
    cout << "Properly rejected: " << handledErrorsCount << "\n";
}