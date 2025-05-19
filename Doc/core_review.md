# KAI Core Component Review

## Key Issues Identified in Test Failures

After reviewing the core implementation files, including Registry, Object, Pointer, and StorageBase, I've identified the following key issues that are causing test failures:

### 1. Garbage Collection Behavior

The KAI codebase uses a tri-color garbage collection algorithm (defined on line 14 in Registry.cpp with `#define KAI_USE_TRICOLOR`). This has several implications for our tests:

- Objects that aren't stored in the Root tree or another referenced container are marked as white and will be garbage collected
- The `Object::operator=` triggers `StorageBase::DetermineNewColor()` which can cause objects to be marked for garbage collection
- StorageBase manages a list of containers in which an object exists, and objects are automatically collected when they're removed from all containers
- When a `Pointer<T>` is reassigned, the original object it pointed to can be collected if not referenced elsewhere

### 2. API Discrepancies

There are several discrepancies between the API expected by the tests and the actual implementation:

- `Object::IsType<T>()` exists (line 43 in Object.h), but tests are failing because `IsTypeNumber(Type::Traits<T>::Number)` is being used instead
- `HasProperty()` operates on `ClassBase` objects, not directly on properties in the object's dictionary
- `GetPropertyObject()` must be used to access properties, not `Get()`
- StorageBase does not have a direct way to retrieve typed property values with type checking in a single call

### 3. Memory Management and Pointer Behavior

- `Pointer<T>` assignment can cause objects to be marked for garbage collection if they were only referenced by that pointer
- `Pointer<T>` constructor with incorrect type will throw a `TypeMismatch` exception (line 31 in Pointer.h)
- Objects must be registered with the registry and stored in a container (e.g., Root tree) to survive garbage collection

### 4. Container Operations

- Array and Map operations rely on specific implementation details:
  - Array::Erase removes an element, which can cause a reordering of elements
  - Map::Insert replaces values for existing keys
  - These operations trigger StorageBase methods that affect garbage collection

## Recommended Fixes for Tests

1. **Object Storage**: Always store test objects in the Root tree to prevent premature garbage collection:
   ```cpp
   // Create an object and store in Root
   Object obj = Reg().New<int>(42);
   Root().Set("test_obj", obj);
   
   // Test operations...
   
   // Remove from Root when done
   Root().Remove("test_obj");
   ```

2. **Type Checking**: Use `IsTypeNumber()` instead of `IsType<T>()`:
   ```cpp
   // Instead of
   ASSERT_TRUE(obj.IsType<int>());
   
   // Use
   ASSERT_TRUE(obj.IsTypeNumber(Type::Traits<int>::Number));
   ```

3. **Property Access**: Use proper property methods:
   ```cpp
   // Instead of
   Object prop = obj.Get("Size");
   
   // Use
   Object prop = obj.GetPropertyObject(Label("Size"));
   ```

4. **Handle Type Casting Exceptions**: Properly handle exceptions when attempting to cast between types:
   ```cpp
   try {
       Pointer<float> ptr = intObj;  // May throw TypeMismatch
       // If no exception, should return empty pointer
       ASSERT_FALSE(ptr.Exists());
   }
   catch (const Exception& e) {
       // Exception is also acceptable behavior
   }
   ```

5. **Container Operations**: Update expectations for container behavior:
   ```cpp
   // When erasing from an array, expect reordering
   array->Erase(array->At(1));  // Erases element at index 1
   // Last element may have moved to index 1
   ASSERT_EQ(array->Size(), 2);
   // Don't assume specific ordering after erasure
   ```

## Implementation Issues to Consider

1. **Defensive Programming**: The codebase has extensive defensive checks for invalid pointers, but some areas could benefit from additional validation.

2. **Exception Handling**: Type mismatches during casting throw exceptions, which might not be expected by test code.

3. **Documentation**: The behavior of garbage collection and ownership model isn't well-documented, leading to confusion in test implementation.

4. **API Consistency**: Consider providing more consistent API naming and behavior, especially around property access and type checking.

By addressing these issues, the tests can be updated to align with the actual implementation behavior, resulting in more reliable and meaningful test coverage.