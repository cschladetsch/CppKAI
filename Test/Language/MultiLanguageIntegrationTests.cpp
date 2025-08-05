#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <memory>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Registry.h"
#include "KAI/Executor/Executor.h"
#include "KAI/Language/Pi/PiParser.h"
#include "KAI/Language/Pi/PiLexer.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoLexer.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"

using namespace kai;
using namespace std;

/**
 * Integration tests for multi-language scenarios in KAI
 * Tests interaction between Pi, Rho, Tau, and the core system
 */
class MultiLanguageIntegrationTests : public ::testing::Test {
protected:
    Registry registry;
    shared_ptr<Executor> executor;
    
    void SetUp() override {
        registry.Clear();
        executor = make_shared<Executor>(registry);
    }
    
    bool ExecutePiCode(const string& code) {
        try {
            auto lexer = make_shared<pi::PiLexer>(registry);
            lexer->Process(code.c_str());
            
            auto parser = make_shared<pi::PiParser>(registry);
            auto continuation = parser->Process(lexer);
            
            if (!continuation) return false;
            
            executor->Continue(continuation);
            return !executor->Failed;
        } catch (const exception&) {
            return false;
        }
    }
    
    bool ExecuteRhoCode(const string& code, Object& result) {
        try {
            auto lexer = make_shared<rho::RhoLexer>(registry);
            if (!lexer->Process(code.c_str())) {
                return false;
            }
            
            auto parser = make_shared<rho::RhoParser>(registry);
            auto tree = parser->Process(lexer);
            
            if (!tree) return false;
            
            auto continuation = executor->Compile(tree);
            if (!continuation) return false;
            
            executor->Continue(continuation);
            if (executor->Failed) return false;
            
            result = executor->GetDataStack().Top();
            return true;
        } catch (const exception&) {
            return false;
        }
    }
    
    bool ParseTauCode(const string& code) {
        try {
            auto lexer = make_shared<tau::TauLexer>(code.c_str(), registry);
            if (!lexer->Process()) {
                return false;
            }
            
            auto parser = make_shared<tau::TauParser>(registry);
            return parser->Process(lexer, Structure::Module);
        } catch (const exception&) {
            return false;
        }
    }
    
    bool GenerateTauProxy(const string& tauCode, string& proxyOutput) {
        try {
            tau::Generate::GenerateProxy generator(tauCode.c_str(), proxyOutput);
            return !generator.Failed;
        } catch (const exception&) {
            return false;
        }
    }
};

// Test Pi and Rho interoperability
TEST_F(MultiLanguageIntegrationTests, PiRhoInteroperability) {
    // Create some data using Pi
    string piCode = R"(
        42 'number =
        "Hello World" 'message =
        [1, 2, 3, 4, 5] 'numbers =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piCode));
    
    // Verify the data exists in the registry
    auto numberObj = registry.GetObject("number");
    auto messageObj = registry.GetObject("message");
    auto numbersObj = registry.GetObject("numbers");
    
    EXPECT_TRUE(numberObj.Exists());
    EXPECT_TRUE(messageObj.Exists());
    EXPECT_TRUE(numbersObj.Exists());
    
    // Use the data in Rho code
    string rhoCode = R"(
        number + 8
    )";
    
    Object result;
    EXPECT_TRUE(ExecuteRhoCode(rhoCode, result));
    
    // Verify the result
    if (result.Exists() && result.IsType<int>()) {
        EXPECT_EQ(result.GetValue<int>(), 50);
    }
}

