# KAI Core Component Analysis

This document provides a detailed analysis of the KAI Core component and outlines key patterns for writing and fixing tests that work reliably with the implementation. It enhances the previous review with more comprehensive patterns and examples.

## 1. Core Object System

The Core Object system in KAI is built around several key abstractions:

- **Object**: The base representation with handle, type, and registry references.
- **Handle**: A unique identifier for objects in the registry.
- **Registry**: Central component that manages object instances, types, and garbage collection.
- **StorageBase**: Underlying storage implementation for all objects with dictionary support.

### Key Behaviors

1. **Garbage Collection**
   - KAI uses a tri-color (white, grey, black) marking algorithm for memory management (defined in Registry.cpp with `#define KAI_USE_TRICOLOR`).
   - Objects not referenced from the Root tree or other reachable objects are collected.
   - Root tree is the primary container for storing objects that must survive GC.
   - Objects without references will be collected during GarbageCollect() operations.
   - The tri-color algorithm marks objects as white (ready for collection), grey (being processed), or black (not to be collected).

2. **Object Lifecycle**
   - Objects are created via Registry::New<T>()
   - Objects must be stored in Root via Root().Set(Label("name"), obj) to prevent GC
   - Objects can be removed from Root via Root().Remove(Label("name"))
   - Objects not referenced from anywhere will be collected
   - StorageBase manages a list of containers in which an object exists

3. **Type Checking**
   - Two methods: IsType<T>() and IsTypeNumber(Type::Number)
   - Both methods are valid, but IsTypeNumber() is more commonly used in the implementation
   - Type numbers are unique identifiers for each type, defined in Type::Traits<T>::Number

## 2. Core Pointer System

The Pointer<T> template provides type-safe access to objects:

- **Pointer<T>**: Type-safe pointer template for accessing objects.
- **Object**: Base class for all KAI objects that Pointer<T> references.

### Key Behaviors

1. **Type Checking**
   - Invalid casts (Pointer<float> ptr = intObj) throw TypeMismatch exceptions
   - Need to handle these with try/catch blocks
   - Pointers provide type safety through the KAI type system
   - Pointer<T> constructor with incorrect type will throw a TypeMismatch exception (line 31 in Pointer.h)

2. **Reference Semantics**
   - Multiple Pointer<T> instances can reference the same object
   - Changes through one pointer are visible through all pointers to the same object
   - Garbage collection only occurs when all pointers to an object are gone AND the object is not in the Root tree
   - Pointer<T> assignment can cause objects to be marked for garbage collection if they were only referenced by that pointer

3. **Null Behavior**
   - Pointer<T>() creates a null pointer (does not exist)
   - Check existence with Pointer<T>.Exists()
   - Null pointer assignments (ptr = nullPtr) may behave differently than expected in different contexts

## 3. Core Registry System

The Registry manages all objects and their lifetime:

- **Registry**: Central manager for object creation, storage, and GC.
- **ClassBase**: Base class for all type-specific Class<T> instances.
- **Tree**: Container for the Root hierarchy of named objects.

### Key Behaviors

1. **Object Creation**
   - Use Registry::New<T>() to create objects
   - Objects are tracked via handles
   - All objects have a handle, type number, and registry reference

2. **Object Storage**
   - Store objects in Root tree using Root().Set(Label("name"), obj)
   - This prevents garbage collection
   - Objects not in the Root tree must be referenced by other reachable objects to survive GC
   - Object::operator= triggers StorageBase::DetermineNewColor() which can cause objects to be marked for garbage collection

3. **Garbage Collection**
   - Registry::GarbageCollect() triggers collection
   - Uses tri-color marking algorithm
   - Unreachable objects are freed
   - The `TriColor()` method in Registry.cpp handles the marking and sweeping phases

## 4. Core Container System

Containers store collections of objects:

- **Array**: Ordered collection of objects.
- **Map**: Key-value collection where both keys and values are objects.

### Key Behaviors

1. **Element Storage**
   - Containers track their elements for GC purposes
   - Elements survive GC as long as the container survives
   - Container methods expose elements (At(), GetValue(), etc.)
   - Removing an element from a container may affect its garbage collection status

2. **Mixed Types**
   - Containers can store objects of different types
   - Type checking must be done when retrieving elements

3. **Container Operations**
   - Array::Erase removes an element, which can cause a reordering of elements
   - Map::Insert replaces values for existing keys
   - These operations trigger StorageBase methods that affect garbage collection

## 5. Property System

The property system provides access to object properties:

