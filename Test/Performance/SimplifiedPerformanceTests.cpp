#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <string>
#include <memory>
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"

using namespace kai;
using namespace std;
using namespace std::chrono;

/**
 * Simplified performance benchmarking tests focusing on Tau IDL
 */
class SimplifiedPerformanceTests : public ::testing::Test {
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
        for (int i = 0; i < 5; ++i) {
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

// Benchmark Tau parsing performance
TEST_F(SimplifiedPerformanceTests, TauParsingPerformance) {
    // Simple interface
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
        ParseTauCode(simpleInterface);
    }, 1000);
    parseResult.print("Tau Simple Interface Parsing");
    
    // Complex interface
    string complexInterface = R"(
        namespace Complex {
            struct DataStruct {
                int id;
                string name;
                float value;
                bool active;
                int64 timestamp;
            }
            
            struct NestedStruct {
                DataStruct data;
                DataStruct[] array;
                string[] tags;
            }
            
            enum Priority {
                Low,
                Medium,
                High,
                Critical
            }
            
            interface IComplexService {
                DataStruct ProcessData(DataStruct input, Priority priority);
                NestedStruct GetNestedData(int id, bool includeHistory);
                void UpdateMultiple(DataStruct[] updates);
                Priority[] GetPriorities();
                
                event DataProcessed(DataStruct result, Priority priority);
                event BatchCompleted(int count, string status);
                event ErrorOccurred(string error, int code);
            }
        }
    )";
    
    auto complexParseResult = BenchmarkFunction([&]() {
        ParseTauCode(complexInterface);
    }, 500);
    complexParseResult.print("Tau Complex Interface Parsing");
}

// Benchmark Tau code generation performance
TEST_F(SimplifiedPerformanceTests, TauCodeGenerationPerformance) {
    string testInterface = R"(
        namespace CodeGen {
            struct RequestData {
                int requestId;
                string operation;
                float[] values;
                bool urgent;
            }
            
            struct ResponseData {
                int requestId;
                bool success;
                float result;
                string message;
            }
            
            interface ITestService {
                ResponseData ProcessRequest(RequestData request);
                void ProcessBatch(RequestData[] requests);
                ResponseData[] GetHistory(int limit);
                
                event RequestProcessed(ResponseData response);
                event BatchCompleted(int count);
            }
        }
    )";
    
    auto proxyResult = BenchmarkFunction([&]() {
        string output;
        GenerateTauProxy(testInterface, output);
    }, 500);
    proxyResult.print("Tau Proxy Generation");
    
    auto agentResult = BenchmarkFunction([&]() {
        string output;
        GenerateTauAgent(testInterface, output);
    }, 500);
    agentResult.print("Tau Agent Generation");
    
    // Combined parse + generate
    auto combinedResult = BenchmarkFunction([&]() {
        ParseTauCode(testInterface);
        string proxyOutput, agentOutput;
        GenerateTauProxy(testInterface, proxyOutput);
        GenerateTauAgent(testInterface, agentOutput);
    }, 200);
    combinedResult.print("Tau Combined Parse + Generation");
}

// Benchmark registry operations
TEST_F(SimplifiedPerformanceTests, RegistryPerformance) {
    auto registryResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        
        // Test registry creation and destruction
        for (int i = 0; i < 10; ++i) {
            Registry tempRegistry;
            tempRegistry.Clear();
        }
    }, 1000);
    registryResult.print("Registry Creation/Destruction");
    
    auto clearResult = BenchmarkFunction([&]() {
        Registry localRegistry;
        localRegistry.Clear();
    }, 2000);
    clearResult.print("Registry Clear Operations");
}

// Benchmark memory allocation patterns
TEST_F(SimplifiedPerformanceTests, MemoryAllocationPerformance) {
    // Standard vector allocation
    auto vectorResult = BenchmarkFunction([&]() {
        vector<int> data;
        data.reserve(1000);
        for (int i = 0; i < 1000; ++i) {
            data.push_back(i);
        }
    }, 500);
    vectorResult.print("Standard Vector Allocation");
    
    // String allocation
    auto stringResult = BenchmarkFunction([&]() {
        vector<string> strings;
        strings.reserve(100);
        for (int i = 0; i < 100; ++i) {
            strings.push_back("test_string_" + to_string(i));
        }
    }, 1000);
    stringResult.print("String Allocation");
}