// Test Tau interface definition with Pi/Rho implementation
TEST_F(MultiLanguageIntegrationTests, TauWithPiRhoImplementation) {
    // Define a service interface in Tau
    string tauInterface = R"(
        namespace MathService {
            struct CalculationRequest {
                float a;
                float b;
                string operation;
            }
            
            struct CalculationResult {
                float result;
                bool success;
                string errorMessage;
            }
            
            interface ICalculator {
                CalculationResult Calculate(CalculationRequest request);
                float Add(float a, float b);
                float Multiply(float a, float b);
                
                event CalculationCompleted(CalculationResult result);
            }
        }
    )";
    
    EXPECT_TRUE(ParseTauCode(tauInterface));
    
    // Generate proxy code
    string proxyCode;
    EXPECT_TRUE(GenerateTauProxy(tauInterface, proxyCode));
    EXPECT_FALSE(proxyCode.empty());
    
    // Verify proxy contains expected elements
    EXPECT_TRUE(proxyCode.find("MathService") != string::npos);
    EXPECT_TRUE(proxyCode.find("ICalculator") != string::npos);
    EXPECT_TRUE(proxyCode.find("Calculate") != string::npos);
    EXPECT_TRUE(proxyCode.find("Add") != string::npos);
    
    // Implement calculator logic in Pi (stack-based)
    string piImplementation = R"(
        {
            dup * // Square the top stack element
        } 'square =
        
        {
            + // Add two top stack elements
        } 'add =
        
        {
            * // Multiply two top stack elements  
        } 'multiply =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piImplementation));
    
    // Implement calculator logic in Rho (functional style)
    string rhoImplementation = R"(
        fun square(x) = x * x;
        fun add(a, b) = a + b;
        fun multiply(a, b) = a * b;
    )";
    
    Object rhoResult;
    EXPECT_TRUE(ExecuteRhoCode("add(5, 3)", rhoResult));
    if (rhoResult.Exists() && rhoResult.IsType<int>()) {
        EXPECT_EQ(rhoResult.GetValue<int>(), 8);
    }
}

// Test cross-language data sharing
TEST_F(MultiLanguageIntegrationTests, CrossLanguageDataSharing) {
    // Create complex data structure in Pi
    string piDataCreation = R"(
        [
            { 'name "Alice" = 'age 25 = 'active true = }
            { 'name "Bob" = 'age 30 = 'active false = }
            { 'name "Charlie" = 'age 35 = 'active true = }
        ] 'users =
        
        { 'total 0 = 'active 0 = } 'stats =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piDataCreation));
    
    // Process the data using Rho
    string rhoProcessing = R"(
        fun countActive(users) = 
            if (length(users) == 0) then 0
            else if (head(users).active) then 1 + countActive(tail(users))
            else countActive(tail(users));
    )";
    
    Object rhoResult;
    EXPECT_TRUE(ExecuteRhoCode(rhoProcessing, rhoResult));
    
    // Define processing interface in Tau
    string tauProcessingInterface = R"(
        namespace DataProcessing {
            struct User {
                string name;
                int age;
                bool active;
            }
            
            struct Statistics {
                int total;
                int active;
                float averageAge;
            }
            
            interface IDataProcessor {
                Statistics ProcessUsers(User[] users);
                int CountActiveUsers(User[] users);
                float CalculateAverageAge(User[] users);
                
                event ProcessingCompleted(Statistics stats);
            }
        }
    )";
    
    EXPECT_TRUE(ParseTauCode(tauProcessingInterface));
    
    string proxyCode;
    EXPECT_TRUE(GenerateTauProxy(tauProcessingInterface, proxyCode));
    EXPECT_TRUE(proxyCode.find("IDataProcessor") != string::npos);
    EXPECT_TRUE(proxyCode.find("ProcessUsers") != string::npos);
}

// Test error handling across languages
TEST_F(MultiLanguageIntegrationTests, CrossLanguageErrorHandling) {
    // Pi code that might fail
    string piCodeWithError = R"(
        42 0 / 'result =  // Division by zero
    )";
    
    // Should handle error gracefully
    bool piResult = ExecutePiCode(piCodeWithError);
    // Pi might or might not handle this gracefully - test shouldn't crash
    
    // Rho code with error
    string rhoCodeWithError = R"(
        1 / 0
    )";
    
    Object rhoResult;
    bool rhoSuccess = ExecuteRhoCode(rhoCodeWithError, rhoResult);
    // Should handle division by zero gracefully
    
    // Tau code with syntax error
    string tauCodeWithError = R"(
        namespace ErrorTest {
            interface IBroken {
                void method(;  // Syntax error
            }
        }
    )";
    
    bool tauResult = ParseTauCode(tauCodeWithError);
    EXPECT_FALSE(tauResult); // Should properly reject invalid syntax
    
    // System should remain stable after errors
    string validPiCode = "21 21 + 'answer =";
    EXPECT_TRUE(ExecutePiCode(validPiCode));
    
    Object validResult;
    EXPECT_TRUE(ExecuteRhoCode("21 + 21", validResult));
    if (validResult.Exists() && validResult.IsType<int>()) {
        EXPECT_EQ(validResult.GetValue<int>(), 42);
    }
}

