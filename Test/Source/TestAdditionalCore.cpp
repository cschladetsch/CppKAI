#include <gtest/gtest.h>
#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Debug.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Pathname.h>
#include <KAI/Core/Object.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/Type.h>
#include <KAI/Core/Value.h>
#include <KAI/Core/StringStream.h>
#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/Map.h>
#include <KAI/Core/BuiltinTypes/Dictionary.h>
#include <KAI/Core/BuiltinTypes/List.h>
#include <KAI/Core/BuiltinTypes/Set.h>
#include <KAI/Core/BuiltinTypes/Stack.h>
#include <cstdlib>
#include <string>
#include <iostream>

using namespace kai;

// A simple class for testing more complex type registration
class TestPerson
{
private:
    String name;
    int age;
    bool employed;

public:
    TestPerson() : name("Unknown"), age(0), employed(false) {}
    TestPerson(const String& n, int a, bool e) : name(n), age(a), employed(e) {}

    const String& GetName() const { return name; }
    void SetName(const String& n) { name = n; }

    int GetAge() const { return age; }
    void SetAge(int a) { age = a; }

    bool IsEmployed() const { return employed; }
    void SetEmployed(bool e) { employed = e; }

    String ToString() const 
    { 
        StringStream ss;
        ss << "Person(\"" << name << "\", " << age << ", " << (employed ? "employed" : "unemployed") << ")";
        return ss.ToString(); 
    }

    bool operator==(const TestPerson& other) const 
    {
        return name == other.name && age == other.age && employed == other.employed;
    }
};

// This registers our TestPerson class with the KAI system
void RegisterTestPerson(Registry& registry) 
{
    ClassBuilder<TestPerson>(registry, "TestPerson")
        .Constructor<const String&, int, bool>()
        .Methods
            ("ToString", &TestPerson::ToString)
            ("GetName", &TestPerson::GetName)
            ("SetName", &TestPerson::SetName)
            ("GetAge", &TestPerson::GetAge)
            ("SetAge", &TestPerson::SetAge)
            ("IsEmployed", &TestPerson::IsEmployed)
            ("SetEmployed", &TestPerson::SetEmployed);
}

// 1. Test Registry Class Registration
TEST(TestAdditionalCore, RegistryClassRegistration) 
{
    Registry registry;
    
    // Register TestPerson
    RegisterTestPerson(registry);
    
    // Verify registration
    ASSERT_TRUE(registry.Exists<TestPerson>());
    
    // Get type info
    TypeInfo const* typeInfo = registry.GetTypeInfo<TestPerson>();
    ASSERT_NE(typeInfo, nullptr);
    ASSERT_EQ(typeInfo->GetLabel().ToString(), "TestPerson");
}

// 2. Test Object Creation through Registry
TEST(TestAdditionalCore, ObjectCreation) 
{
    Registry registry;
    RegisterTestPerson(registry);
    
    // Create object using registry
    Object obj = registry.New<TestPerson>("Jane", 30, true);
    ASSERT_TRUE(obj.Valid());
    
    // Verify object properties
    TestPerson& person = obj.GetObject<TestPerson>();
    ASSERT_EQ(person.GetName(), "Jane");
    ASSERT_EQ(person.GetAge(), 30);
    ASSERT_TRUE(person.IsEmployed());
}

// 3. Test Object Method Invocation
TEST(TestAdditionalCore, ObjectMethodInvocation) 
{
    Registry registry;
    RegisterTestPerson(registry);
    
    // Create and modify object
    Object obj = registry.New<TestPerson>("John", 25, false);
    
    // Get reference to actual person object
    TestPerson& person = obj.GetObject<TestPerson>();
    
    // Test original values
    ASSERT_EQ(person.GetName(), "John");
    ASSERT_EQ(person.GetAge(), 25);
    ASSERT_FALSE(person.IsEmployed());
    
    // Change values through methods
    person.SetName("John Smith");
    person.SetAge(26);
    person.SetEmployed(true);
    
    // Verify changes
    ASSERT_EQ(person.GetName(), "John Smith");
    ASSERT_EQ(person.GetAge(), 26);
    ASSERT_TRUE(person.IsEmployed());
    
    // Test ToString
    String str = person.ToString();
    ASSERT_FALSE(str.Empty());
    ASSERT_TRUE(str.IndexOf("John Smith") != -1);
}

