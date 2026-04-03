# KAI Project Style Guide

This document defines the coding standards and style guidelines for the KAI project. Following these guidelines will help maintain consistency across the codebase and make it easier for developers to read, understand, and contribute to the project.

## Table of Contents

1. [Naming Conventions](#naming-conventions)
2. [Formatting and Braces](#formatting-and-braces)
3. [Comments and Documentation](#comments-and-documentation)
4. [Error Handling](#error-handling)
5. [Modern C++ Features](#modern-c-features)
6. [Project-Specific Patterns](#project-specific-patterns)

## Naming Conventions

### General Principles

- Names should be descriptive and convey meaning
- Avoid abbreviations unless they're widely understood
- Maintain consistency with existing patterns in the codebase

### Classes and Types

- Use `PascalCase` for class names, type names, and type aliases
- Class names should be nouns or noun phrases

```cpp
// GOOD
class StorageBase;
class Object;
class BinaryStream;
using HandleSet = std::set<Handle>;

// BAD
class storageBase;
class object_class;
```

### Variables

- Use `camelCase` for variable names
- Private member variables should use trailing underscore: `memberVariable_`
- Protected/public member variables should use camelCase without underscore

```cpp
// GOOD
int localVariable;
Dictionary dictionary;
Registry* registry;

// Member variables
class Example {
private:
    int privateValue_;
    std::string name_;
public:
    bool isEnabled;
};

// BAD
int LocalVariable;
Dictionary Dictionary;
registry* Registry;
```

### Functions and Methods

- Use `PascalCase` for public methods (to match the KAI codebase convention)
- Methods that return bool should use `Is` or `Has` prefix when appropriate

```cpp
// GOOD
void SetValue(int value);
bool IsValid() const;
bool HasChild(const Label& label) const;

// BAD
void set_value(int value);
bool valid() const;
```

### Constants and Enums

- Use `PascalCase` for enum names and enum values
- Use `ALL_CAPS` for macro definitions

```cpp
// GOOD
enum class Color { Red, Green, Blue };
#define KAI_THROW_0(x) throw Exception::x()

// BAD
enum class color { red, GREEN, blue };
#define kaiThrow0(x) throw Exception::x()
```

### Namespaces

- Use `PascalCase` for namespace names
- Use the `KAI_BEGIN` and `KAI_END` macros for the main KAI namespace

```cpp
// GOOD
KAI_BEGIN
// code here
KAI_END

namespace Network {
    // Networking code
}

// BAD
namespace network {
    // Networking code
}
```

## Formatting and Braces

### Indentation and Spacing

- Use 4 spaces for indentation (not tabs)
- No trailing whitespace at the end of lines
- Use a single space after keywords like `if`, `for`, `while`
- No space between function name and opening parenthesis
- Use a space after commas in argument lists

```cpp
// GOOD
if (condition) {
    DoSomething();
}

void Function(int arg1, int arg2);

// BAD
if(condition){
    DoSomething();
}

void Function ( int arg1,int arg2 );
```

### Braces

- Use K&R style braces with the opening brace on the same line as the statement
- Always use braces for control structures, even for single-line bodies
- Empty functions can have braces on the same line

```cpp
// GOOD
if (condition) {
    DoSomething();
} else {
    DoSomethingElse();
}

void EmptyFunction() {}

// BAD
if (condition)
    DoSomething();

if (condition)
{
    DoSomething();
}
```

### Line Length and Wrapping

- Aim for a maximum line length of 80 characters
- When wrapping lines, indent continuation lines by 4 spaces
- Break lines after operators, not before them

```cpp
// GOOD
SomeLongFunctionName(argument1, argument2,
    argument3, argument4);

if (veryLongConditionThatNeedsToBeWrapped &&
    anotherPartOfTheCondition) {
    // Code
}

// BAD
SomeLongFunctionName(argument1, argument2
    , argument3, argument4);

if (veryLongConditionThatNeedsToBeWrapped
    && anotherPartOfTheCondition) {
    // Code
}
```

## Comments and Documentation

### General Principles

- Write code that is self-documenting when possible
- Comments should explain "why", not "what" (the code shows what)
- Keep comments up-to-date with code changes
- Use complete sentences with proper punctuation for comments

### File Headers

- Each source file should begin with a `#pragma once` (for headers) 
- No copyright banners or license information in individual files (in LICENSE file)

```cpp
#pragma once

#include <KAI/Core/Base.h>
// Rest of includes
```

### Function Documentation

- Use descriptive comments for non-obvious functions
- Document parameters, return values, and exceptions where needed
- For complex functions, describe the algorithm or purpose

```cpp
// Retrieves an object from storage and ensures it exists
// Throws NullObject if the object handle is invalid
StorageBase& GetStorageBase(Object const& object);
```

### Class Documentation

- Document the purpose of the class and any design considerations
- Document any special patterns or usage requirements

```cpp
// A Value<> has direct access to the storage of an object.
// This means that accessing the value is faster as there is no
// lookup required. However, it is unsafe to use a Value over multiple
// frames, as the storage could be deleted by the garbage collector.
// So it is only really safe to use a Value<> in local scope,
// unless you can be sure that the corresponding Object will not
// be collected.
template <class T>
struct Value : ConstValue<T> {
    // Implementation
};
```

### TODO Comments

- Use `// TODO: description` for code that needs future attention
- Include enough context for another developer to understand the task

```cpp
// TODO: Implement caching to improve performance of repeated lookups
```

## Error Handling

### Exception Usage

- Use the KAI_THROW macros for throwing exceptions
- Use descriptive exception types from the Exception hierarchy
- Check preconditions and invariants consistently

```cpp
// GOOD
if (!Valid())
    KAI_THROW_0(NullObject);

if (stream.GetRegistry() == 0)
    KAI_THROW_1(Base, "NullRegistry");

// BAD
if (!Valid())
    throw "Invalid object";  // Non-specific exception
```

### Return Values

- Use return values to indicate success/failure when appropriate
- Return `bool` for simple success/failure indications
- Return `Object()` for null/empty objects rather than null pointers

```cpp
// GOOD
bool Has(const Label& label) const;
Object Get(const Label& label) const;  // Returns empty Object if not found

// BAD
Object* Get(const Label& label) const;  // Returning raw pointer
```

### Validation

- Validate arguments at the beginning of functions
- Use assertions (in debug builds) for invariant checking
- Use early returns to reduce nesting

```cpp
// GOOD
void SetValue(int value) {
    if (!Valid()) {
        KAI_THROW_0(NullObject);
    }
    
    // Implementation
}

// BAD - deep nesting
void SetValue(int value) {
    if (Valid()) {
        if (value > 0) {
            if (someOtherCondition) {
                // Implementation
            }
        }
    }
}
```

## Modern C++ Features

### General Guidelines

- Use C++11/14/17 features where appropriate
- Prefer standard library containers and algorithms over custom implementations
- Use `nullptr` instead of `0` or `NULL` for pointers
- Use `auto` where it improves readability, but not where it obscures types

### Smart Pointers

- Use `std::shared_ptr` or KAI's `Pointer<T>` for shared ownership
- Use `std::unique_ptr` for exclusive ownership
- Avoid raw pointers for ownership (use for non-owning references)

```cpp
// GOOD
std::unique_ptr<Object> CreateObject();
void ProcessObject(Object* obj);  // Non-owning reference

// BAD
Object* CreateObject();  // Unclear ownership
```

### Lambda Expressions

- Use lambdas for short callbacks or when it improves readability
- Capture only what is needed, prefer capture by value for simple types
- Use meaningful names for lambda parameters

```cpp
// GOOD
std::sort(items.begin(), items.end(), 
    [](const Item& a, const Item& b) { return a.GetValue() < b.GetValue(); });

// BAD
std::sort(items.begin(), items.end(), 
    [&](auto& x, auto& y) { return x.GetValue() < y.GetValue(); });  // Captures too much
```

### Range-based For Loops

- Prefer range-based for loops over index-based when possible
- Use `const auto&` for non-modifying access to elements
- Use `auto&` when modifying elements

```cpp
// GOOD
for (const auto& item : collection) {
    Process(item);
}

for (auto& item : collection) {
    item.Update();
}

// BAD
for (size_t i = 0; i < collection.size(); ++i) {
    Process(collection[i]);
}
```

## Project-Specific Patterns

### KAI Namespace

- Use the `KAI_BEGIN` and `KAI_END` macros to wrap code in the KAI namespace
- Use `KAI_NAMESPACE()` macro when referring to elements in the KAI namespace from inside the namespace

```cpp
KAI_BEGIN

void Function() {
    // Call another function in the KAI namespace
    KAI_NAMESPACE(OtherFunction)();
}

KAI_END
```

### Object Handling

- Use `Object` class for type-erased objects
- Use `Value<T>` or `ConstValue<T>` for type-safe direct access
- Check object validity with `Exists()` or `Valid()` methods before use

```cpp
// GOOD
if (object.Exists()) {
    Value<int> value(object);
    *value = 42;
}

// BAD
Value<int> value(object);  // Not checking validity
*value = 42;  // May throw
```

### Memory Management

- Objects should be managed by the Registry
- Use `SetManaged(true)` for objects that should be garbage collected
- Avoid manual memory management when possible

```cpp
// GOOD
Object object = registry->New<SomeType>();
object.SetManaged(true);

// BAD
SomeType* obj = new SomeType();  // Manual memory management
```

### Type System

- Use the KAI type system for reflectable types
- Register types with the Registry
- Use `KAI_TYPE_TRAITS` macro to declare type traits

```cpp
// GOOD
KAI_TYPE_TRAITS(MyType, Number::MyType, 
                Properties::StringStreamInsert | Properties::BinaryStreamInsert);

// Registration
void MyType::Register(Registry& registry) {
    ClassBuilder<MyType>(registry, "MyType")
        .Method("Method1", &MyType::Method1)
        .Property("Property1", &MyType::GetProperty1, &MyType::SetProperty1);
}
```

## Examples

### Complete Class Example

```cpp
#pragma once

#include <KAI/Core/Base.h>

KAI_BEGIN

// A simple data container class with managed storage
class DataContainer {
private:
    std::vector<int> values_;
    std::string name_;
    bool isValid_;

public:
    DataContainer() : isValid_(false) {}
    
    explicit DataContainer(const std::string& name) 
        : name_(name), isValid_(true) {}
    
    void AddValue(int value) {
        if (!isValid_) {
            KAI_THROW_0(InvalidOperation);
        }
        values_.push_back(value);
    }
    
    bool IsValid() const { return isValid_; }
    
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }
    
    std::vector<int> GetValues() const { return values_; }
    
    static void Register(Registry& registry) {
        ClassBuilder<DataContainer>(registry, "DataContainer")
            .Method("AddValue", &DataContainer::AddValue)
            .Method("IsValid", &DataContainer::IsValid)
            .Property("Name", &DataContainer::GetName, &DataContainer::SetName)
            .Property("Values", &DataContainer::GetValues);
    }
};

KAI_TYPE_TRAITS(DataContainer, Number::DataContainer, 
                Properties::StringStreamInsert | Properties::BinaryStreamInsert);

KAI_END
```

### Function Implementation Example

```cpp
KAI_BEGIN

Object CreateDataContainer(Registry& registry, const std::string& name) {
    if (name.empty()) {
        KAI_THROW_1(InvalidArgument, "Container name cannot be empty");
    }
    
    Object container = registry.New<DataContainer>(name);
    container.SetManaged(true);
    
    return container;
}

void ProcessDataContainers(const std::vector<Object>& containers) {
    for (const auto& container : containers) {
        if (!container.Exists() || !container.IsType<DataContainer>()) {
            continue;
        }
        
        ConstValue<DataContainer> dataContainer(container);
        if (!dataContainer->IsValid()) {
            continue;
        }
        
        // Process the container
        std::cout << "Container: " << dataContainer->GetName() << std::endl;
        for (int value : dataContainer->GetValues()) {
            std::cout << "  Value: " << value << std::endl;
        }
    }
}

KAI_END
```

---

This style guide is a living document and may be updated as the KAI project evolves. Developers should follow these guidelines for all new code and aim to update existing code to conform when making modifications.