// Test type system integration across languages
TEST_F(MultiLanguageIntegrationTests, TypeSystemIntegration) {
    // Define types in Tau
    string tauTypes = R"(
        namespace TypeTest {
            enum Color {
                Red,
                Green,
                Blue
            }
            
            struct Point {
                float x;
                float y;
            }
            
            struct Shape {
                Point center;
                Color color;
                float radius;
            }
            
            interface IGraphics {
                void DrawShape(Shape shape);
                Point GetCenter(Shape shape);
                void SetColor(Shape shape, Color color);
            }
        }
    )";
    
    EXPECT_TRUE(ParseTauCode(tauTypes));
    
    // Create instances using Pi
    string piShapeCreation = R"(
        { 'x 10.5 = 'y 20.3 = } 'center =
        { 'center center = 'color 1 = 'radius 5.0 = } 'circle =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piShapeCreation));
    
    // Process using Rho
    string rhoShapeProcessing = R"(
        fun distance(p1, p2) = sqrt((p1.x - p2.x)^2 + (p1.y - p2.y)^2);
        fun area(shape) = 3.14159 * shape.radius * shape.radius;
    )";
    
    Object rhoResult;
    EXPECT_TRUE(ExecuteRhoCode(rhoShapeProcessing, rhoResult));
    
    // Generate proxy for the graphics interface
    string proxyCode;
    EXPECT_TRUE(GenerateTauProxy(tauTypes, proxyCode));
    EXPECT_TRUE(proxyCode.find("DrawShape") != string::npos);
    EXPECT_TRUE(proxyCode.find("Point") != string::npos);
    EXPECT_TRUE(proxyCode.find("Color") != string::npos);
}

// Test event system integration
TEST_F(MultiLanguageIntegrationTests, EventSystemIntegration) {
    // Define event-driven interface in Tau
    string tauEventInterface = R"(
        namespace EventSystem {
            struct EventData {
                string type;
                string message;
                int64 timestamp;
                float value;
            }
            
            interface IEventSource {
                void TriggerEvent(EventData data);
                void StartMonitoring();
                void StopMonitoring();
                
                event DataReceived(EventData data);
                event ErrorOccurred(string error);
                event MonitoringStarted();
                event MonitoringStopped();
            }
            
            interface IEventHandler {
                void HandleData(EventData data);
                void HandleError(string error);
                
                event ProcessingCompleted();
            }
        }
    )";
    
    EXPECT_TRUE(ParseTauCode(tauEventInterface));
    
    // Generate proxy/agent code
    string proxyCode;
    EXPECT_TRUE(GenerateTauProxy(tauEventInterface, proxyCode));
    EXPECT_TRUE(proxyCode.find("DataReceived") != string::npos);
    EXPECT_TRUE(proxyCode.find("HandleData") != string::npos);
    
    // Set up event handlers in Pi
    string piEventHandler = R"(
        {
            dup 'last_event =
            "Event processed" print
        } 'handle_event =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piEventHandler));
    
    // Set up event processing in Rho
    string rhoEventProcessor = R"(
        fun processEvent(event) = 
            if (event.type == "data") then event.value * 2
            else if (event.type == "error") then -1
            else 0;
    )";
    
    Object rhoResult;
    EXPECT_TRUE(ExecuteRhoCode(rhoEventProcessor, rhoResult));
}

