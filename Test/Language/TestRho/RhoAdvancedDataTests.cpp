#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/Map.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Logger.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing advanced Rho data structure operations
struct RhoAdvancedDataTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);

        // Register additional types needed for tests
        reg_->AddClass<Map>(Label("Map"));
        reg_->AddClass<Array>(Label("Array"));
        reg_->AddClass<String>(Label("String"));
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<bool>(Label("bool"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to execute Rho code and verify the result
    template <typename T>
    void ExecuteRhoAndVerify(const std::string& code, const T& expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Rho code
        console_.Execute(code, Structure::Program);

        // Process the stack to extract values from continuations
        UnwrapStackValues();

        // Verify the result
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_TRUE(data_->Top().IsType<T>())
            << "Expected result type " << typeid(T).name() << " but got "
            << (data_->Top().Exists()
                    ? data_->Top().GetClass()->GetName().ToString()
                    : "null");
        ASSERT_EQ(ConstDeref<T>(data_->Top()), expected)
            << "Expected value " << expected << " but got "
            << ConstDeref<T>(data_->Top());
    }

    // Helper to verify string results
    void ExecuteRhoAndVerifyString(const std::string& code,
                                   const std::string& expected) {
        ExecuteRhoAndVerify<String>(code, String(expected));
    }

    // Helper to verify array operations
    void VerifyArrayResult(const std::string& code, std::vector<int> expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Rho code
        console_.Execute(code, Structure::Program);

        // Process the stack
        UnwrapStackValues();

        // Verify the result is an array
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_TRUE(data_->Top().IsType<Array>())
            << "Expected Array type but got "
            << (data_->Top().Exists()
                    ? data_->Top().GetClass()->GetName().ToString()
                    : "null");

        Pointer<Array> arr = data_->Top();

        // Verify array size
        ASSERT_EQ(arr->Size(), expected.size())
            << "Array size doesn't match expected size";

        // Verify array elements
        for (size_t i = 0; i < expected.size() && i < arr->Size(); i++) {
            ASSERT_TRUE(arr->At(i).IsType<int>())
                << "Array element " << i << " is not an integer";
            ASSERT_EQ(ConstDeref<int>(arr->At(i)), expected[i])
                << "Array element " << i << " doesn't match expected value";
        }
    }

    // Helper to verify map operations
    void VerifyMapKeyValues(const std::string& code,
                            std::vector<std::pair<String, int>> expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Rho code
        console_.Execute(code, Structure::Program);

        // Process the stack
        UnwrapStackValues();

        // Verify the result is a map
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_TRUE(data_->Top().IsType<Map>())
            << "Expected Map type but got "
            << (data_->Top().Exists()
                    ? data_->Top().GetClass()->GetName().ToString()
                    : "null");

        Pointer<Map> map = data_->Top();

        // Verify map size
        ASSERT_EQ(map->Size(), expected.size())
            << "Map size doesn't match expected size";

        // Verify map entries
        for (const auto& [key, expectedValue] : expected) {
            Object keyObj = reg_->New<String>(key);
            ASSERT_TRUE(map->ContainsKey(keyObj))
                << "Map does not contain expected key: " << key;

            Object valueObj = map->GetValue(keyObj);
            ASSERT_TRUE(valueObj.IsType<int>())
                << "Map value for key " << key << " is not an integer";

            ASSERT_EQ(ConstDeref<int>(valueObj), expectedValue)
                << "Map value for key " << key
                << " doesn't match expected value";
        }
    }
};

// Test array creation and access
TEST_F(RhoAdvancedDataTests, ArrayCreationAndAccess) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50]\n"
        "arr",
        {10, 20, 30, 40, 50});
}

// Test array element modification
TEST_F(RhoAdvancedDataTests, ArrayElementModification) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50]\n"
        "arr[2] = 99\n"
        "arr",
        {10, 20, 99, 40, 50});
}

// Test array concatenation
// Test array concatenation
TEST_F(RhoAdvancedDataTests, ArrayConcatenation) {
    VerifyArrayResult(
        "arr1 = [1, 2, 3]\n"
        "arr2 = [4, 5, 6]\n"
        "arr1 + arr2",
        {1, 2, 3, 4, 5, 6});
}

