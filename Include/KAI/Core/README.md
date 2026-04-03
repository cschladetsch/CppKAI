# KAI Core System

The KAI Core system provides the fundamental infrastructure for the distributed object model, including object lifecycle management, type system, memory management, and the foundational services that all other KAI components depend upon.

## Overview

The Core system is the heart of KAI, providing:

- **Registry**: Central object factory and type system
- **Object Model**: Unified object representation with reflection
- **Memory Management**: Incremental garbage collection and smart object lifecycle
- **Type System**: Dynamic typing with compile-time and runtime type safety
- **Configuration**: System-wide configuration and settings management
- **Event System**: Observer pattern implementation for system events

## Core Architecture

### Registry System

The Registry is KAI's central object factory and type management system:

```cpp
// Object creation and management
auto registry = Registry::Create();
auto object = registry->New<MyClass>(constructor_args);
auto shared = registry->New("TypeName", args...);

// Type registration and reflection
registry->AddClass<MyClass>("MyClass");
registry->AddMethod<MyClass>("methodName", &MyClass::method);
```

**Key Features:**
- **Type-safe object creation**: Factory pattern with compile-time type checking
- **Reflection system**: Runtime introspection of types and their members
- **Cross-network object references**: Objects can be referenced across network boundaries
- **Automatic lifecycle management**: Integration with garbage collector

### Object Model

KAI's object model provides a unified representation for all objects in the system:

```cpp
class Object : public ObjectBase {
public:
    // Core object interface
    virtual Object* Clone() const = 0;
    virtual void MarkGray() = 0;
    virtual bool IsConst() const = 0;
    virtual Type::Number GetTypeNumber() const = 0;
    
    // Registry integration
    Registry* GetRegistry() const;
    Handle GetHandle() const;
    
    // Reflection support
    virtual PropertyBase* GetProperty(Label const&) = 0;
    virtual MethodBase* GetMethod(Label const&) = 0;
};
```

**Key Features:**
- **Unified interface**: All KAI objects inherit from common base
- **Handle system**: Lightweight references with automatic cleanup
- **Property/method access**: Dynamic member access by name
- **Serialization support**: Network-transparent object marshaling

### Memory Management

KAI uses an incremental tri-color garbage collector for automatic memory management:

```cpp
// Garbage collection is automatic, but can be controlled
registry->GetGarbageCollector()->SetCollectionFrequency(1000);
registry->GetGarbageCollector()->PerformFullCollection();

// Objects are automatically tracked
auto obj = registry->New<MyClass>();  // Automatically registered for GC
// No explicit deletion needed - GC handles cleanup
```

**Tri-Color Algorithm:**
- **White objects**: Unreachable, candidates for collection
- **Gray objects**: Reachable but not yet processed  
- **Black objects**: Reachable and fully processed

**Benefits:**
- **Incremental collection**: No stop-the-world pauses
- **Cycle detection**: Handles circular references correctly
- **Low overhead**: Minimal impact on application performance
- **Deterministic cleanup**: Predictable memory usage patterns

### Type System

KAI's type system provides both static and dynamic typing capabilities:

```cpp
// Static typing at compile time
auto typed_object = registry->New<Vector3>(1.0f, 2.0f, 3.0f);

// Dynamic typing at runtime  
auto dynamic_object = registry->New("Vector3", Value(1.0f), Value(2.0f), Value(3.0f));

// Type inspection
Type::Number type_num = object->GetTypeNumber();
std::string type_name = registry->GetTypeName(type_num);
```

**Type Registration:**
```cpp
// Register a new type with the system
registry->AddClass<MyClass>("MyClass")
    .Methods
        ("method1", &MyClass::Method1)
        ("method2", &MyClass::Method2)
    .Properties
        ("property1", &MyClass::property1)
        ("property2", &MyClass::property2);
```

### Configuration System

System-wide configuration management:

```cpp
// Configuration access
auto config = registry->GetConfig();
config->Set("network.port", 8080);
config->Set("debug.trace_level", 2);

// Type-safe configuration retrieval
int port = config->Get<int>("network.port");
bool debug_enabled = config->Get<bool>("debug.enabled", false); // with default
```

**Configuration Features:**
- **Hierarchical settings**: Dot-notation for nested configuration
- **Type-safe access**: Compile-time type checking for configuration values
- **Default values**: Fallback values when settings are not defined
- **Runtime modification**: Configuration can be changed during execution
- **Persistence**: Configuration can be saved/loaded from files

## Core Components

### Directory Structure

```
Include/KAI/Core/
├── README.md                    # This file
├── Object/                      # Object model implementation
│   ├── Object.h                 # Base object interface
│   ├── ObjectBase.h             # Object implementation base
│   ├── Handle.h                 # Object handle system
│   └── README.md                # Object system documentation
├── Config/                      # Configuration management
│   ├── Config.h                 # Configuration interface
│   ├── ConfigBase.h             # Configuration implementation
│   └── README.md                # Configuration documentation
├── Registry/                    # Central object registry
│   ├── Registry.h               # Registry interface
│   ├── RegistryBase.h           # Registry implementation
│   └── Class.h                  # Type registration utilities
├── Type/                        # Type system
│   ├── Type.h                   # Type definitions
│   ├── Number.h                 # Type numbering system
│   └── Traits.h                 # Type trait utilities
├── Memory/                      # Memory management
│   ├── MemorySystem.h           # Memory subsystem interface
│   └── GarbageCollector.h       # GC implementation
└── Events/                      # Event system
    ├── Event.h                  # Event base classes
    └── EventSystem.h            # Event dispatch system
```

