# KAI Build System Architecture

## CMake Project Structure

```mermaid
graph TB
    subgraph "Root Configuration"
        ROOT[CMakeLists.txt<br/>Project setup, compiler selection<br/>C++23 standard, dependencies]
    end
    
    subgraph "Library Layer"
        LIB_CMAKE[Source/Library/CMakeLists.txt<br/>Core libraries definition]
        
        subgraph "Core Libraries"
            CORE[Core Library<br/>Registry, Objects, Memory]
            EXECUTOR[Executor Library<br/>Virtual Machine, Stack]
            NETWORK[Network Library<br/>Distributed computing<br/>(conditional)]
        end
        
        subgraph "Language Libraries"
            COMMON_LANG[CommonLang<br/>Shared language infrastructure]
            PI_LANG[PiLang<br/>Stack-based language]
            RHO_LANG[RhoLang<br/>Infix language]
            TAU_LANG[TauLang<br/>IDL generator]
        end
    end
    
    subgraph "Application Layer"
        APP_CMAKE[Source/App/CMakeLists.txt<br/>Application definitions]
        
        subgraph "Applications"
            CONSOLE[Console<br/>Interactive shell]
            WINDOW[Window<br/>GUI application<br/>(optional)]
            NETTEST[NetworkTest<br/>Network validation]
            NETGEN[NetworkGenerate<br/>Code generation]
        end
    end
    
    subgraph "Test Layer"
        TEST_CMAKE[Test/CMakeLists.txt<br/>Test suite configuration]
        
        subgraph "Test Categories"
            CORE_TESTS[Core Tests<br/>Unit tests for core systems]
            LANG_TESTS[Language Tests<br/>Pi, Rho, Tau validation]
            NET_TESTS[Network Tests<br/>Distributed computing]
            CONSOLE_TESTS[Console Tests<br/>Interactive features]
        end
    end
    
    ROOT --> LIB_CMAKE
    ROOT --> APP_CMAKE
    ROOT --> TEST_CMAKE
    
    LIB_CMAKE --> CORE
    LIB_CMAKE --> EXECUTOR
    LIB_CMAKE --> NETWORK
    LIB_CMAKE --> COMMON_LANG
    LIB_CMAKE --> PI_LANG
    LIB_CMAKE --> RHO_LANG
    LIB_CMAKE --> TAU_LANG
    
    APP_CMAKE --> CONSOLE
    APP_CMAKE --> WINDOW
    APP_CMAKE --> NETTEST
    APP_CMAKE --> NETGEN
    
    TEST_CMAKE --> CORE_TESTS
    TEST_CMAKE --> LANG_TESTS
    TEST_CMAKE --> NET_TESTS
    TEST_CMAKE --> CONSOLE_TESTS
    
    style ROOT fill:#ffeb3b
    style CORE fill:#4caf50
    style CONSOLE fill:#2196f3
    style CORE_TESTS fill:#ff9800
```

## Dependency Graph

```mermaid
graph LR
    subgraph "External Dependencies"
        BOOST[Boost Libraries<br/>system, filesystem<br/>program_options, regex]
        GTEST[Google Test<br/>Unit testing framework]
        RAKNET[RakNet<br/>Network communication<br/>(stub implementation)]
    end
    
    subgraph "Core Layer"
        CORE[Core<br/>Registry, Objects<br/>Base functionality]
    end
    
    subgraph "Language Layer"
        COMMON[CommonLang<br/>Shared infrastructure]
        PI[PiLang<br/>Stack language]
        RHO[RhoLang<br/>Infix language] 
        TAU[TauLang<br/>IDL generator]
    end
    
    subgraph "Execution Layer"
        EXECUTOR[Executor<br/>Virtual machine]
        NETWORK[Network<br/>Distributed computing]
    end
    
    subgraph "Application Layer"
        CONSOLE[Console Application]
        WINDOW[Window Application]
        TESTS[Test Executables]
    end
    
    BOOST --> CORE
    GTEST --> TESTS
    RAKNET --> NETWORK
    
    CORE --> COMMON
    CORE --> EXECUTOR
    COMMON --> PI
    PI --> RHO
    COMMON --> TAU
    
    EXECUTOR --> CONSOLE
    EXECUTOR --> WINDOW
    NETWORK --> CONSOLE
    
    PI --> TESTS
    RHO --> TESTS
    TAU --> TESTS
    EXECUTOR --> TESTS
    
    style BOOST fill:#e1f5fe
    style CORE fill:#4caf50
    style CONSOLE fill:#2196f3
    style TESTS fill:#ff9800
```

## Build Configuration Options

```mermaid
graph TD
    subgraph "Compiler Selection"
        COMPILER{Compiler Choice}
        COMPILER --> CLANG[Clang++ (default)<br/>Modern C++ support<br/>Better diagnostics]
        COMPILER --> GCC[GCC<br/>Alternative option<br/>-DBUILD_GCC=ON]
    end
    
    subgraph "Build Features"
        FEATURES[Build Features]
        FEATURES --> CPP23[C++23 Standard<br/>Latest language features]
        FEATURES --> NINJA[Ninja Generator<br/>Fast parallel builds]
        FEATURES --> OUTDIR[Output Directory<br/>Bin/ for all executables]
    end
    
    subgraph "Optional Components"
        OPTIONS[Component Options]
        OPTIONS --> RAKNET_OPT[KAI_BUILD_RAKNET<br/>Enable real RakNet<br/>Default: OFF (stub)]
        OPTIONS --> NETWORK_OPT[KAI_BUILD_NETWORK_APPS<br/>Network applications<br/>Default: OFF]
        OPTIONS --> TESTS_OPT[KAI_BUILD_TEST_ALL<br/>Complete test suite<br/>Default: ON]
        OPTIONS --> SHELL_OPT[ENABLE_SHELL_SYNTAX<br/>Backtick operations<br/>Security setting]
    end
    
    subgraph "Platform Support"
        PLATFORMS[Platform Configuration]
        PLATFORMS --> LINUX[Linux<br/>Primary development]
        PLATFORMS --> WINDOWS[Windows<br/>Cross-platform support]
        PLATFORMS --> MACOS[macOS<br/>Unix compatibility]
    end
    
    style CLANG fill:#4caf50
    style CPP23 fill:#2196f3
    style RAKNET_OPT fill:#ff9800
    style LINUX fill:#9c27b0
```