// 4. Test Value Construction and Assignment
TEST(TestAdditionalCore, ValueConstruction) 
{
    // Test simple type values
    Value<int> intVal(42);
    ASSERT_EQ(intVal.GetValue(), 42);
    
    Value<float> floatVal(3.14f);
    ASSERT_FLOAT_EQ(floatVal.GetValue(), 3.14f);
    
    Value<String> strVal("Test String");
    ASSERT_EQ(strVal.GetValue(), "Test String");
    
    // Test assignment
    intVal = 100;
    ASSERT_EQ(intVal.GetValue(), 100);
    
    floatVal = 2.718f;
    ASSERT_FLOAT_EQ(floatVal.GetValue(), 2.718f);
    
    strVal = "New String";
    ASSERT_EQ(strVal.GetValue(), "New String");
}

// 5. Test Object Serialization
TEST(TestAdditionalCore, ObjectSerialization) 
{
    Registry registry;
    RegisterTestPerson(registry);
    
    // Create test object
    Object obj = registry.New<TestPerson>("Alice", 28, true);
    
    // Serialize to string
    StringStream ss;
    ss << obj;
    String serialized = ss.ToString();
    
    // Verify it contains expected content
    ASSERT_FALSE(serialized.Empty());
}

// 6. Test Registry Type Lookup
TEST(TestAdditionalCore, RegistryTypeLookup) 
{
    Registry registry;
    
    // Get various type info objects
    const TypeInfo* intType = registry.GetTypeInfo("Signed32");
    ASSERT_NE(intType, nullptr);
    ASSERT_EQ(intType->GetLabel().ToString(), "Signed32");
    
    const TypeInfo* floatType = registry.GetTypeInfo("Single");
    ASSERT_NE(floatType, nullptr);
    ASSERT_EQ(floatType->GetLabel().ToString(), "Single");
    
    const TypeInfo* stringType = registry.GetTypeInfo("String");
    ASSERT_NE(stringType, nullptr);
    ASSERT_EQ(stringType->GetLabel().ToString(), "String");
    
    // Try a non-existent type
    const TypeInfo* nonExistentType = registry.GetTypeInfo("NonExistentType");
    ASSERT_EQ(nonExistentType, nullptr);
}

// 7. Test Path Operations
TEST(TestAdditionalCore, PathnameOperations) 
{
    // Test pathname parsing and components
    Pathname path("/foo/bar/baz.txt");
    
    ASSERT_EQ(path.GetFullname(), "/foo/bar/baz.txt");
    ASSERT_EQ(path.GetExtension(), ".txt");
    ASSERT_EQ(path.GetStem(), "baz");
    ASSERT_EQ(path.GetDirectory(), "/foo/bar");
    ASSERT_TRUE(path.IsAbsolute());
    
    // Test manipulating pathnames
    Pathname path2 = path.GetParent();
    ASSERT_EQ(path2.GetFullname(), "/foo/bar");
    
    // Test combining paths
    Pathname path3 = path2 / "qux.dat";
    ASSERT_EQ(path3.GetFullname(), "/foo/bar/qux.dat");
    
    // Test relative paths
    Pathname relPath("rel/path/file.ext");
    ASSERT_FALSE(relPath.IsAbsolute());
    ASSERT_EQ(relPath.GetStem(), "file");
    ASSERT_EQ(relPath.GetExtension(), ".ext");
}

