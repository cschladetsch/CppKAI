#include "KAI/Core/StringStream.h"
#include "TestCommon.h"

USING_NAMESPACE_KAI

// Test fixture specifically for Map tests
struct TestMap : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        Reg().AddClass<String>();
        Reg().AddClass<Map>();
        Reg().AddClass<StringStream>();
        Reg().AddClass<BinaryStream>();
    }
};

// ========== Tests from TestContainer/TestMap.cpp ==========

TEST_F(TestMap, TestCreation) {
    Pointer<Map> map = Reg().New<Map>();
    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(map->Size() == 0);
    ASSERT_TRUE(map->Empty());

    Reg().GarbageCollect();

    ASSERT_FALSE(map.Exists());
}

TEST_F(TestMap, TestInsertDelete) {
    Pointer<Map> map = Reg().New<Map>();
    Pointer<Map> dangling = Reg().New<Map>();

    // add the map to the root_ of the object tree_, so it can be
    // found and hence not GC'd
    root_.Set("map", map);
    Reg().GarbageCollect();

    // the map will still exist after the GC - but the `dangling` map
    // won't because it can't be reached by the Registry reg
    ASSERT_TRUE(map.Exists());
    ASSERT_FALSE(dangling.Exists());

    // make a key and a value to insert into map
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    map->Insert(n, s);

    ASSERT_TRUE(map->ContainsKey(n));
    Object found = map->GetValue(n);

    ASSERT_STREQ(ConstDeref<String>(found).c_str(), "Hello");
    ASSERT_EQ(found.GetHandle(), s.GetHandle());

    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(n.Exists());
    ASSERT_TRUE(found.Exists());

    // by removing the key associated with n, we also remove the value
    map->Erase(n);
    Reg().GarbageCollect();

    // now, neither the key nor value should exist,
    // but the map itself should exist because it was added
    // to the root_ of the tree_ above
    ASSERT_TRUE(map.Exists());
    ASSERT_FALSE(n.Exists());
    ASSERT_FALSE(found.Exists());
}

TEST_F(TestMap, TestComparison) {
    Pointer<Map> m0 = Reg().New<Map>();
    Pointer<Map> m1 = Reg().New<Map>();

    Object n = Reg().New(42);
    Pointer<String> s0 = Reg().New<String>("World");
    Pointer<String> s1 = Reg().New<String>("World");

    // make two value-identical maps (but with different value objects)
    m0->Insert(n, s0);
    m1->Insert(n, s1);

    // get a reference to the class for maps to make things easier to type
    ClassBase const &k = *m0.GetClass();

    // test that both maps are the same size and have same set of key/value
    // pairs (by object value)
    ASSERT_TRUE(k.Equiv(m0, m1));

    // change the value that the second map has for it's key valued 42
    *s1 = "Hello";
    ASSERT_FALSE(k.Equiv(m0, m1));

    // and just to prove that wasn't a fluke:
    *s1 = "World";
    ASSERT_TRUE(k.Equiv(m0, m1));
}

TEST_F(TestMap, TestStringStream) {
    Pointer<Map> m = Reg().New<Map>();
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    m->Insert(n, s);

    // make a new string stream, insert the map into it
    Pointer<StringStream> t = Reg().New<StringStream>();
    *t << m;

    std::cout << t->ToString().c_str() << std::endl;

    // here, we extract a map back out of the stream.
    //
    // not happy about this approach. we have to make the map with a Reg().stry,
    // before extracting it from a StringStream. It seems we should be able to
    // just say:
    //        Object q;
    //        *t >> q;
    //        ASSERT_TRUE(q.GetClass().GetType() == Type::Traits<Map>::Number);
    // Pointer<Map> m1 = Reg().New<Map>();
    // *t >> m1;

    // ensure the extracted map is the same value as the original map
    // ASSERT_EQ(*m, *m1);
}

TEST_F(TestMap, TestBinaryStream) {
    Pointer<Map> m0 = Reg().New<Map>();
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    m0->Insert(n, s);

    // make a new string stream, insert the map into it
    Pointer<BinaryStream> t = Reg().New<BinaryStream>();
    *t << m0;

    // TODO: BinaryStream  derives from BinaryPacket, which needs a Registry.
    // now, the BinaryStream has one, but there's currently no means to pass
    // that to the underlying BinaryPacket...
    // this means that the following fails, as it attempts to extract from
    // a BinaryPacket with a null Registry.
    //
    // There should be a way to inject custom code on construction. If it
    // doesn't exist in the current_ Traits system, it has to be added.
    //
    // The main functionality is to be able to pass arguments to base class
    // on construction...
    // *t >> m1;

    // ASSERT_EQ(*m0, *m1);
}

// ========== Tests from CoreContainerTests.cpp ==========