## Build Process Flow

```mermaid
sequenceDiagram
    participant User
    participant BuildScript as ./b (Build Script)
    participant CMake
    participant Ninja
    participant Compiler as Clang++
    participant Linker
    
    User->>BuildScript: ./b
    BuildScript->>BuildScript: Create build directory
    BuildScript->>CMake: Configure project
    CMake->>CMake: Process CMakeLists.txt
    CMake->>CMake: Find dependencies (Boost, GTest)
    CMake->>CMake: Generate build.ninja
    CMake->>BuildScript: Configuration complete
    
    BuildScript->>Ninja: Build with 24 parallel jobs
    Ninja->>Compiler: Compile source files
    Compiler->>Compiler: C++23 compilation
    Compiler->>Linker: Object files
    Linker->>Linker: Create libraries
    Linker->>Linker: Link executables
    Linker->>Ninja: Built targets
    Ninja->>BuildScript: Build complete
    
    BuildScript->>User: Executables in Bin/
    
    Note over CMake,Ninja: Dependency tracking
    Note over Compiler,Linker: Parallel compilation
```

## Output Structure

```mermaid
graph TB
    subgraph "Build Output Directory Structure"
        BIN[Bin/<br/>All executables]
        
        subgraph "Main Executables"
            CONSOLE_BIN[Console<br/>Interactive shell]
            WINDOW_BIN[Window<br/>GUI application]
        end
        
        subgraph "Test Executables"
            TEST_DIR[Test/<br/>Test executables]
            CORE_TEST[TestCore<br/>Core system tests]
            PI_TEST[TestPi<br/>Pi language tests]
            RHO_TEST[TestRho<br/>Rho language tests]
            TAU_TEST[TestTau<br/>Tau IDL tests]
            CONSOLE_TEST[TestConsole<br/>Console feature tests]
        end
        
        subgraph "Libraries"
            LIB_DIR[Lib/<br/>Static libraries]
            CORE_LIB[libCore.a]
            EXECUTOR_LIB[libExecutor.a]
            LANG_LIBS[libPiLang.a<br/>libRhoLang.a<br/>libTauLang.a]
        end
    end
    
    BIN --> CONSOLE_BIN
    BIN --> WINDOW_BIN
    BIN --> TEST_DIR
    
    TEST_DIR --> CORE_TEST
    TEST_DIR --> PI_TEST
    TEST_DIR --> RHO_TEST
    TEST_DIR --> TAU_TEST
    TEST_DIR --> CONSOLE_TEST
    
    BIN --> LIB_DIR
    LIB_DIR --> CORE_LIB
    LIB_DIR --> EXECUTOR_LIB
    LIB_DIR --> LANG_LIBS
    
    style BIN fill:#ffeb3b
    style CONSOLE_BIN fill:#2196f3
    style TEST_DIR fill:#ff9800
    style LIB_DIR fill:#4caf50
```

## Build Targets and Rules

```mermaid
graph LR
    subgraph "Core Targets"
        CORE_TARGET[Core Target<br/>Basic object system]
        EXECUTOR_TARGET[Executor Target<br/>Virtual machine]
        NETWORK_TARGET[Network Target<br/>Distributed computing]
    end
    
    subgraph "Language Targets"
        COMMON_TARGET[CommonLang Target<br/>Shared infrastructure]
        PI_TARGET[PiLang Target<br/>Stack language]
        RHO_TARGET[RhoLang Target<br/>Infix language]
        TAU_TARGET[TauLang Target<br/>IDL system]
    end
    
    subgraph "Application Targets"
        CONSOLE_TARGET[Console Target<br/>Main application]
        WINDOW_TARGET[Window Target<br/>GUI application]
        TEST_TARGETS[Test Targets<br/>Validation suite]
    end
    
    subgraph "Build Rules"
        COMPILE[Compile Rule<br/>C++ source files]
        LINK_LIB[Link Library Rule<br/>Static libraries]
        LINK_EXE[Link Executable Rule<br/>Final binaries]
    end
    
    CORE_TARGET --> COMPILE
    EXECUTOR_TARGET --> COMPILE
    COMMON_TARGET --> COMPILE
    PI_TARGET --> COMPILE
    RHO_TARGET --> COMPILE
    TAU_TARGET --> COMPILE
    
    COMPILE --> LINK_LIB
    LINK_LIB --> CONSOLE_TARGET
    LINK_LIB --> WINDOW_TARGET
    LINK_LIB --> TEST_TARGETS
    
    CONSOLE_TARGET --> LINK_EXE
    WINDOW_TARGET --> LINK_EXE
    TEST_TARGETS --> LINK_EXE
    
    style COMPILE fill:#4caf50
    style LINK_LIB fill:#2196f3
    style LINK_EXE fill:#ff9800
```