### Key Headers

- **Registry.h**: Central object factory and type system
- **Object.h**: Base object interface and handle system
- **Config.h**: System configuration management
- **Type.h**: Type system definitions and utilities
- **MemorySystem.h**: Memory management and garbage collection
- **Event.h**: Event system for loose coupling

## Core Patterns and Principles

### Factory Pattern
The Registry implements the factory pattern for object creation:
```cpp
// Type-safe factory method
template<class T, class... Args>
Pointer<T> New(Args&&... args);

// Dynamic factory method
Pointer<Object> New(const String& type_name, const Arguments& args);
```

### Handle System
Lightweight object references that automatically update when objects move:
```cpp
Handle handle = object->GetHandle();
auto retrieved = registry->Get(handle);  // Always valid if object exists
```

### Observer Pattern
Event system for decoupled communication:
```cpp
registry->GetEventSystem()->Subscribe<MyEvent>([](const MyEvent& event) {
    // Handle event
});

registry->GetEventSystem()->Publish(MyEvent{data});
```

### Reflection System
Runtime introspection and dynamic method/property access:
```cpp
auto method = object->GetMethod("methodName");
auto result = method->Invoke(object, arguments);

auto property = object->GetProperty("propertyName");  
property->SetValue(object, new_value);
```

## Integration with Other Systems

### Language Integration
The Core system provides the foundation for all KAI languages:
- **Pi**: Direct access to registry for variable storage and retrieval
- **Rho**: Type-safe object creation and method calling
- **Tau**: Type registration for network proxy/agent generation

### Network Integration
Core objects are network-transparent:
- **Serialization**: Automatic marshaling of object state
- **Remote references**: Objects can be referenced across network boundaries
- **Distributed GC**: Garbage collection works across network nodes

### Executor Integration
The Executor relies on Core services:
- **Object storage**: Registry provides object lifetime management
- **Type dispatch**: Method calls use reflection system
- **Memory management**: GC ensures proper cleanup of execution artifacts

## Performance Characteristics

### Object Creation
- **Fast allocation**: Optimized object factory with memory pooling
- **Type registration overhead**: One-time cost during system initialization
- **Handle dereferencing**: O(1) lookup with minimal overhead

### Memory Management
- **Incremental GC**: Spreads collection work across many cycles
- **Low pause times**: Typically <1ms collection cycles
- **Memory overhead**: ~8-16 bytes per object for GC metadata
- **Collection frequency**: Tunable based on allocation rate

### Type System
- **Method dispatch**: Hash table lookup, typically O(1)
- **Property access**: Direct memory access after initial lookup
- **Type checking**: Compile-time where possible, minimal runtime cost

## Usage Examples

### Basic Object Management
```cpp
// Create registry
auto registry = Registry::Create();

// Register custom type
registry->AddClass<MyClass>("MyClass")
    .Methods("calculate", &MyClass::Calculate)
    .Properties("value", &MyClass::value);

// Create and use objects
auto obj = registry->New<MyClass>(42);
obj->Calculate();

// Dynamic access
auto dynamic_obj = registry->New("MyClass", Value(42));
auto method = dynamic_obj->GetMethod("calculate");
method->Invoke(dynamic_obj);
```

### Configuration Management
```cpp
auto config = registry->GetConfig();

// Set configuration values
config->Set("app.window.width", 1920);
config->Set("app.window.height", 1080);
config->Set("app.debug.enabled", true);

// Use configuration
int width = config->Get<int>("app.window.width");
bool debug = config->Get<bool>("app.debug.enabled", false);
```

### Event System
```cpp
// Define custom event
struct ConfigChangedEvent {
    std::string key;
    Value old_value;
    Value new_value;
};

// Subscribe to events
auto subscription = registry->GetEventSystem()->Subscribe<ConfigChangedEvent>(
    [](const ConfigChangedEvent& event) {
        std::cout << "Config changed: " << event.key << std::endl;
    });

// Publish events
registry->GetEventSystem()->Publish(ConfigChangedEvent{
    "app.debug.level", Value(1), Value(2)
});
```

## See Also

- **[Object System Documentation](Object/README.md)** - Detailed object model implementation
- **[Configuration Documentation](Config/README.md)** - Configuration system details  
- **[Executor Integration](../Executor/README.md)** - How Core integrates with the virtual machine
- **[Language Integration](../Language/README.md)** - Core system language support
- **[Main Documentation Hub](../../../Doc/Documentation.md)** - Central navigation point
- **[Architecture Resources](../../../resources/README.md)** - System architecture diagrams

The Core system provides the solid foundation upon which all KAI functionality is built, ensuring consistent behavior, automatic memory management, and seamless integration across the entire distributed object model.