- **PropertyBase**: Base class for all property implementations.
- **ClassBase::GetProperty**: Returns a const reference to a property.
- **ClassBase::HasProperty**: Checks if a class has a specific property.

### Key Behaviors

1. **Property Access**
   - Use HasProperty instead of GetProperty to check for property existence
   - GetProperty throws an exception if the property doesn't exist
   - Object::GetPropertyObject() must be used to access properties, not Get()
   - Property access is mediated through the class system

2. **Property Values**
   - Properties can be of any type
   - Property values can be accessed through specific methods
   - Property types are verified through the type system

## Fixed Test Patterns

To write reliable tests that work with the KAI Core implementation, follow these patterns:

### 1. Object Creation and Storage

```cpp
// Create an object
Object intObj = Reg().New<int>(42);

// Store in Root to prevent garbage collection
Root().Set(Label("test_object"), intObj);

// ... test operations ...

// Clean up
Root().Remove(Label("test_object"));
```

### 2. Type Checking

```cpp
// Check type using IsType<T>() template method
ASSERT_TRUE(obj.IsType<int>());

// Or using IsTypeNumber() with Type::Traits<T>::Number (preferred)
ASSERT_TRUE(obj.IsTypeNumber(Type::Traits<int>::Number));
```

### 3. Handle Invalid Casts

```cpp
try {
    // This will throw a TypeMismatch exception
    Pointer<float> invalidPtr = intObj; 
    // We shouldn't get here
    FAIL() << "Expected exception was not thrown";
}
catch (...) {
    // This is the expected behavior
    SUCCEED();
}
```

### 4. Access Object Values

```cpp
// Using Pointer<T>
Pointer<int> ptr = obj;
ASSERT_EQ(*ptr, 42);

// Or using ConstDeref<T>
ASSERT_EQ(ConstDeref<int>(obj), 42);
```

### 5. Property Access

```cpp
// Check if property exists using HasProperty
ASSERT_TRUE(classBase->HasProperty(Label("Size")));

// Access property object via GetPropertyObject
Object sizeProperty = obj.GetPropertyObject(Label("Size"));
ASSERT_TRUE(sizeProperty.Exists());
```

### 6. Container Element Access

```cpp
// Create a container
Pointer<Array> array = Reg().New<Array>();

// Add elements
array->PushBack(Reg().New<int>(1));
array->PushBack(Reg().New<int>(2));

// Access elements
Object element = array->At(0);
ASSERT_TRUE(element.IsType<int>());
ASSERT_EQ(ConstDeref<int>(element), 1);

// When erasing elements, be aware of reordering
array->Erase(array->At(0));
ASSERT_EQ(array->Size(), 1);
// The remaining element may not be in its original position
```

## Common Issues and Solutions

1. **Objects disappearing during GC**
   - ISSUE: Objects are being collected unexpectedly
   - SOLUTION: Store objects in Root tree using Root().Set(Label("name"), obj)

2. **TypeMismatch exceptions during casting**
   - ISSUE: Invalid casts throw exceptions
   - SOLUTION: Use try/catch blocks to handle these exceptions

3. **Null pointer behavior**
   - ISSUE: Assigning nullPtr doesn't always clear pointers as expected
   - SOLUTION: Check existence with Exists() and handle both cases

4. **Container element access**
   - ISSUE: Elements may not exist when accessed directly after GC
   - SOLUTION: Verify the container structure (Size()) rather than direct element existence

5. **Property access methods**
   - ISSUE: GetProperty returns a const reference and can't be compared to nullptr
   - SOLUTION: Use HasProperty instead to check for property existence

## Implementation Issues to Consider

1. **Defensive Programming**: The codebase has extensive defensive checks for invalid pointers, but some areas could benefit from additional validation.

2. **Exception Handling**: Type mismatches during casting throw exceptions, which might not be expected by test code.

3. **Documentation**: The behavior of garbage collection and ownership model isn't well-documented, leading to confusion in test implementation.

4. **API Consistency**: Consider providing more consistent API naming and behavior, especially around property access and type checking.

## Conclusion

Understanding the KAI Core component's implementation details is crucial for writing reliable tests. The patterns outlined in this document provide a framework for creating tests that accurately assess the system's behavior while accommodating its specific implementation choices.

Remember to:
1. Store objects in Root tree to prevent garbage collection
2. Handle TypeMismatch exceptions with try/catch blocks
3. Verify object existence and type appropriately
4. Clean up after tests by removing objects from Root
5. Use HasProperty instead of GetProperty for property checking
6. Be aware of container reordering during element erasure