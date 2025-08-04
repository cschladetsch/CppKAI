# KAI Test System Architecture

## Test Suite Organization

```mermaid
graph TB
    subgraph "Test Execution Framework"
        RUNNER[Test Runner<br/>Scripts/run_all_tests.sh]
        TIMEOUT[Timeout Management<br/>10 minutes per suite]
    end
    
    subgraph "Core System Tests"
        CORE_TESTS[TestCore<br/>Registry, Objects, Memory]
        FIXED_TESTS[FixedCoreSuite<br/>Regression tests]
        KAI_TESTS[KaiTest<br/>Integration tests]
    end
    
    subgraph "Language System Tests"
        PI_TESTS[TestPi<br/>Stack-based language<br/>25+ test cases]
        RHO_TESTS[TestRho<br/>Infix language<br/>100+ test cases]
        TAU_TESTS[TestTau<br/>IDL generator<br/>Network interfaces]
    end
    
    subgraph "Application Tests"
        CONSOLE_TESTS[TestConsole<br/>Interactive features<br/>Zsh-like commands]
        WINDOW_TESTS[TestWindow<br/>GUI functionality<br/>Performance tests]
    end
    
    subgraph "Network Tests"
        NET_TESTS[Network Tests<br/>P2P communication<br/>RakNet validation]
        CONSOLE_NET[Console Networking<br/>16KB test suite<br/>8 comprehensive tests]
    end
    
    subgraph "Script-based Tests"
        SHELL_TESTS[Shell Command Tests<br/>Backtick operations]
        RHO_SCRIPTS[Rho Script Tests<br/>File-based validation]
        PI_SCRIPTS[Pi Script Tests<br/>Stack operations]
    end
    
    RUNNER --> CORE_TESTS
    RUNNER --> FIXED_TESTS
    RUNNER --> PI_TESTS
    RUNNER --> RHO_TESTS
    RUNNER --> TAU_TESTS
    RUNNER --> CONSOLE_TESTS
    
    style RUNNER fill:#ffeb3b
    style CORE_TESTS fill:#4caf50
    style PI_TESTS fill:#9c27b0
    style RHO_TESTS fill:#9c27b0
    style CONSOLE_TESTS fill:#2196f3
    style NET_TESTS fill:#ff9800
```

## Test Infrastructure Components

```mermaid
classDiagram
    class TestFramework {
        <<GoogleTest>>
        +SetUp()
        +TearDown()
        +ASSERT_*()
        +EXPECT_*()
    }
    
    class TestCommon {
        +CreateRegistry()
        +CreateExecutor()
        +SetupEnvironment()
        +CleanupEnvironment()
    }
    
    class MyTestStruct {
        +int number
        +string text
        +TestMethod()
    }
    
    class LanguageTestBase {
        +Registry registry
        +Executor executor
        +TestTranslation()
        +TestExecution()
        +ValidateResult()
    }
    
    class ConsoleTestFixture {
        +Console console
        +NetworkSetup()
        +CommandExecution()
        +ResultValidation()
    }
    
    TestFramework <|-- TestCommon
    TestCommon <|-- LanguageTestBase
    TestCommon <|-- ConsoleTestFixture
    TestCommon --> MyTestStruct : uses
    
    class PiTestSuite {
        +BasicOperations()
        +StackManipulation()
        +ControlFlow()
        +BacktickCommands()
    }
    
    class RhoTestSuite {
        +BinaryOperations()
        +ControlStructures()
        +Functions()
        +CrossLanguage()
    }
    
    class ConsoleNetworkingTest {
        +BasicNetworkSetup()
        +SendCommandToPeer()
        +BroadcastCommand()
        +CrossLanguageCommunication()
    }
    
    LanguageTestBase <|-- PiTestSuite
    LanguageTestBase <|-- RhoTestSuite
    ConsoleTestFixture <|-- ConsoleNetworkingTest
```

## Test Execution Flow