// 8. Test String Operations
TEST(TestAdditionalCore, StringOperations) 
{
    // Construction and concatenation
    String str1("Hello");
    String str2(" World");
    String combined = str1 + str2;
    
    ASSERT_EQ(combined, "Hello World");
    ASSERT_EQ(combined.Length(), 11);
    
    // Substrings
    String sub = combined.Substring(6, 5);
    ASSERT_EQ(sub, "World");
    
    // Find and replace
    String text = "The quick brown fox jumps over the lazy dog";
    int pos = text.IndexOf("brown");
    ASSERT_NE(pos, -1);
    ASSERT_EQ(pos, 10);
    
    String replaced = text.Replace("brown", "red");
    ASSERT_EQ(replaced, "The quick red fox jumps over the lazy dog");
    
    // Case conversion
    String lower = combined.ToLower();
    ASSERT_EQ(lower, "hello world");
    
    String upper = combined.ToUpper();
    ASSERT_EQ(upper, "HELLO WORLD");
}

// 9. Test Array Container
TEST(TestAdditionalCore, ArrayAdvanced) 
{
    Registry registry;
    
    // Create and populate array
    Object arrayObj = registry.New<Array>();
    Array& array = arrayObj.GetObject<Array>();
    
    for (int i = 0; i < 10; ++i) {
        array.Push(registry.New(i * 10));
    }
    
    // Test size and access
    ASSERT_EQ(array.Size(), 10);
    ASSERT_EQ(ConstDeref<int>(array.At(3)), 30);
    ASSERT_EQ(ConstDeref<int>(array.At(7)), 70);
    
    // Test insertion and removal
    array.Insert(5, registry.New(55));
    ASSERT_EQ(array.Size(), 11);
    ASSERT_EQ(ConstDeref<int>(array.At(5)), 55);
    
    array.Erase(5);
    ASSERT_EQ(array.Size(), 10);
    ASSERT_EQ(ConstDeref<int>(array.At(5)), 50);
    
    // Clear and check emptiness
    array.Clear();
    ASSERT_TRUE(array.Empty());
    ASSERT_EQ(array.Size(), 0);
}

// 10. Test Map Container
TEST(TestAdditionalCore, MapAdvanced) 
{
    Registry registry;
    
    // Create and populate map
    Object mapObj = registry.New<Map>();
    Map& map = mapObj.GetObject<Map>();
    
    // Add elements
    map.Insert(registry.New("one"), registry.New(1));
    map.Insert(registry.New("two"), registry.New(2));
    map.Insert(registry.New("three"), registry.New(3));
    
    // Test size and contains
    ASSERT_EQ(map.Size(), 3);
    ASSERT_TRUE(map.Contains(registry.New("two")));
    ASSERT_FALSE(map.Contains(registry.New("four")));
    
    // Test retrieval
    ASSERT_EQ(ConstDeref<int>(map.At(registry.New("one"))), 1);
    ASSERT_EQ(ConstDeref<int>(map.At(registry.New("three"))), 3);
    
    // Test removal
    map.Erase(registry.New("two"));
    ASSERT_EQ(map.Size(), 2);
    ASSERT_FALSE(map.Contains(registry.New("two")));
    
    // Clear and check emptiness
    map.Clear();
    ASSERT_TRUE(map.Empty());
    ASSERT_EQ(map.Size(), 0);
}

// 11. Test Exception Handling
TEST(TestAdditionalCore, ExceptionHandling) 
{
    // Test basic exception construction and properties
    try {
        throw Exception::AssertionFailed(LOCATION, "Intentionally throwing exception for testing");
    }
    catch (Exception::AssertionFailed const& e) {
        ASSERT_TRUE(std::string(e.what()).find("Intentionally") != std::string::npos);
    }
    
    // Test file not found exception
    try {
        throw Exception::FileNotFound("nonexistent.txt");
    }
    catch (Exception::FileNotFound const& e) {
        ASSERT_TRUE(std::string(e.what()).find("nonexistent.txt") != std::string::npos);
    }
    catch (...) {
        ASSERT_TRUE(false && "Wrong exception type caught");
    }
}