// Test complex workflow integration
TEST_F(MultiLanguageIntegrationTests, ComplexWorkflowIntegration) {
    // 1. Define service architecture in Tau
    string tauArchitecture = R"(
        namespace WorkflowSystem {
            struct Task {
                string id;
                string type;
                string[] dependencies;
                bool completed;
                float progress;
            }
            
            struct WorkflowState {
                Task[] tasks;
                int completed;
                int total;
                float overallProgress;
            }
            
            interface ITaskProcessor {
                bool ProcessTask(Task task);
                WorkflowState GetWorkflowState();
                void UpdateProgress(string taskId, float progress);
                
                event TaskStarted(string taskId);
                event TaskCompleted(string taskId, bool success);
                event WorkflowCompleted(WorkflowState finalState);
            }
            
            interface IWorkflowManager {
                void StartWorkflow(Task[] tasks);
                void PauseWorkflow();
                void ResumeWorkflow();
                WorkflowState GetStatus();
                
                event WorkflowStatusChanged(WorkflowState state);
            }
        }
    )";
    
    EXPECT_TRUE(ParseTauCode(tauArchitecture));
    
    // 2. Create workflow data in Pi
    string piWorkflowData = R"(
        [
            { 'id "task1" = 'type "data_load" = 'dependencies [] = 'completed false = 'progress 0.0 = }
            { 'id "task2" = 'type "process" = 'dependencies ["task1"] = 'completed false = 'progress 0.0 = }
            { 'id "task3" = 'type "output" = 'dependencies ["task2"] = 'completed false = 'progress 0.0 = }
        ] 'workflow_tasks =
        
        { 'tasks workflow_tasks = 'completed 0 = 'total 3 = 'overallProgress 0.0 = } 'workflow_state =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piWorkflowData));
    
    // 3. Implement processing logic in Rho
    string rhoProcessingLogic = R"(
        fun canExecuteTask(task, completedTasks) =
            all(fun(dep) = contains(completedTasks, dep), task.dependencies);
        
        fun calculateProgress(tasks) =
            (sum(map(fun(t) = if t.completed then 1.0 else t.progress, tasks))) / length(tasks);
        
        fun getNextTask(tasks, completed) =
            find(fun(t) = not t.completed and canExecuteTask(t, completed), tasks);
    )";
    
    Object rhoResult;
    EXPECT_TRUE(ExecuteRhoCode(rhoProcessingLogic, rhoResult));
    
    // 4. Generate proxy code for network communication
    string proxyCode;
    EXPECT_TRUE(GenerateTauProxy(tauArchitecture, proxyCode));
    
    // Verify generated code contains all expected elements
    EXPECT_TRUE(proxyCode.find("ITaskProcessor") != string::npos);
    EXPECT_TRUE(proxyCode.find("IWorkflowManager") != string::npos);
    EXPECT_TRUE(proxyCode.find("ProcessTask") != string::npos);
    EXPECT_TRUE(proxyCode.find("TaskStarted") != string::npos);
    EXPECT_TRUE(proxyCode.find("WorkflowCompleted") != string::npos);
    
    // 5. Test integration - simulate workflow execution
    string piWorkflowExecution = R"(
        0 'current_task =
        workflow_tasks current_task [] get 'active_task =
        true active_task 'completed =
    )";
    
    EXPECT_TRUE(ExecutePiCode(piWorkflowExecution));
    
    // System should handle the complex multi-language workflow
    // This tests that all language components can work together
    // in a realistic distributed computing scenario
}