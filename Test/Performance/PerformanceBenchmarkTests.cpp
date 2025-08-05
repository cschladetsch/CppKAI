#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <random>
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/Object.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"

using namespace kai;
using namespace std;
using namespace std::chrono;

/**
 * Performance benchmarking tests for KAI system components
 * Measures execution time, memory usage, and throughput
 */
class PerformanceBenchmarkTests : public ::testing::Test {
protected:
    Registry registry;
    
    void SetUp() override {
        registry.Clear();
    }
    
    struct BenchmarkResult {
        double averageTime;
        double minTime;
        double maxTime;
        double standardDeviation;
        size_t iterations;
        
        void print(const string& testName) const {
            cout << "\n" << testName << " Performance Results:\n";
            cout << "Iterations: " << iterations << "\n";
            cout << "Average: " << fixed << setprecision(3) << averageTime << " ms\n";
            cout << "Min: " << minTime << " ms\n";
            cout << "Max: " << maxTime << " ms\n";
            cout << "Std Dev: " << standardDeviation << " ms\n";
            cout << "Ops/sec: " << (1000.0 / averageTime) << "\n";
        }
    };
    
    template<typename Func>
    BenchmarkResult BenchmarkFunction(Func func, size_t iterations = 1000) {
        vector<double> times;
        times.reserve(iterations);
        
        // Warm-up runs
        for (int i = 0; i < 10; ++i) {
            func();
        }
        
        // Actual benchmark runs
        for (size_t i = 0; i < iterations; ++i) {
            auto start = high_resolution_clock::now();
            func();
            auto end = high_resolution_clock::now();
            
            double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
            times.push_back(timeMs);
        }
        
        BenchmarkResult result;
        result.iterations = iterations;
        result.averageTime = accumulate(times.begin(), times.end(), 0.0) / times.size();
        result.minTime = *min_element(times.begin(), times.end());
        result.maxTime = *max_element(times.begin(), times.end());
        
        // Calculate standard deviation
        double variance = 0.0;
        for (double time : times) {
            variance += (time - result.averageTime) * (time - result.averageTime);
        }
        result.standardDeviation = sqrt(variance / times.size());
        
        return result;
    }
    
    bool ParseTauCode(const string& code) {
        try {
            Registry localRegistry;
            auto lexer = make_shared<tau::TauLexer>(code.c_str(), localRegistry);
            if (!lexer->Process()) {
                return false;
            }
            
            auto parser = make_shared<tau::TauParser>(localRegistry);
            return parser->Process(lexer, Structure::Module);
        } catch (const exception&) {
            return false;
        }
    }
    
    bool GenerateTauProxy(const string& code, string& output) {
        try {
            tau::Generate::GenerateProxy generator(code.c_str(), output);
            return !generator.Failed;
        } catch (const exception&) {
            return false;
        }
    }
    
    bool GenerateTauAgent(const string& code, string& output) {
        try {
            tau::Generate::GenerateAgent generator(code.c_str(), output);
            return !generator.Failed;
        } catch (const exception&) {
            return false;
        }
    }
};

// Benchmark core object system performance
TEST_F(PerformanceBenchmarkTests, CoreObjectSystemPerformance) {
    // Object creation
    auto createResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 100; ++i) {
            auto obj = localRegistry.New<int>(i);
            (void)obj; // Suppress unused warning
        }
    }, 1000);
    createResult.print("Object Creation");
    
    // Object comparison
    auto compareResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        auto obj1 = localRegistry.New<int>(42);
        auto obj2 = localRegistry.New<int>(42);
        auto obj3 = localRegistry.New<int>(43);
        
        bool result1 = obj1 == obj2;
        bool result2 = obj1 == obj3;
        (void)result1; (void)result2; // Suppress warnings
    }, 2000);
    compareResult.print("Object Comparison");
    
    // String operations
    auto stringResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 50; ++i) {
            string text = "test_string_" + to_string(i);
            auto obj = localRegistry.New<string>(text);
            (void)obj; // Suppress unused warning
        }
    }, 1000);
    stringResult.print("String Object Operations");
}

// Benchmark Tau parsing and code generation
TEST_F(PerformanceBenchmarkTests, TauCodeGenerationPerformance) {
    // Simple interface parsing
    string simpleInterface = R"(
        namespace Simple {
            interface ISimple {
                void Method1();
                int Method2(string param);
                event SimpleEvent(int data);
            }
        }
    )";
    
    auto parseResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        auto lexer = make_shared<tau::TauLexer>(simpleInterface.c_str(), localRegistry);
        lexer->Process();
        auto parser = make_shared<tau::TauParser>(localRegistry);
        parser->Process(lexer, Structure::Module);
    }, 2000);
    parseResult.print("Tau Simple Interface Parsing");
    
    auto proxyResult = BenchmarkFunction([&]() {
        string output;
        tau::Generate::GenerateProxy generator(simpleInterface.c_str(), output);
    }, 1000);
    proxyResult.print("Tau Simple Proxy Generation");
    
    auto agentResult = BenchmarkFunction([&]() {
        string output;
        tau::Generate::GenerateAgent generator(simpleInterface.c_str(), output);
    }, 1000);
    agentResult.print("Tau Simple Agent Generation");
    
    // Complex interface with many methods
    stringstream complexInterface;
    complexInterface << "namespace Complex {\n";
    complexInterface << "    struct DataStruct {\n";
    complexInterface << "        int id; string name; float value; bool active;\n";
    complexInterface << "    }\n";
    complexInterface << "    interface IComplex {\n";
    for (int i = 0; i < 50; ++i) {
        complexInterface << "        DataStruct Method" << i << "(DataStruct param" << i << ");\n";
    }
    for (int i = 0; i < 25; ++i) {
        complexInterface << "        event Event" << i << "(DataStruct data" << i << ");\n";
    }
    complexInterface << "    }\n}\n";
    
    string complexInterfaceStr = complexInterface.str();
    
    auto complexParseResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        auto lexer = make_shared<tau::TauLexer>(complexInterfaceStr.c_str(), localRegistry);
        lexer->Process();
        auto parser = make_shared<tau::TauParser>(localRegistry);
        parser->Process(lexer, Structure::Module);
    }, 200);
    complexParseResult.print("Tau Complex Interface Parsing");
    
    auto complexProxyResult = BenchmarkFunction([&]() {
        string output;
        tau::Generate::GenerateProxy generator(complexInterfaceStr.c_str(), output);
    }, 100);
    complexProxyResult.print("Tau Complex Proxy Generation");
}