// Test Map container functionality
TEST_F(TestMap, TestMapBasicOperations) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Store in root to prevent garbage collection
    Root().Set(Label("test_map"), map);

    // Create keys and values
    Object keyOne = Reg().New<String>("one");
    Object keyTwo = Reg().New<String>("two");
    Object keyThree = Reg().New<String>("three");

    Object valOne = Reg().New<int>(1);
    Object valTwo = Reg().New<int>(2);
    Object valThree = Reg().New<int>(3);

    // Store keys and values in root to prevent garbage collection
    Root().Set(Label("test_key_one"), keyOne);
    Root().Set(Label("test_key_two"), keyTwo);
    Root().Set(Label("test_key_three"), keyThree);

    Root().Set(Label("test_val_one"), valOne);
    Root().Set(Label("test_val_two"), valTwo);
    Root().Set(Label("test_val_three"), valThree);

    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(map->Empty());
    ASSERT_EQ(map->Size(), 0);

    // Add key-value pairs using Insert
    map->Insert(keyOne, valOne);
    map->Insert(keyTwo, valTwo);
    map->Insert(keyThree, valThree);

    // Verify size
    ASSERT_EQ(map->Size(), 3);
    ASSERT_FALSE(map->Empty());

    // Get values by key using GetValue
    Object one = map->GetValue(keyOne);
    Object two = map->GetValue(keyTwo);
    Object three = map->GetValue(keyThree);

    ASSERT_TRUE(one.Exists());
    ASSERT_TRUE(two.Exists());
    ASSERT_TRUE(three.Exists());

    ASSERT_TRUE(one.IsType<int>());
    ASSERT_TRUE(one.IsTypeNumber(Type::Traits<int>::Number));

    ASSERT_EQ(ConstDeref<int>(one), 1);
    ASSERT_EQ(ConstDeref<int>(two), 2);
    ASSERT_EQ(ConstDeref<int>(three), 3);

    // Clean up
    Root().Remove(Label("test_map"));
    Root().Remove(Label("test_key_one"));
    Root().Remove(Label("test_key_two"));
    Root().Remove(Label("test_key_three"));
    Root().Remove(Label("test_val_one"));
    Root().Remove(Label("test_val_two"));
    Root().Remove(Label("test_val_three"));
}

// Test Map with different value types
TEST_F(TestMap, TestMapMixedValues) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Store in root to prevent garbage collection
    Root().Set(Label("test_mixed_map"), map);

    // Create keys and values
    Object keyInt = Reg().New<String>("int");
    Object keyFloat = Reg().New<String>("float");
    Object keyString = Reg().New<String>("string");
    Object keyBool = Reg().New<String>("bool");

    Object valInt = Reg().New<int>(42);
    Object valFloat = Reg().New<float>(3.14f);
    Object valString = Reg().New<String>("Hello");
    Object valBool = Reg().New<bool>(true);

    // Store keys and values in root to prevent garbage collection
    Root().Set(Label("test_key_int"), keyInt);
    Root().Set(Label("test_key_float"), keyFloat);
    Root().Set(Label("test_key_string"), keyString);
    Root().Set(Label("test_key_bool"), keyBool);

    Root().Set(Label("test_val_int"), valInt);
    Root().Set(Label("test_val_float"), valFloat);
    Root().Set(Label("test_val_string"), valString);
    Root().Set(Label("test_val_bool"), valBool);

    // Add key-value pairs with different value types
    map->Insert(keyInt, valInt);
    map->Insert(keyFloat, valFloat);
    map->Insert(keyString, valString);
    map->Insert(keyBool, valBool);

    // Verify retrieval and types using both methods
    ASSERT_TRUE(map->GetValue(keyInt).IsType<int>());
    ASSERT_TRUE(map->GetValue(keyInt).IsTypeNumber(Type::Traits<int>::Number));

    ASSERT_TRUE(map->GetValue(keyFloat).IsType<float>());
    ASSERT_TRUE(
        map->GetValue(keyFloat).IsTypeNumber(Type::Traits<float>::Number));

    ASSERT_TRUE(map->GetValue(keyString).IsType<String>());
    ASSERT_TRUE(
        map->GetValue(keyString).IsTypeNumber(Type::Traits<String>::Number));

    ASSERT_TRUE(map->GetValue(keyBool).IsType<bool>());
    ASSERT_TRUE(
        map->GetValue(keyBool).IsTypeNumber(Type::Traits<bool>::Number));

    ASSERT_EQ(ConstDeref<int>(map->GetValue(keyInt)), 42);
    ASSERT_FLOAT_EQ(ConstDeref<float>(map->GetValue(keyFloat)), 3.14f);
    ASSERT_EQ(ConstDeref<String>(map->GetValue(keyString)), "Hello");
    ASSERT_EQ(ConstDeref<bool>(map->GetValue(keyBool)), true);

    // Clean up
    Root().Remove(Label("test_mixed_map"));
    Root().Remove(Label("test_key_int"));
    Root().Remove(Label("test_key_float"));
    Root().Remove(Label("test_key_string"));
    Root().Remove(Label("test_key_bool"));
    Root().Remove(Label("test_val_int"));
    Root().Remove(Label("test_val_float"));
    Root().Remove(Label("test_val_string"));
    Root().Remove(Label("test_val_bool"));
}