// Comprehensive array Plus operation tests
TEST_F(RhoAdvancedDataTests, ArrayPlusOperations) {
    // Test 1: Basic concatenation
    VerifyArrayResult("[1, 2] + [3, 4]", {1, 2, 3, 4});

    // Test 2: Empty array + non-empty array
    VerifyArrayResult("[] + [1, 2, 3]", {1, 2, 3});

    // Test 3: Non-empty array + empty array
    VerifyArrayResult("[1, 2, 3] + []", {1, 2, 3});

    // Test 4: Empty array + empty array
    VerifyArrayResult("[] + []", {});

    // Test 5: Multiple concatenations
    VerifyArrayResult("[1] + [2] + [3] + [4]", {1, 2, 3, 4});

    // Test 6: Concatenation with assignment
    VerifyArrayResult(
        "a = [1, 2]\n"
        "b = [3, 4]\n"
        "c = a + b\n"
        "c",
        {1, 2, 3, 4});

    // Test 7: Original arrays unchanged after concatenation
    VerifyArrayResult(
        "a = [1, 2]\n"
        "b = [3, 4]\n"
        "c = a + b\n"
        "a",  // Check that 'a' is unchanged
        {1, 2});

    // Test 8: Concatenating arrays with same size different values
    VerifyArrayResult("[10, 20] + [30, 40]", {10, 20, 30, 40});

    // Test 9: Concatenating larger arrays
    VerifyArrayResult("[1, 2, 3, 4, 5] + [6, 7, 8, 9, 10]",
                      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    // Test 10: Chain concatenation with variables
    VerifyArrayResult(
        "x = [1, 2]\n"
        "y = [3, 4]\n"
        "z = [5, 6]\n"
        "result = x + y + z\n"
        "result",
        {1, 2, 3, 4, 5, 6});
}

// Test array slicing
// Disabled: requires unimplemented .slice method
TEST_F(RhoAdvancedDataTests, ArraySlicing) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50]\n"
        "arr.slice(1, 4)",  // Elements at index 1, 2, 3
        {20, 30, 40});
}

// Test array iteration with map-like operation
// Disabled: requires unimplemented .size method
TEST_F(RhoAdvancedDataTests, ArrayMapOperation) {
    VerifyArrayResult(
        "arr = [1, 2, 3, 4, 5]\n"
        "result = []\n"
        "for i = 0; i < arr.size(); i = i + 1\n"
        "    result.push(arr[i] * 2)\n"
        "result",
        {2, 4, 6, 8, 10});
}

// Test array filtering
// Disabled: requires unimplemented features (likely .size and .push methods)
TEST_F(RhoAdvancedDataTests, ArrayFilterOperation) {
    VerifyArrayResult(
        "arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]\n"
        "result = []\n"
        "for i = 0; i < arr.size(); i = i + 1\n"
        "    if arr[i] % 2 == 0\n"
        "        result.push(arr[i])\n"
        "result",
        {2, 4, 6, 8, 10});
}

// Test array reduction (sum)
// Disabled: requires unimplemented features (likely .size method)
TEST_F(RhoAdvancedDataTests, ArrayReduceSum) {
    ExecuteRhoAndVerify<int>(
        "arr = [1, 2, 3, 4, 5]\n"
        "sum = 0\n"
        "for i = 0; i < arr.size(); i = i + 1\n"
        "    sum = sum + arr[i]\n"
        "sum",
        15);
}

// Test nested arrays
TEST_F(RhoAdvancedDataTests, NestedArrays) {
    ExecuteRhoAndVerify<int>(
        "matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]\n"
        "matrix[1][1]",  // Access element at row 1, column 1 (5)
        5);
}

// Test map creation and access
// Disabled: maps not fully implemented
TEST_F(RhoAdvancedDataTests, MapCreationAndAccess) {
    VerifyMapKeyValues(
        "map = {}\n"
        "map['one'] = 1\n"
        "map['two'] = 2\n"
        "map['three'] = 3\n"
        "map",
        {{"one", 1}, {"two", 2}, {"three", 3}});
}

// Test map value update
// Disabled: maps not fully implemented
TEST_F(RhoAdvancedDataTests, MapValueUpdate) {
    VerifyMapKeyValues(
        "map = {}\n"
        "map['one'] = 1\n"
        "map['two'] = 2\n"
        "map['one'] = 10\n"  // Update value
        "map",
        {{"one", 10}, {"two", 2}});
}