```mermaid
sequenceDiagram
    participant User
    participant Runner as run_all_tests.sh
    participant TestSuite as Test Suite
    participant GTest as Google Test
    participant System as KAI System
    
    User->>Runner: ./Scripts/run_all_tests.sh
    Runner->>Runner: Initialize test environment
    
    loop For each test script
        Runner->>TestSuite: Execute test suite
        TestSuite->>GTest: Initialize test framework
        GTest->>System: SetUp test environment
        
        loop For each test case
            GTest->>System: Execute test
            System->>System: Run operations
            System->>GTest: Return results
            GTest->>GTest: Validate results
        end
        
        GTest->>System: TearDown test environment
        GTest->>TestSuite: Report results
        TestSuite->>Runner: Return exit code
    end
    
    Runner->>Runner: Aggregate results
    Runner->>User: Final test report
    
    Note over Runner,System: Timeout: 10 minutes per suite
    Note over GTest,System: Isolated test environments
```

## Language Test Coverage

```mermaid
graph TB
    subgraph "Pi Language Tests"
        PI_BASIC[Basic Operations<br/>Arithmetic, Stack ops]
        PI_CONTROL[Control Flow<br/>Labels, jumps, loops]
        PI_ADVANCED[Advanced Features<br/>Continuations, functions]
        PI_SHELL[Shell Integration<br/>Backtick commands]
        PI_NETWORK[Network Operations<br/>Distributed execution]
    end
    
    subgraph "Rho Language Tests"
        RHO_SYNTAX[Syntax Tests<br/>Expressions, statements]
        RHO_CONTROL[Control Structures<br/>if/while/for loops]
        RHO_FUNCTIONS[Function Tests<br/>Definitions, calls, scope]
        RHO_CROSS[Cross-language<br/>Pi embedding, translation]
        RHO_CONTAINERS[Container Tests<br/>Arrays, objects]
    end
    
    subgraph "Tau Language Tests"
        TAU_INTERFACE[Interface Definition<br/>Method signatures]
        TAU_STRUCT[Struct Generation<br/>Data types, fields]
        TAU_NAMESPACE[Namespace Support<br/>Qualified names]
        TAU_NETWORK[Network Generation<br/>Proxy/Agent creation]
        TAU_CODEGEN[Code Generation<br/>C++ output validation]
    end
    
    subgraph "Test Validation"
        COMPILE[Compilation Tests<br/>Syntax validation]
        EXECUTE[Execution Tests<br/>Runtime behavior] 
        RESULT[Result Validation<br/>Expected outputs]
        ERROR[Error Handling<br/>Exception behavior]
    end
    
    PI_BASIC --> COMPILE
    PI_CONTROL --> EXECUTE
    PI_ADVANCED --> RESULT
    
    RHO_SYNTAX --> COMPILE
    RHO_CONTROL --> EXECUTE
    RHO_FUNCTIONS --> RESULT
    
    TAU_INTERFACE --> COMPILE
    TAU_STRUCT --> EXECUTE
    TAU_CODEGEN --> RESULT
    
    COMPILE --> ERROR
    EXECUTE --> ERROR
    RESULT --> ERROR
    
    style PI_BASIC fill:#e3f2fd
    style RHO_SYNTAX fill:#e8f5e8
    style TAU_INTERFACE fill:#f3e5f5
    style COMPILE fill:#fff3e0
```

## Console Test Categories

```mermaid
graph LR
    subgraph "Interactive Features"
        REPL[REPL Testing<br/>Command processing<br/>History management]
        ZSH[Zsh-like Features<br/>!! expansion<br/>Word designators<br/>20 test cases]
        LANG_SWITCH[Language Switching<br/>pi ↔ rho commands<br/>State preservation]
    end
    
    subgraph "Networking Features"
        NET_SETUP[Network Setup<br/>/network start/stop<br/>Port management]
        PEER_MGMT[Peer Management<br/>/connect, /peers<br/>Connection handling]
        CMD_EXEC[Command Execution<br/>/@peer, /broadcast<br/>Remote operations]
        MSG_HISTORY[Message History<br/>/nethistory<br/>Communication logs]
    end
    
    subgraph "Error Handling"
        INVALID_CMD[Invalid Commands<br/>Error reporting]
        NET_ERRORS[Network Errors<br/>Connection failures<br/>Timeout handling]
        SYNTAX_ERR[Syntax Errors<br/>Language validation]
    end
    
    subgraph "Performance Tests"
        THROUGHPUT[Command Throughput<br/>High-frequency operations]
        MEMORY[Memory Usage<br/>Object lifecycle<br/>Garbage collection]
        NETWORK_PERF[Network Performance<br/>Message latency<br/>Bandwidth usage]
    end
    
    REPL --> NET_SETUP
    ZSH --> PEER_MGMT
    LANG_SWITCH --> CMD_EXEC
    
    NET_SETUP --> INVALID_CMD
    PEER_MGMT --> NET_ERRORS
    CMD_EXEC --> SYNTAX_ERR
    
    INVALID_CMD --> THROUGHPUT
    NET_ERRORS --> MEMORY
    SYNTAX_ERR --> NETWORK_PERF
    
    style REPL fill:#2196f3
    style NET_SETUP fill:#ff9800
    style INVALID_CMD fill:#f44336
    style THROUGHPUT fill:#4caf50
```