// 12. Test StringStream
TEST(TestAdditionalCore, StringStreamAdvanced) 
{
    Registry registry;
    
    // Create objects to stream
    Object intObj = registry.New(42);
    Object floatObj = registry.New(3.14f);
    Object strObj = registry.New<String>("test string");
    
    // Test streaming objects
    StringStream ss;
    ss << intObj << " " << floatObj << " " << strObj;
    String result = ss.ToString();
    
    ASSERT_FALSE(result.Empty());
    ASSERT_TRUE(result.IndexOf("42") != -1);
    ASSERT_TRUE(result.IndexOf("3.14") != -1);
    ASSERT_TRUE(result.IndexOf("test string") != -1);
    
    // Test formatting
    StringStream ss2;
    ss2.SetFormatted(true);
    ss2 << intObj << '\n' << floatObj << '\n' << strObj;
    String formatted = ss2.ToString();
    
    ASSERT_TRUE(formatted.IndexOf('\n') != -1);
}

// 13. Test Dictionary Container
TEST(TestAdditionalCore, DictionaryAdvanced) 
{
    Registry registry;
    
    // Create dictionary
    Object dictObj = registry.New<Dictionary>();
    Dictionary& dict = dictObj.GetObject<Dictionary>();
    
    // Add elements
    dict.Insert("one", registry.New(1));
    dict.Insert("two", registry.New(2));
    dict.Insert("three", registry.New(3));
    
    // Test size and access
    ASSERT_EQ(dict.Size(), 3);
    ASSERT_TRUE(dict.Exists("two"));
    ASSERT_FALSE(dict.Exists("four"));
    
    Object oneVal = dict.Get("one");
    ASSERT_TRUE(oneVal.Valid());
    ASSERT_EQ(ConstDeref<int>(oneVal), 1);
    
    // Test removal
    dict.Erase("two");
    ASSERT_EQ(dict.Size(), 2);
    ASSERT_FALSE(dict.Exists("two"));
}

// 14. Test List Container
TEST(TestAdditionalCore, ListAdvanced) 
{
    Registry registry;
    
    // Create list
    Object listObj = registry.New<List>();
    List& list = listObj.GetObject<List>();
    
    // Add elements
    list.PushBack(registry.New(10));
    list.PushBack(registry.New(20));
    list.PushBack(registry.New(30));
    list.PushFront(registry.New(5));
    
    // Test size and access
    ASSERT_EQ(list.Size(), 4);
    ASSERT_EQ(ConstDeref<int>(list.Front()), 5);
    ASSERT_EQ(ConstDeref<int>(list.Back()), 30);
    
    // Test removal
    list.PopFront();
    ASSERT_EQ(list.Size(), 3);
    ASSERT_EQ(ConstDeref<int>(list.Front()), 10);
    
    list.PopBack();
    ASSERT_EQ(list.Size(), 2);
    ASSERT_EQ(ConstDeref<int>(list.Back()), 20);
}

// 15. Test Debug Tracing
TEST(TestAdditionalCore, DebugTracing) 
{
    // Save current trace level
    int oldLevel = debug::GetTraceLevel();
    
    // Set a specific trace level
    debug::SetTraceLevel(3);
    ASSERT_EQ(debug::GetTraceLevel(), 3);
    
    // Test trace enabled/disabled for various levels
    ASSERT_TRUE(debug::TraceEnabled(2));
    ASSERT_TRUE(debug::TraceEnabled(3));
    ASSERT_FALSE(debug::TraceEnabled(4));
    
    // Restore previous level
    debug::SetTraceLevel(oldLevel);
}

// 16. Test Set Container
TEST(TestAdditionalCore, SetAdvanced) 
{
    Registry registry;
    
    // Create set
    Object setObj = registry.New<Set>();
    Set& set = setObj.GetObject<Set>();
    
    // Add elements
    set.Insert(registry.New(10));
    set.Insert(registry.New(20));
    set.Insert(registry.New(30));
    
    // Test size and contains
    ASSERT_EQ(set.Size(), 3);
    ASSERT_TRUE(set.Contains(registry.New(20)));
    ASSERT_FALSE(set.Contains(registry.New(40)));
    
    // Test removal
    set.Erase(registry.New(20));
    ASSERT_EQ(set.Size(), 2);
    ASSERT_FALSE(set.Contains(registry.New(20)));
    
    // Test clearing
    set.Clear();
    ASSERT_TRUE(set.Empty());
}

