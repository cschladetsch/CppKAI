# Event System

## Overview

The KAI Event System provides a modern C++23 implementation of the Observer pattern through a multi-cast event mechanism. Events allow objects to communicate and respond to state changes without tight coupling. An event can have multiple subscribers (listeners) that are notified when the event is triggered.

## Features

- **Variadic Templates**: Events can have any number of parameters with any types
- **Type Safety**: Uses C++23 concepts to ensure type correctness at compile time
- **Memory Safety**: Uses smart pointers to manage delegate lifetimes
- **Ease of Use**: Simple syntax for subscribing to and unsubscribing from events
- **Flexibility**: Support for both functions and methods as event handlers

## Usage Examples

### Basic Example

```cpp
// Define an event with two parameters
Event<int, std::string> onValueChanged;

// Register a free function as a handler
void handleValueChanged(int id, std::string name) {
    std::cout << "Value changed: " << id << ", " << name << std::endl;
}
onValueChanged += handleValueChanged;

// Fire the event
onValueChanged(42, "Hello World");
```

### Class Methods as Event Handlers

```cpp
class Logger {
public:
    void logMessage(int priority, std::string message) {
        std::cout << "[Priority: " << priority << "] " << message << std::endl;
    }
};

// Create an instance
Logger logger;

// Register the method
onValueChanged += std::pair(&logger, &Logger::logMessage);

// Fire the event - will call both handleValueChanged and logger.logMessage
onValueChanged(1, "Test message");
```

### Unregistering Handlers

```cpp
// Remove a function handler
onValueChanged -= handleValueChanged;

// Remove a method handler
onValueChanged -= std::pair(&logger, &Logger::logMessage);
```

## Implementation Details

The Event system utilizes several modern C++ features:

1. **Concepts**: Ensuring handlers match the event signature
2. **Variadic Templates**: Supporting any number of parameters
3. **Perfect Forwarding**: Preserving value categories during event firing
4. **Smart Pointers**: Memory safety for event handlers
5. **Type Erasure**: Allowing different types of callables to be stored

## Event Delegates

The Event system supports three types of delegates:

1. **Function Delegates**: Regular functions and lambda expressions
2. **Method Delegates**: Member functions of classes
3. **Object Method Delegates**: Methods bound to objects in the KAI registry

## Performance Considerations

- Events use type erasure which may involve some runtime overhead
- Smart pointers provide memory safety but with a minor performance cost
- Variadic templates have no runtime overhead but increase compilation time

## Best Practices

1. Keep event handlers simple and fast
2. Avoid invoking events within other event handlers to prevent cascades
3. Always unregister handlers before the handler object is destroyed
4. Use const references for large parameter types to avoid unnecessary copies

## Technical Notes

- Events are not thread-safe by default
- Events do not guarantee the order of handler invocation
- Return values from handlers are ignored (all handlers must return void)

## Complete Example

For a complete working example, see [EventExample.cpp](EventExample.cpp)