// Benchmark large interface scalability
TEST_F(SimplifiedPerformanceTests, ScalabilityBenchmark) {
    cout << "\n=== Tau Scalability Benchmark ===\n";
    
    vector<int> methodCounts = {10, 25, 50, 100};
    
    cout << "Methods\tParse(ms)\tProxy(ms)\tAgent(ms)\tTotal(ms)\n";
    cout << "-------\t---------\t---------\t---------\t---------\n";
    
    for (int methodCount : methodCounts) {
        // Generate interface with specified number of methods
        stringstream ss;
        ss << "namespace Scale" << methodCount << " {\n";
        ss << "    struct DataType {\n";
        ss << "        int id;\n";
        ss << "        string name;\n";
        ss << "        float value;\n";
        ss << "    }\n\n";
        ss << "    interface IScaleTest {\n";
        
        for (int i = 0; i < methodCount; ++i) {
            ss << "        DataType Method" << i << "(DataType param" << i << ");\n";
        }
        
        for (int i = 0; i < methodCount / 4; ++i) {
            ss << "        event Event" << i << "(DataType data" << i << ");\n";
        }
        
        ss << "    }\n}\n";
        
        string interfaceCode = ss.str();
        
        // Benchmark parsing
        auto parseTime = BenchmarkFunction([&]() {
            ParseTauCode(interfaceCode);
        }, 100);
        
        // Benchmark proxy generation  
        auto proxyTime = BenchmarkFunction([&]() {
            string output;
            GenerateTauProxy(interfaceCode, output);
        }, 50);
        
        // Benchmark agent generation
        auto agentTime = BenchmarkFunction([&]() {
            string output;
            GenerateTauAgent(interfaceCode, output);
        }, 50);
        
        double totalTime = parseTime.averageTime + proxyTime.averageTime + agentTime.averageTime;
        
        cout << methodCount << "\t" 
             << fixed << setprecision(2) 
             << parseTime.averageTime << "\t\t"
             << proxyTime.averageTime << "\t\t"
             << agentTime.averageTime << "\t\t"
             << totalTime << "\n";
    }
}

// Overall system throughput benchmark
TEST_F(SimplifiedPerformanceTests, SystemThroughputBenchmark) {
    cout << "\n=== KAI Tau System Throughput Benchmark ===\n";
    
    string benchmarkInterface = R"(
        namespace Throughput {
            struct Message {
                int id;
                string content;
                float priority;
                bool processed;
            }
            
            interface IMessageProcessor {
                Message ProcessMessage(Message input);
                void ProcessBatch(Message[] messages);
                Message[] GetPending();
                
                event MessageProcessed(Message result);
                event BatchCompleted(int count);
            }
        }
    )";
    
    auto throughputResult = BenchmarkFunction([&]() {
        // Parse interface
        ParseTauCode(benchmarkInterface);
        
        // Generate proxy
        string proxyOutput;
        GenerateTauProxy(benchmarkInterface, proxyOutput);
        
        // Generate agent
        string agentOutput;
        GenerateTauAgent(benchmarkInterface, agentOutput);
        
        // Simulate some processing
        Registry tempRegistry;
        tempRegistry.Clear();
    }, 100);
    
    throughputResult.print("System Throughput");
    
    // Calculate operations per second
    int operationsPerIteration = 4; // parse + proxy + agent + registry ops
    double operationsPerSecond = (operationsPerIteration * 1000.0) / throughputResult.averageTime;
    
    cout << "Operations per iteration: " << operationsPerIteration << "\n";
    cout << "System throughput: " << operationsPerSecond << " ops/sec\n";
    cout << "Average latency per operation: " << throughputResult.averageTime / operationsPerIteration << " ms\n";
}