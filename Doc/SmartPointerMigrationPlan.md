# Smart Pointer Migration Plan for KAI Codebase

## Overview
This document outlines the systematic approach to replace raw pointers with smart pointers throughout the KAI codebase to improve memory safety and prevent leaks.

## Priority Order

### 1. ClassBuilder Memory Leak Fix (HIGHEST PRIORITY)
**Issue**: ClassBuilder allocates a `Class<T>*` in its constructor but never deletes it, causing a memory leak.
**Solution**: Use `std::unique_ptr<Class<T>>` for single ownership.

### 2. Registry Class Management
**Issue**: Registry stores raw pointers to ClassBase objects in a vector.
**Solution**: Use `std::shared_ptr<const ClassBase>` for shared ownership across the system.

### 3. Storage and Object Lifecycle
**Issue**: StorageBase pointers are manually managed throughout the system.
**Solution**: 
- Use `std::shared_ptr<StorageBase>` for instances in Registry
- Use `std::weak_ptr<StorageBase>` for back-references to prevent cycles

### 4. Property and Method Management
**Issue**: PropertyBase and MethodBase pointers are manually deleted in ClassBase destructor.
**Solution**: Use `std::unique_ptr` in the property/method maps.

### 5. Network Layer
**Issue**: Network components use raw pointers for connections and nodes.
**Solution**: Implement RAII wrappers with appropriate smart pointers.

## Ownership Patterns

### unique_ptr Usage
- **Single ownership**: Factory return values, internal resources
- **Examples**:
  - ClassBuilder's internal Class<T> instance
  - Property and Method objects in ClassBase
  - Temporary objects during operations

### shared_ptr Usage
- **Shared ownership**: Registry-managed objects
- **Examples**:
  - ClassBase instances in Registry
  - StorageBase instances for active objects
  - Network nodes and connections

### weak_ptr Usage
- **Breaking cycles**: Parent-child relationships
- **Examples**:
  - Child objects referencing parent containers
  - Observer patterns in the event system
  - Network peer references

### Raw Pointer Usage (Limited)
- **Non-owning references**: Function parameters, temporary access
- **Examples**:
  - Method parameters that don't transfer ownership
  - Internal pointers within algorithms
  - Performance-critical paths (after profiling)

## Implementation Strategy

### Phase 1: Core Memory Management (Week 1-2)
1. Update ClassBuilder with unique_ptr
2. Convert Registry's class management to shared_ptr
3. Update factory methods to return smart pointers
4. Add comprehensive unit tests

### Phase 2: Object Lifecycle (Week 3-4)
1. Convert StorageBase management to shared_ptr
2. Update Object class to use smart pointers internally
3. Fix circular reference issues with weak_ptr
4. Update garbage collection to work with smart pointers

### Phase 3: Properties and Methods (Week 5)
1. Convert PropertyBase storage to unique_ptr
2. Convert MethodBase storage to unique_ptr
3. Update all derived classes
4. Ensure proper move semantics

### Phase 4: Network Layer (Week 6-7)
1. Create RAII wrappers for network resources
2. Convert Connection management to shared_ptr
3. Update Node ownership model
4. Test distributed scenarios

### Phase 5: Testing and Documentation (Week 8)
1. Comprehensive integration testing
2. Performance benchmarking
3. Update documentation
4. Create migration guide for users

## Backward Compatibility

### API Preservation
- Maintain existing public APIs where possible
- Add overloads that accept smart pointers
- Deprecate raw pointer interfaces gradually

### Migration Path
1. Add smart pointer overloads alongside existing APIs
2. Mark raw pointer methods as deprecated
3. Provide migration tools/scripts
4. Remove deprecated APIs in next major version

## Technical Considerations

### Performance
- Use make_shared/make_unique for efficiency
- Avoid unnecessary reference counting in hot paths
- Profile critical sections before/after migration

### Thread Safety
- shared_ptr reference counting is thread-safe
- Object access still requires synchronization
- Document thread safety guarantees

### Exception Safety
- Smart pointers provide automatic cleanup
- Use RAII consistently throughout
- Ensure strong exception guarantee where needed

## Success Metrics
- Zero memory leaks in valgrind/sanitizers
- No decrease in performance benchmarks
- All unit tests passing
- Clean static analysis results

## Risk Mitigation
- Incremental migration with thorough testing
- Feature flags for rollback capability
- Extensive code review process
- Automated testing at each phase