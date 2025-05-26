#include <gtest/gtest.h>
#include "KAI/Core/Object.h"
#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/StringStream.h"
#include "TestCommon.h"

using namespace kai;

// Test suite for serialization functionality
class SerializationTest : public TestCommon {
protected:
    void SetUp() override {
        TestCommon::SetUp();
    }
};

// Test 16: Binary serialization of primitives
TEST_F(SerializationTest, BinarySerializationPrimitives) {
    BinaryStream stream(reg);
    
    // Write various types
    stream << reg->New<int>(42);
    stream << reg->New<float>(3.14f);
    stream << reg->New<String>("Hello");
    stream << reg->New<bool>(true);
    
    // Read back
    stream.ToStart();
    
    Object intObj, floatObj, strObj, boolObj;
    stream >> intObj >> floatObj >> strObj >> boolObj;
    
    EXPECT_EQ(ConstDeref<int>(intObj), 42);
    EXPECT_FLOAT_EQ(ConstDeref<float>(floatObj), 3.14f);
    EXPECT_EQ(ConstDeref<String>(strObj), "Hello");
    EXPECT_EQ(ConstDeref<bool>(boolObj), true);
}

// Test 17: Container serialization
TEST_F(SerializationTest, ContainerSerialization) {
    BinaryStream stream(reg);
    
    // Create and populate containers
    auto array = reg->New<Array>();
    Deref<Array>(array).PushBack(reg->New<int>(1));
    Deref<Array>(array).PushBack(reg->New<int>(2));
    Deref<Array>(array).PushBack(reg->New<int>(3));
    
    auto map = reg->New<Map>();
    Deref<Map>(map).Insert(reg->New<String>("key1"), reg->New<int>(100));
    Deref<Map>(map).Insert(reg->New<String>("key2"), reg->New<int>(200));
    
    // Serialize
    stream << array << map;
    
    // Deserialize
    stream.ToStart();
    Object arrayOut, mapOut;
    stream >> arrayOut >> mapOut;
    
    // Verify
    EXPECT_EQ(Deref<Array>(arrayOut).Size(), 3);
    EXPECT_EQ(ConstDeref<int>(Deref<Array>(arrayOut)[0]), 1);
    
    EXPECT_EQ(Deref<Map>(mapOut).Size(), 2);
    EXPECT_TRUE(Deref<Map>(mapOut).Has(reg->New<String>("key1")));
}

// Test 18: String stream serialization
TEST_F(SerializationTest, StringStreamSerialization) {
    StringStream stream;
    
    // Write human-readable format
    stream << "Value: " << 42 << ", Name: " << "Test" << ", Flag: " << true;
    
    String result = stream.ToString();
    EXPECT_TRUE(result.Contains("Value: 42"));
    EXPECT_TRUE(result.Contains("Name: Test"));
    EXPECT_TRUE(result.Contains("Flag: true"));
}

// Test 19: Circular reference handling in serialization
TEST_F(SerializationTest, CircularReferenceSerialization) {
    // Create objects with circular references
    auto obj1 = reg->New<Map>();
    auto obj2 = reg->New<Map>();
    
    Deref<Map>(obj1).Insert(reg->New<String>("next"), obj2);
    Deref<Map>(obj2).Insert(reg->New<String>("prev"), obj1);
    
    BinaryStream stream(reg);
    
    // Serialization should handle circular references
    EXPECT_NO_THROW(stream << obj1);
    
    // Deserialize
    stream.ToStart();
    Object restored;
    EXPECT_NO_THROW(stream >> restored);
}

// Test 20: Custom serialization format
TEST_F(SerializationTest, CustomSerializationFormat) {
    // Test custom serialization through string representation
    struct Point {
        int x, y;
        String ToString() const { 
            return String("(") + String(x) + "," + String(y) + ")"; 
        }
    };
    
    reg->AddClass<Point>(Label("Point"));
    
    auto point = reg->New<Point>();
    Deref<Point>(point).x = 10;
    Deref<Point>(point).y = 20;
    
    StringStream stream;
    stream << Deref<Point>(point).ToString();
    
    EXPECT_EQ(stream.ToString(), "(10,20)");
}