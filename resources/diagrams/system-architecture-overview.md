# KAI System Architecture Overview

The KAI system provides a multi-layered architecture that enables distributed
object programming with multiple language frontends. This diagram shows the
high-level layers and how they connect, from user applications down through the
languages, console/networking, execution engine, core object model, and the
platform/runtime and external dependencies.

```mermaid
graph TB
    subgraph "Application Layer"
        USER[User Applications]
        SCRIPTS[Scripts & Automation]
        TOOLS[Developer Tools]
    end
    
    subgraph "Language Layer"
        RHO[Rho Language<br/>Infix Syntax<br/>Python-like]
        PI[Pi Language<br/>Stack-based<br/>Forth-like]
        TAU[Tau Language<br/>Interface Definition<br/>Code Generation]
    end
    
    subgraph "Console & Networking"
        CONSOLE[Interactive Console<br/>Multi-language REPL]
        P2P[Peer-to-Peer<br/>Console Networking]
        NETCMD[Network Commands<br/>Remote Execution]
    end
    
    subgraph "Execution Engine"
        EXECUTOR[Stack-based Executor<br/>Virtual Machine]
        COMPILER[Multi-language<br/>Compiler/Translator]
        CONTINUATION[Continuation Support<br/>Advanced Control Flow]
    end
    
    subgraph "Core Object Model"
        REGISTRY[Type Registry<br/>Object Factory<br/>Reflection System]
        OBJECTS[Distributed Objects<br/>Network Transparency]
        GC[Incremental GC<br/>Tri-color Algorithm]
    end
    
    subgraph "Platform & Runtime"
        MEMORY[Memory Management<br/>Smart Pointers]
        SERIALIZE[Serialization<br/>Network Protocol]
        PLATFORM[Cross-platform<br/>Windows/Linux/macOS]
    end
    
    subgraph "External Dependencies"
        BOOST[Boost Libraries]
        ENET[ENet Networking]
        GTEST[Google Test]
        CMAKE[CMake Build]
    end
    
    USER --> RHO
    USER --> PI  
    USER --> TAU
    SCRIPTS --> CONSOLE
    TOOLS --> CONSOLE
    RHO --> COMPILER
    PI --> EXECUTOR
    TAU --> COMPILER
    CONSOLE --> EXECUTOR
    CONSOLE --> P2P
    P2P --> NETCMD
    NETCMD --> EXECUTOR
    COMPILER --> EXECUTOR
    EXECUTOR --> CONTINUATION
    EXECUTOR --> REGISTRY
    REGISTRY --> OBJECTS
    REGISTRY --> GC
    OBJECTS --> SERIALIZE
    EXECUTOR --> MEMORY
    OBJECTS --> MEMORY
    SERIALIZE --> PLATFORM
    PLATFORM --> BOOST
    P2P --> ENET
    PLATFORM --> GTEST
    PLATFORM --> CMAKE
    
    style USER fill:#e1f5fe
    style RHO fill:#c8e6c9
    style PI fill:#ffecb3
    style TAU fill:#f3e5f5
    style CONSOLE fill:#fff3e0
    style EXECUTOR fill:#e8f5e8
    style REGISTRY fill:#fce4ec
    style OBJECTS fill:#e0f2f1
    style GC fill:#f1f8e9
```