// Benchmark registry operations
TEST_F(PerformanceBenchmarkTests, RegistryPerformance) {
    // Object creation
    auto createResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 100; ++i) {
            auto obj = localRegistry.New<int>(i);
            (void)obj; // Suppress unused warning
        }
    }, 500);
    createResult.print("Registry Object Creation");
    
    // Registry clearing
    auto clearResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 50; ++i) {
            auto obj = localRegistry.New<int>(i);
            (void)obj;
        }
        localRegistry.Clear();
    }, 1000);
    clearResult.print("Registry Clear Operations");
}

// Benchmark memory allocation patterns
TEST_F(PerformanceBenchmarkTests, AllocationPatterns) {
    // Sequential allocation
    auto sequentialResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        vector<Object> objects;
        objects.reserve(100);
        
        for (int i = 0; i < 100; ++i) {
            objects.push_back(localRegistry.New<int>(i));
        }
    }, 1000);
    sequentialResult.print("Sequential Allocation");
    
    // Mixed type allocation
    auto mixedResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 50; ++i) {
            auto intObj = localRegistry.New<int>(i);
            auto floatObj = localRegistry.New<float>(i * 1.5f);
            auto stringObj = localRegistry.New<string>("test" + to_string(i));
            (void)intObj; (void)floatObj; (void)stringObj;
        }
    }, 500);
    mixedResult.print("Mixed Type Allocation");
}

// Benchmark memory allocation and garbage collection
TEST_F(PerformanceBenchmarkTests, MemoryManagementPerformance) {
    // Object allocation
    auto allocResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        vector<Object> objects;
        objects.reserve(1000);
        
        for (int i = 0; i < 1000; ++i) {
            objects.push_back(localRegistry.New<int>(i));
        }
    }, 100);
    allocResult.print("Memory Allocation");
    
    // Large object allocation
    auto largeAllocResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        vector<Object> objects;
        objects.reserve(10);
        
        for (int i = 0; i < 10; ++i) {
            vector<int> largeVector(10000, i);
            objects.push_back(localRegistry.New<vector<int>>(largeVector));
        }
    }, 100);
    largeAllocResult.print("Large Object Allocation");
    
    // Garbage collection simulation
    auto gcResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        
        // Create many objects
        for (int i = 0; i < 1000; ++i) {
            auto obj = localRegistry.New<int>(i);
            // Let them go out of scope to trigger collection
        }
        
        // Force cleanup
        localRegistry.Clear();
    }, 50);
    gcResult.print("Garbage Collection Simulation");
}

// Benchmark string processing
TEST_F(PerformanceBenchmarkTests, StringProcessingPerformance) {
    // String creation and manipulation
    auto stringResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        for (int i = 0; i < 100; ++i) {
            string baseString = "performance_test_string_";
            string fullString = baseString + to_string(i) + "_end";
            auto obj = localRegistry.New<string>(fullString);
            (void)obj;
        }
    }, 1000);
    stringResult.print("String Processing");
}

// Overall system throughput benchmark
TEST_F(PerformanceBenchmarkTests, SystemThroughputBenchmark) {
    cout << "\n=== KAI System Throughput Benchmark ===\n";
    
    string tauInterface = R"(
        namespace Benchmark {
            interface IBenchmark {
                int Process(int value);
                event Completed(int result);
            }
        }
    )";
    
    auto mixedWorkloadResult = BenchmarkFunction([&]() {
        // Core object operations
        Registry localRegistry;
        for (int i = 0; i < 10; ++i) {
            auto obj = localRegistry.New<int>(i);
            (void)obj;
        }
        
        // Parse Tau interface
        ParseTauCode(tauInterface);
        
        // Generate proxy
        string proxyOutput;
        GenerateTauProxy(tauInterface, proxyOutput);
        
        // Generate agent
        string agentOutput;
        GenerateTauAgent(tauInterface, agentOutput);
    }, 100);
    
    mixedWorkloadResult.print("Mixed Workload Throughput");
    
    int totalOperations = 13; // 10 object creations + parse + proxy + agent
    double operationsPerSecond = (totalOperations * 1000.0) / mixedWorkloadResult.averageTime;
    
    cout << "Operations per iteration: " << totalOperations << "\n";
    cout << "System throughput: " << operationsPerSecond << " ops/sec\n";
    cout << "Average latency per operation: " << mixedWorkloadResult.averageTime / totalOperations << " ms\n";
}