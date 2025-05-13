# Tau Language Tutorial

Tau is KAI's Interface Definition Language (IDL), designed specifically for describing networked objects, interfaces, and services within the KAI distributed object model. It provides a way to define how components communicate across the network, enabling seamless distributed computing.

## Introduction to Tau

Tau serves as the bridge between different nodes in a KAI network, defining the contracts between distributed components. It is used to generate code for proxies, agents, and serialization across network boundaries.

### Key Characteristics

- **Interface-focused**: Defines the public interfaces of networked objects
- **Language-neutral**: Can generate code for multiple target languages
- **Network-oriented**: Designed for cross-network communication
- **Type-safe**: Ensures type compatibility across network boundaries
- **Declarative**: Focuses on what rather than how

## Basic Syntax

### Namespaces

Tau uses namespaces to organize definitions and prevent naming collisions:

```tau
namespace MyApplication {
    // Definitions go here
}
```

### Basic Type Definitions

```tau
namespace Finance {
    // Define a simple type
    struct Money {
        string currency;
        float amount;
    }
}
```

## Defining Interfaces

Interfaces are the core of Tau, defining the contract between distributed components:

```tau
namespace Trading {
    // Define an interface
    interface ITrader {
        // Method definitions
        bool PlaceOrder(string symbol, int quantity, float price);
        void CancelOrder(string orderId);
        Money GetBalance();
    }
}
```

### Data Types

Tau supports various data types to ensure proper serialization across the network:

```tau
namespace Common {
    // Primitive types
    struct TypeSamples {
        bool flag;
        int32 count;
        int64 bigNumber;
        float price;
        double preciseValue;
        string name;
        bytes rawData;
        date timestamp;
    }
    
    // Arrays and sequences
    struct Collections {
        int[] numbers;           // Array of integers
        string[] names;          // Array of strings
        Money[] transactions;    // Array of user-defined types
    }
}
```

### Enumerations

Tau supports enumerated types for sets of named values:

```tau
namespace Status {
    // Define an enumeration
    enum OrderStatus {
        Pending = 0,
        Filled = 1,
        PartiallyFilled = 2,
        Cancelled = 3,
        Rejected = 4
    }
}
```

## Advanced Features

### Inheritance

Tau supports interface inheritance for creating hierarchies of related interfaces:

```tau
namespace Security {
    interface IAuthentication {
        bool Authenticate(string username, string passwordHash);
        void Logout(string sessionId);
    }
    
    // Inherits and extends IAuthentication
    interface IAdvancedAuthentication : IAuthentication {
        bool AuthenticateWithToken(string token);
        bool ValidateToken(string token);
    }
}
```

### Generics/Templates

Tau supports generic/template types for type-parameterized interfaces:

```tau
namespace Collections {
    // Generic repository interface
    interface IRepository<T> {
        T GetById(string id);
        void Save(T item);
        void Delete(string id);
        T[] GetAll();
    }
}
```

### Asynchronous Methods

Tau provides syntax for defining asynchronous operations:

```tau
namespace Async {
    interface IDataService {
        // Synchronous method
        int GetValue(string key);
        
        // Asynchronous method (returns a Future/Promise)
        async Data FetchLargeData(string source);
        
        // Method with callback
        void ProcessInBackground(bytes data, callback OnComplete);
    }
}
```

### Events

Tau supports defining events that components can publish and subscribe to:

```tau
namespace Notifications {
    // Event data structure
    struct PriceUpdate {
        string symbol;
        float price;
        date timestamp;
    }
    
    interface IMarketDataService {
        // Define an event
        event PriceChanged(PriceUpdate update);
        
        // Methods to manage subscriptions
        void SubscribeToSymbol(string symbol);
        void UnsubscribeFromSymbol(string symbol);
    }
}
```

## Network Distribution

### Service Definitions

Services are top-level components that can be discovered and accessed across the network:

```tau
namespace Network {
    // Define a network service
    service UserService {
        implements IUserManagement;
        
        // Service metadata
        version = "1.0";
        discoverable = true;
        
        // Deployment constraints
        requires {
            minMemory = "512MB";
            minCPUCores = 2;
        }
    }
}
```

### Remote Procedure Calls (RPCs)

Tau explicitly marks methods that can be called remotely:

