# Smart Pointer Migration Plan for KAI

## Overview

This document outlines the systematic migration from raw pointers to smart pointers in the KAI codebase to improve memory safety, reduce manual memory management, and prevent memory leaks.

## Principles

1. **Ownership Clarity**: Every dynamically allocated object must have clear ownership
2. **RAII**: Resource Acquisition Is Initialization - use RAII wrappers
3. **Type Safety**: Prefer type-safe smart pointers over void* or raw pointers
4. **Backward Compatibility**: Provide deprecation path for existing APIs

## Smart Pointer Types and Usage

### std::unique_ptr
- **Use for**: Single ownership, factory functions, internal resources
- **Examples**: Methods, Properties, allocated buffers
- **Benefits**: Zero overhead, move semantics, automatic cleanup

### std::shared_ptr  
- **Use for**: Shared ownership, registry objects, cached resources
- **Examples**: ClassBase instances, StorageBase in Registry
- **Benefits**: Reference counting, thread-safe, weak_ptr support

### std::weak_ptr
- **Use for**: Breaking circular references, observers, caches
- **Examples**: Parent-child relationships, event listeners
- **Benefits**: Prevents circular references, can check validity

### Raw Pointers (T*)
- **Use for**: Non-owning references, optional parameters, arrays
- **Examples**: Function parameters, temporary references
- **Rule**: Never own memory through raw pointers

## Migration Priority

### Phase 1: Critical Memory Leaks (Week 1-2)
1. **ClassBuilder** ✓ Already fixed - uses unique_ptr
2. **Factory Methods** - MakeMethod, MakeProperty, MakeFunction
3. **ClassBase** - Methods and Properties collections

### Phase 2: Core Infrastructure (Week 3-4)
1. **Registry** - StorageBase management with shared_ptr
2. **Object/Storage** - Reference counting integration
3. **Tree** - Parent-child relationships with weak_ptr

### Phase 3: Language System (Week 5-6)
1. **Lexer/Parser** - Token and AST node ownership
2. **Translator** - Generated code ownership
3. **Executor** - Stack frame management

### Phase 4: Network Layer (Week 7-8)
1. **Network RAII** - Connection and session management
2. **Agent/Proxy** - Lifecycle management
3. **External Resources** - RakNet wrapper

## Implementation Strategy

### Step 1: Create Smart Pointer Versions
Create new headers with _SmartPtr suffix containing updated versions:
- ClassBase_SmartPtr.h
- Method_SmartPtr.h
- Registry_SmartPtr.h

### Step 2: Update Factory Functions
Change return types from raw pointers to smart pointers:
```cpp
// Old
MethodBase* MakeMethod(...);

// New  
std::unique_ptr<MethodBase> MakeMethod(...);
```

### Step 3: Update Container Types
Replace raw pointer containers with smart pointer versions:
```cpp
// Old
std::map<Label, MethodBase*> methods;

// New
std::map<Label, std::unique_ptr<MethodBase>> methods;
```

### Step 4: Add Compatibility Layer
Provide deprecated compatibility functions:
```cpp
[[deprecated("Use version returning unique_ptr")]]
MethodBase* MakeMethodRaw(...) {
    return MakeMethod(...).release();
}
```

### Step 5: Update Calling Code
Systematically update all callers to use new APIs.

## Registry Refactoring Example

### Current Issues
- Manual memory management of StorageBase instances
- Complex lifecycle with unclear ownership
- Potential leaks on exceptions

### Proposed Solution
```cpp
class Registry {
    // Old
    typedef std::map<Handle, StorageBase*> Instances;
    
    // New
    typedef std::map<Handle, std::shared_ptr<StorageBase>> Instances;
    
    // For objects that need weak references
    typedef std::map<Handle, std::weak_ptr<StorageBase>> WeakInstances;
};
```

## Network Layer RAII

### Connection Management
```cpp
class ManagedConnection {
    std::unique_ptr<RakNet::Connection, ConnectionDeleter> conn_;
public:
    ManagedConnection(RakNet::Connection* c) : conn_(c) {}
    ~ManagedConnection() { /* automatic cleanup */ }
};
```

### Node Lifecycle
```cpp
class Node {
    std::map<Handle, std::shared_ptr<AgentBase>> agents_;
    std::map<Handle, std::shared_ptr<ProxyBase>> proxies_;
};
```

## Testing Strategy

1. **Unit Tests**: Test each refactored component
2. **Integration Tests**: Ensure components work together
3. **Memory Tests**: Use valgrind/sanitizers to verify no leaks
4. **Performance Tests**: Ensure no significant overhead

## Rollback Plan

If issues arise:
1. Keep old headers available
2. Use preprocessor flags to switch implementations
3. Gradual rollout with feature flags

## Success Metrics

- Zero memory leaks in valgrind
- Reduced manual delete calls by 90%
- All factory functions return smart pointers
- Clear ownership for all dynamic allocations
- No raw new/delete in application code

## Timeline

- Week 1-2: Factory methods and ClassBase
- Week 3-4: Registry and core infrastructure  
- Week 5-6: Language system components
- Week 7-8: Network layer and finalization

## Notes

- Start with leaf classes to minimize disruption
- Update tests alongside implementation
- Document ownership patterns clearly
- Consider performance implications for hot paths