// Test map iteration
// Disabled: maps not fully implemented
TEST_F(RhoAdvancedDataTests, MapIteration) {
    ExecuteRhoAndVerify<int>(
        "map = {}\n"
        "map['a'] = 5\n"
        "map['b'] = 10\n"
        "map['c'] = 15\n"
        "sum = 0\n"
        "keys = map.keys()\n"
        "for i = 0; i < keys.size(); i = i + 1\n"
        "    key = keys[i]\n"
        "    sum = sum + map[key]\n"
        "sum",
        30);
}

// Test complex data structure (array of maps)
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, ArrayOfMaps) {
    ExecuteRhoAndVerify<int>(
        "users = [\n"
        "    { 'name': 'Alice', 'age': 30, 'score': 85 },\n"
        "    { 'name': 'Bob', 'age': 25, 'score': 92 },\n"
        "    { 'name': 'Charlie', 'age': 35, 'score': 78 }\n"
        "]\n"
        "users[1]['score']",  // Access Bob's score
        92);
}

// Test complex data structure manipulation
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, ComplexDataManipulation) {
    ExecuteRhoAndVerify<int>(
        "users = [\n"
        "    { 'name': 'Alice', 'age': 30, 'score': 85 },\n"
        "    { 'name': 'Bob', 'age': 25, 'score': 92 },\n"
        "    { 'name': 'Charlie', 'age': 35, 'score': 78 }\n"
        "]\n"
        "totalScore = 0\n"
        "for i = 0; i < users.size(); i = i + 1\n"
        "    totalScore = totalScore + users[i]['score']\n"
        "averageScore = totalScore / users.size()\n"
        "averageScore",
        85  // (85 + 92 + 78) / 3 = 85
    );
}

// Test string operations with arrays
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, StringArrayOperations) {
    ExecuteRhoAndVerifyString(
        "words = ['Hello', ' ', 'World', '!']\n"
        "message = ''\n"
        "for i = 0; i < words.size(); i = i + 1\n"
        "    message = message + words[i]\n"
        "message",
        "Hello World!");
}

// Test higher order function simulation (passing functions as values)
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, HigherOrderFunctions) {
    ExecuteRhoAndVerify<int>(
        "fun applyOperation(a, b, operation)\n"
        "    if operation == 'add'\n"
        "        return a + b\n"
        "    else if operation == 'multiply'\n"
        "        return a * b\n"
        "    else if operation == 'subtract'\n"
        "        return a - b\n"
        "    else\n"
        "        return 0\n"
        "result = applyOperation(10, 5, 'multiply')\n"
        "result",
        50);
}

// Test closures simulation
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, ClosureSimulation) {
    ExecuteRhoAndVerify<int>(
        "fun makeCounter(start)\n"
        "    count = start\n"
        "    fun increment()\n"
        "        count = count + 1\n"
        "        return count\n"
        "    return increment\n"
        "counter = makeCounter(10)\n"
        "counter()\n"  // Returns 11
        "counter()\n"  // Returns 12
        "counter()",   // Returns 13
        13);
}

// Test advanced map manipulations
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, AdvancedMapManipulations) {
    ExecuteRhoAndVerifyString(
        "config = {\n"
        "    'server': 'api.example.com',\n"
        "    'port': 8080,\n"
        "    'secure': true,\n"
        "    'timeout': 30\n"
        "}\n"
        "fun buildConnectionString(config)\n"
        "    protocol = config['secure'] ? 'https' : 'http'\n"
        "    return protocol + '://' + config['server'] + ':' + "
        "config['port']\n"
        "buildConnectionString(config)",
        "https://api.example.com:8080");
}

// Test array sorting algorithm
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, ArraySorting) {
    VerifyArrayResult(
        "fun bubbleSort(arr)\n"
        "    n = arr.size()\n"
        "    for i = 0; i < n; i = i + 1\n"
        "        for j = 0; j < n - i - 1; j = j + 1\n"
        "            if arr[j] > arr[j + 1]\n"
        "                // Swap the elements\n"
        "                temp = arr[j]\n"
        "                arr[j] = arr[j + 1]\n"
        "                arr[j + 1] = temp\n"
        "    return arr\n"
        "unsortedArray = [64, 34, 25, 12, 22, 11, 90]\n"
        "bubbleSort(unsortedArray)",
        {11, 12, 22, 25, 34, 64, 90});
}