```tau
namespace RPC {
    interface ICalculator {
        // Remote method with error handling
        @remote int Add(int a, int b) throws NetworkException;
        
        // Remote method with timeout
        @remote(timeout=5000) float Divide(float a, float b) throws DivideByZeroException, TimeoutException;
    }
}
```

## Code Generation

### Target Languages

Tau can generate code for multiple target languages:

```tau
// Generate code for C++, C#, and Python
@generate(languages=["cpp", "csharp", "python"])
namespace CrossPlatform {
    interface IDataAccess {
        Data GetRecord(string id);
        void SaveRecord(Data record);
    }
}
```

### Customization Options

Tau allows customization of the generated code:

```tau
// Customizing the output location and namespace
@generate(
    language="cpp",
    outputPath="../generated/cpp",
    namespace="com::example::services"
)
namespace CustomGeneration {
    // Definitions here
}
```

## Integration with KAI

### Object Registry

Tau definitions integrate with KAI's object registry system:

```tau
namespace KAIIntegration {
    // Define a registrable type
    @registrable
    class Vector3 {
        float x;
        float y;
        float z;
        
        // Method that will be exposed to KAI
        Vector3 Normalize();
    }
}
```

### Network Proxies

Tau generates proxies for transparent network communication:

```tau
namespace Proxies {
    // Generate network proxies automatically
    @proxy
    interface IRemoteManager {
        void Start();
        void Stop();
        Status GetStatus();
    }
}
```

## Best Practices

1. **Focus on Interfaces**: Define clear interfaces rather than implementation details
2. **Version Everything**: Include version information for backward compatibility
3. **Exception Handling**: Define precise exception types for error scenarios
4. **Documentation**: Include descriptive comments for interfaces and methods
5. **Minimize Dependencies**: Keep interfaces focused with minimal dependencies
6. **Consider Bandwidth**: Design with network constraints in mind

## Example: Complete Service Definition

Here's a comprehensive example of a complete service definition in Tau:

```tau
// Define the namespace
namespace Trading {
    // Data structures
    struct Order {
        string id;
        string symbol;
        int quantity;
        float price;
        OrderType type;
        OrderStatus status;
        date createdAt;
    }
    
    // Enumerations
    enum OrderType {
        Market = 0,
        Limit = 1,
        StopLoss = 2
    }
    
    enum OrderStatus {
        Pending = 0,
        Filled = 1,
        PartiallyFilled = 2,
        Cancelled = 3,
        Rejected = 4
    }
    
    // Exceptions
    exception InsufficientFundsException {
        string message;
        float available;
        float required;
    }
    
    exception InvalidOrderException {
        string message;
        string reason;
    }
    
    // Main interface
    @versioned(1.0)
    interface ITradingService {
        // Order operations
        @remote Order PlaceOrder(string symbol, int quantity, float price, OrderType type) 
            throws InsufficientFundsException, InvalidOrderException;
        
        @remote bool CancelOrder(string orderId) 
            throws OrderNotFoundException;
        
        @remote Order GetOrderById(string orderId);
        
        @remote Order[] GetOrdersByStatus(OrderStatus status);
        
        // Account operations
        @remote float GetAccountBalance();
        
        // Events
        event OrderStatusChanged(Order order);
        event AccountBalanceChanged(float newBalance);
        
        // Subscriptions
        void SubscribeToSymbol(string symbol);
        void UnsubscribeFromSymbol(string symbol);
    }
    
    // Define the service
    @generate(languages=["cpp", "csharp"])
    service TradingService {
        implements ITradingService;
        
        version = "1.0.5";
        discoverable = true;
        
        requires {
            minMemory = "1GB";
            minCPUCores = 2;
            dependencies = ["DatabaseService", "AuthenticationService"];
        }
    }
}
```

## Real-World Applications

Tau is particularly suited for:

1. **Distributed systems**: Defining interfaces between networked components
2. **Microservices**: Creating contracts between service boundaries
3. **Cross-platform applications**: Generating code for multiple languages
4. **Real-time systems**: Defining event-based communication channels
5. **Service discovery**: Advertising and locating network services

## Conclusion

Tau provides a powerful way to define and coordinate distributed components in a KAI system. By focusing on clear interface definitions, it enables transparent cross-network communication while maintaining type safety and versioning. As you build more complex distributed applications with KAI, Tau becomes an essential tool for ensuring robust communication between components.

For practical examples, explore the [Tau test scripts](../Test/Language/TestTau/Scripts) which demonstrate various aspects of the language.