// Test Map key existence check
TEST_F(TestMap, TestMapContains) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Store in root to prevent garbage collection
    Root().Set(Label("test_contains_map"), map);

    // Create keys and values
    Object keyOne = Reg().New<String>("one");
    Object keyTwo = Reg().New<String>("two");
    Object keyThree = Reg().New<String>("three");  // Key that won't be added

    Object valOne = Reg().New<int>(1);
    Object valTwo = Reg().New<int>(2);

    // Store in root to prevent garbage collection
    Root().Set(Label("test_contains_key_one"), keyOne);
    Root().Set(Label("test_contains_key_two"), keyTwo);
    Root().Set(Label("test_contains_key_three"), keyThree);
    Root().Set(Label("test_contains_val_one"), valOne);
    Root().Set(Label("test_contains_val_two"), valTwo);

    // Add some entries
    map->Insert(keyOne, valOne);
    map->Insert(keyTwo, valTwo);

    // Check existing keys
    ASSERT_TRUE(map->ContainsKey(keyOne));
    ASSERT_TRUE(map->ContainsKey(keyTwo));

    // Check non-existing key
    ASSERT_FALSE(map->ContainsKey(keyThree));

    // Clean up
    Root().Remove(Label("test_contains_map"));
    Root().Remove(Label("test_contains_key_one"));
    Root().Remove(Label("test_contains_key_two"));
    Root().Remove(Label("test_contains_key_three"));
    Root().Remove(Label("test_contains_val_one"));
    Root().Remove(Label("test_contains_val_two"));
}

// Test Map value replacement
TEST_F(TestMap, TestMapValueReplacement) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Store in root to prevent garbage collection
    Root().Set(Label("test_replace_map"), map);

    // Create key and values
    Object key = Reg().New<String>("key");
    Object val1 = Reg().New<int>(1);
    Object val2 = Reg().New<int>(2);
    Object val3 = Reg().New<String>("new value");

    // Store in root to prevent garbage collection
    Root().Set(Label("test_replace_key"), key);
    Root().Set(Label("test_replace_val1"), val1);
    Root().Set(Label("test_replace_val2"), val2);
    Root().Set(Label("test_replace_val3"), val3);

    // Add an initial key-value pair
    map->Insert(key, val1);
    ASSERT_EQ(ConstDeref<int>(map->GetValue(key)), 1);

    // Replace the value
    map->Insert(key, val2);
    ASSERT_EQ(ConstDeref<int>(map->GetValue(key)), 2);

    // Replace with a different type
    map->Insert(key, val3);
    ASSERT_TRUE(map->GetValue(key).IsType<String>());
    ASSERT_TRUE(map->GetValue(key).IsTypeNumber(Type::Traits<String>::Number));
    ASSERT_EQ(ConstDeref<String>(map->GetValue(key)), "new value");

    // Clean up
    Root().Remove(Label("test_replace_map"));
    Root().Remove(Label("test_replace_key"));
    Root().Remove(Label("test_replace_val1"));
    Root().Remove(Label("test_replace_val2"));
    Root().Remove(Label("test_replace_val3"));
}

// Test Map Erase operation
TEST_F(TestMap, TestMapErase) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Add entries
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    map->Insert(Reg().New<String>("three"), Reg().New<int>(3));

    ASSERT_EQ(map->Size(), 3);

    // Store a reference to a value
    Handle valueHandle = map->GetValue(Reg().New<String>("two")).GetHandle();

    // Erase an entry
    map->Erase(Reg().New<String>("two"));

    // Verify entry was removed
    ASSERT_EQ(map->Size(), 2);
    ASSERT_FALSE(map->ContainsKey(Reg().New<String>("two")));
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("one")));
    ASSERT_TRUE(map->ContainsKey(Reg().New<String>("three")));

    // Verify value was removed from registry after GC
    Reg().GarbageCollect();

    Object value = Reg().GetObject(valueHandle);
    ASSERT_FALSE(value.Exists());
}

// Test Map iteration
TEST_F(TestMap, TestMapIteration) {
    // Create a map
    Pointer<Map> map = Reg().New<Map>();

    // Add key-value pairs
    map->Insert(Reg().New<String>("one"), Reg().New<int>(1));
    map->Insert(Reg().New<String>("two"), Reg().New<int>(2));
    map->Insert(Reg().New<String>("three"), Reg().New<int>(3));

    // Verify iteration (note: depends on map implementation details)
    int sum = 0;
    for (Map::const_iterator it = map->Begin(); it != map->End(); ++it) {
        sum += ConstDeref<int>(it->second);
    }

    ASSERT_EQ(sum, 6);  // 1 + 2 + 3 = 6
}