// Test complex return value from function
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, ComplexReturnValue) {
    ExecuteRhoAndVerify<int>(
        "fun processData(data)\n"
        "    result = {\n"
        "        'min': data[0],\n"
        "        'max': data[0],\n"
        "        'sum': 0,\n"
        "        'avg': 0\n"
        "    }\n"
        "    \n"
        "    for i = 0; i < data.size(); i = i + 1\n"
        "        value = data[i]\n"
        "        if value < result['min']\n"
        "            result['min'] = value\n"
        "        if value > result['max']\n"
        "            result['max'] = value\n"
        "        result['sum'] = result['sum'] + value\n"
        "    \n"
        "    result['avg'] = result['sum'] / data.size()\n"
        "    return result\n"
        "\n"
        "data = [4, 7, 2, 9, 3]\n"
        "result = processData(data)\n"
        "result['max']",
        9);
}

// Test data transformation pipeline
// Requires unimplemented features
TEST_F(RhoAdvancedDataTests, DataTransformationPipeline) {
    VerifyArrayResult(
        "fun filterEven(arr)\n"
        "    result = []\n"
        "    for i = 0; i < arr.size(); i = i + 1\n"
        "        if arr[i] % 2 == 0\n"
        "            result.push(arr[i])\n"
        "    return result\n"
        "\n"
        "fun doubleValues(arr)\n"
        "    result = []\n"
        "    for i = 0; i < arr.size(); i = i + 1\n"
        "        result.push(arr[i] * 2)\n"
        "    return result\n"
        "\n"
        "fun addOffset(arr, offset)\n"
        "    result = []\n"
        "    for i = 0; i < arr.size(); i = i + 1\n"
        "        result.push(arr[i] + offset)\n"
        "    return result\n"
        "\n"
        "data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]\n"
        "step1 = filterEven(data)           // [2, 4, 6, 8, 10]\n"
        "step2 = doubleValues(step1)        // [4, 8, 12, 16, 20]\n"
        "step3 = addOffset(step2, 5)        // [9, 13, 17, 21, 25]\n"
        "step3",
        {9, 13, 17, 21, 25});
}

// Direct test of Array Plus operator implementation
TEST_F(RhoAdvancedDataTests, DirectArrayPlusOperator) {
    // Create two arrays
    auto arr1 = reg_->New<Array>();
    auto& a1 = Deref<Array>(arr1);
    a1.Append(reg_->New(1));
    a1.Append(reg_->New(2));
    a1.Append(reg_->New(3));

    auto arr2 = reg_->New<Array>();
    auto& a2 = Deref<Array>(arr2);
    a2.Append(reg_->New(4));
    a2.Append(reg_->New(5));
    a2.Append(reg_->New(6));

    // Test the Plus operator
    Array result = a1 + a2;

    // Verify the result
    ASSERT_EQ(result.Size(), 6);
    EXPECT_EQ(ConstDeref<int>(result.At(0)), 1);
    EXPECT_EQ(ConstDeref<int>(result.At(1)), 2);
    EXPECT_EQ(ConstDeref<int>(result.At(2)), 3);
    EXPECT_EQ(ConstDeref<int>(result.At(3)), 4);
    EXPECT_EQ(ConstDeref<int>(result.At(4)), 5);
    EXPECT_EQ(ConstDeref<int>(result.At(5)), 6);

    // Verify original arrays are unchanged
    ASSERT_EQ(a1.Size(), 3);
    ASSERT_EQ(a2.Size(), 3);
}

// Test empty array concatenation with Plus operator
TEST_F(RhoAdvancedDataTests, EmptyArrayPlusOperator) {
    auto arr1 = reg_->New<Array>();
    auto& a1 = Deref<Array>(arr1);

    auto arr2 = reg_->New<Array>();
    auto& a2 = Deref<Array>(arr2);
    a2.Append(reg_->New(1));
    a2.Append(reg_->New(2));

    // Test empty + non-empty
    Array result1 = a1 + a2;
    ASSERT_EQ(result1.Size(), 2);
    EXPECT_EQ(ConstDeref<int>(result1.At(0)), 1);
    EXPECT_EQ(ConstDeref<int>(result1.At(1)), 2);

    // Test non-empty + empty
    Array result2 = a2 + a1;
    ASSERT_EQ(result2.Size(), 2);
    EXPECT_EQ(ConstDeref<int>(result2.At(0)), 1);
    EXPECT_EQ(ConstDeref<int>(result2.At(1)), 2);

    // Test empty + empty
    Array result3 = a1 + a1;
    ASSERT_EQ(result3.Size(), 0);
}