// 17. Test Registry Creation Factory
TEST(TestAdditionalCore, RegistryCreationFactory) 
{
    Registry registry;
    
    // Create standard types
    Object intObj = registry.New(42);
    Object floatObj = registry.New(3.14f);
    Object strObj = registry.New<String>("test");
    
    ASSERT_TRUE(intObj.Valid());
    ASSERT_TRUE(floatObj.Valid());
    ASSERT_TRUE(strObj.Valid());
    
    ASSERT_EQ(ConstDeref<int>(intObj), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(floatObj), 3.14f);
    ASSERT_EQ(ConstDeref<String>(strObj), "test");
    
    // Create container types
    Object arrayObj = registry.New<Array>();
    Object mapObj = registry.New<Map>();
    Object listObj = registry.New<List>();
    
    ASSERT_TRUE(arrayObj.Valid());
    ASSERT_TRUE(mapObj.Valid());
    ASSERT_TRUE(listObj.Valid());
}

// 18. Test Object Ownership and Reference Counting
TEST(TestAdditionalCore, ObjectOwnership) 
{
    Registry registry;
    RegisterTestPerson(registry);
    
    // Create parent object
    Object parent = registry.New<Array>();
    Array& array = parent.GetObject<Array>();
    
    // Create and add child objects
    Object child1 = registry.New<TestPerson>("Child1", 5, false);
    Object child2 = registry.New<TestPerson>("Child2", 7, false);
    
    array.Push(child1);
    array.Push(child2);
    
    // Test child access through parent
    ASSERT_EQ(array.Size(), 2);
    Object retrievedChild1 = array.At(0);
    ASSERT_TRUE(retrievedChild1.Valid());
    ASSERT_EQ(retrievedChild1.GetPointer().GetValue(), child1.GetPointer().GetValue());
    
    // Test removing objects
    array.Erase(0);
    ASSERT_EQ(array.Size(), 1);
    
    // Original child1 should still be valid as we have a reference
    ASSERT_TRUE(child1.Valid());
}

// 19. Test Type System Hierarchies
TEST(TestAdditionalCore, TypeSystemHierarchies) 
{
    Registry registry;
    
    // Get type info for various types
    const TypeInfo* intType = registry.GetTypeInfo<int>();
    const TypeInfo* floatType = registry.GetTypeInfo<float>();
    const TypeInfo* strType = registry.GetTypeInfo<String>();
    const TypeInfo* arrayType = registry.GetTypeInfo<Array>();
    
    ASSERT_NE(intType, nullptr);
    ASSERT_NE(floatType, nullptr);
    ASSERT_NE(strType, nullptr);
    ASSERT_NE(arrayType, nullptr);
    
    // Test type names
    ASSERT_EQ(intType->GetLabel().ToString(), "Signed32");
    ASSERT_EQ(floatType->GetLabel().ToString(), "Single");
    ASSERT_EQ(strType->GetLabel().ToString(), "String");
    ASSERT_EQ(arrayType->GetLabel().ToString(), "Array");
}

// 20. Test Stack Container Operations
TEST(TestAdditionalCore, StackOperations) 
{
    Registry registry;
    
    // Create stack
    Object stackObj = registry.New<kai::Stack>();
    kai::Stack& stack = stackObj.GetObject<kai::Stack>();
    
    // Test initially empty
    ASSERT_TRUE(stack.Empty());
    
    // Push elements
    stack.Push(registry.New(10));
    stack.Push(registry.New(20));
    stack.Push(registry.New(30));
    
    // Test size
    ASSERT_EQ(stack.Size(), 3);
    ASSERT_FALSE(stack.Empty());
    
    // Test top access
    ASSERT_EQ(ConstDeref<int>(stack.Top()), 30);
    
    // Test popping
    stack.Pop();
    ASSERT_EQ(stack.Size(), 2);
    ASSERT_EQ(ConstDeref<int>(stack.Top()), 20);
    
    stack.Pop();
    ASSERT_EQ(stack.Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack.Top()), 10);
    
    stack.Pop();
    ASSERT_TRUE(stack.Empty());
}