## Test Data Flow and Validation

```mermaid
graph TD
    subgraph "Test Input Generation"
        TEST_DATA[Test Data<br/>Scripts, commands, objects]
        MOCK_DATA[Mock Data<br/>Network messages, objects]
        RANDOM_DATA[Random Data<br/>Stress testing, edge cases]
    end
    
    subgraph "System Under Test"
        REGISTRY[Registry<br/>Object management]
        EXECUTOR[Executor<br/>Command execution]
        NETWORK[Network<br/>Communication layer]
        CONSOLE[Console<br/>User interface]
    end
    
    subgraph "Result Validation"
        OUTPUT_CHECK[Output Validation<br/>Expected vs actual]
        STATE_CHECK[State Validation<br/>System consistency]
        PERF_CHECK[Performance Validation<br/>Timing, memory usage]
        ERROR_CHECK[Error Validation<br/>Exception handling]
    end
    
    subgraph "Test Reporting"
        PASS_FAIL[Pass/Fail Status<br/>Individual test results]
        COVERAGE[Code Coverage<br/>Execution analysis]
        METRICS[Performance Metrics<br/>Timing, memory stats]
        LOGS[Test Logs<br/>Detailed execution traces]
    end
    
    TEST_DATA --> REGISTRY
    TEST_DATA --> EXECUTOR
    MOCK_DATA --> NETWORK
    RANDOM_DATA --> CONSOLE
    
    REGISTRY --> OUTPUT_CHECK
    EXECUTOR --> STATE_CHECK
    NETWORK --> PERF_CHECK
    CONSOLE --> ERROR_CHECK
    
    OUTPUT_CHECK --> PASS_FAIL
    STATE_CHECK --> COVERAGE
    PERF_CHECK --> METRICS
    ERROR_CHECK --> LOGS
    
    style TEST_DATA fill:#e3f2fd
    style REGISTRY fill:#4caf50
    style OUTPUT_CHECK fill:#fff3e0
    style PASS_FAIL fill:#ffeb3b
```

## Test Suite Statistics

```mermaid
pie title Test Suite Distribution
    "Rho Language Tests" : 100
    "Pi Language Tests" : 25
    "Tau Language Tests" : 15
    "Console Tests" : 20
    "Core System Tests" : 30
    "Network Tests" : 8
    "Integration Tests" : 12
```

## Continuous Integration Flow

```mermaid
graph LR
    subgraph "CI Pipeline"
        TRIGGER[Code Push<br/>Pull Request]
        BUILD[Build System<br/>./b script]
        UNIT[Unit Tests<br/>Individual components]
        INTEGRATION[Integration Tests<br/>Cross-component]
        PERFORMANCE[Performance Tests<br/>Benchmarking]
        REPORT[Test Report<br/>Results summary]
    end
    
    subgraph "Quality Gates"
        COMPILE_OK[Compilation Success<br/>All targets build]
        TESTS_PASS[Tests Pass<br/>>95% success rate]
        PERF_OK[Performance OK<br/>Within thresholds]
        COVERAGE_OK[Coverage OK<br/>>80% code coverage]
    end
    
    TRIGGER --> BUILD
    BUILD --> COMPILE_OK
    COMPILE_OK --> UNIT
    UNIT --> TESTS_PASS
    TESTS_PASS --> INTEGRATION
    INTEGRATION --> PERFORMANCE
    PERFORMANCE --> PERF_OK
    PERF_OK --> COVERAGE_OK
    COVERAGE_OK --> REPORT
    
    style TRIGGER fill:#4caf50
    style COMPILE_OK fill:#2196f3
    style TESTS_PASS fill:#ff9800
    style REPORT fill:#9c27b0
```