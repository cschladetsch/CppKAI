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
        console_.Execute(code);

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
        console_.Execute(code);

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
        console_.Execute(code);

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

// 1. Test array creation and access
TEST_F(RhoAdvancedDataTests, ArrayCreationAndAccess) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50];\n"
        "arr;",
        {10, 20, 30, 40, 50});
}

// 2. Test array element modification
TEST_F(RhoAdvancedDataTests, ArrayElementModification) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50];\n"
        "arr[2] = 99;\n"
        "arr;",
        {10, 20, 99, 40, 50});
}

// 3. Test array concatenation
TEST_F(RhoAdvancedDataTests, ArrayConcatenation) {
    VerifyArrayResult(
        "arr1 = [1, 2, 3];\n"
        "arr2 = [4, 5, 6];\n"
        "arr1 + arr2;",
        {1, 2, 3, 4, 5, 6});
}

// 4. Test array slicing
TEST_F(RhoAdvancedDataTests, ArraySlicing) {
    VerifyArrayResult(
        "arr = [10, 20, 30, 40, 50];\n"
        "arr.slice(1, 4);",  // Elements at index 1, 2, 3
        {20, 30, 40});
}

// 5. Test array iteration with map-like operation
TEST_F(RhoAdvancedDataTests, ArrayMapOperation) {
    VerifyArrayResult(
        "arr = [1, 2, 3, 4, 5];\n"
        "result = [];\n"
        "for (i = 0; i < arr.size(); i = i + 1) {\n"
        "    result.push(arr[i] * 2);\n"
        "}\n"
        "result;",
        {2, 4, 6, 8, 10});
}

// 6. Test array filtering
TEST_F(RhoAdvancedDataTests, ArrayFilterOperation) {
    VerifyArrayResult(
        "arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];\n"
        "result = [];\n"
        "for (i = 0; i < arr.size(); i = i + 1) {\n"
        "    if (arr[i] % 2 == 0) {\n"
        "        result.push(arr[i]);\n"
        "    }\n"
        "}\n"
        "result;",
        {2, 4, 6, 8, 10});
}

// 7. Test array reduction (sum)
TEST_F(RhoAdvancedDataTests, ArrayReduceSum) {
    ExecuteRhoAndVerify<int>(
        "arr = [1, 2, 3, 4, 5];\n"
        "sum = 0;\n"
        "for (i = 0; i < arr.size(); i = i + 1) {\n"
        "    sum = sum + arr[i];\n"
        "}\n"
        "sum;",
        15);
}

// 8. Test nested arrays
TEST_F(RhoAdvancedDataTests, NestedArrays) {
    ExecuteRhoAndVerify<int>(
        "matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];\n"
        "matrix[1][1];",  // Access element at row 1, column 1 (5)
        5);
}

// 9. Test map creation and access
TEST_F(RhoAdvancedDataTests, MapCreationAndAccess) {
    VerifyMapKeyValues(
        "map = {};\n"
        "map['one'] = 1;\n"
        "map['two'] = 2;\n"
        "map['three'] = 3;\n"
        "map;",
        {{"one", 1}, {"two", 2}, {"three", 3}});
}

// 10. Test map value update
TEST_F(RhoAdvancedDataTests, MapValueUpdate) {
    VerifyMapKeyValues(
        "map = {};\n"
        "map['one'] = 1;\n"
        "map['two'] = 2;\n"
        "map['one'] = 10;\n"  // Update value
        "map;",
        {{"one", 10}, {"two", 2}});
}

// 11. Test map iteration
TEST_F(RhoAdvancedDataTests, MapIteration) {
    ExecuteRhoAndVerify<int>(
        "map = {};\n"
        "map['a'] = 5;\n"
        "map['b'] = 10;\n"
        "map['c'] = 15;\n"
        "sum = 0;\n"
        "keys = map.keys();\n"
        "for (i = 0; i < keys.size(); i = i + 1) {\n"
        "    key = keys[i];\n"
        "    sum = sum + map[key];\n"
        "}\n"
        "sum;",
        30);
}

// 12. Test complex data structure (array of maps)
TEST_F(RhoAdvancedDataTests, ArrayOfMaps) {
    ExecuteRhoAndVerify<int>(
        "users = [\n"
        "    { 'name': 'Alice', 'age': 30, 'score': 85 },\n"
        "    { 'name': 'Bob', 'age': 25, 'score': 92 },\n"
        "    { 'name': 'Charlie', 'age': 35, 'score': 78 }\n"
        "];\n"
        "users[1]['score'];",  // Access Bob's score
        92);
}

// 13. Test complex data structure manipulation
TEST_F(RhoAdvancedDataTests, ComplexDataManipulation) {
    ExecuteRhoAndVerify<int>(
        "users = [\n"
        "    { 'name': 'Alice', 'age': 30, 'score': 85 },\n"
        "    { 'name': 'Bob', 'age': 25, 'score': 92 },\n"
        "    { 'name': 'Charlie', 'age': 35, 'score': 78 }\n"
        "];\n"
        "totalScore = 0;\n"
        "for (i = 0; i < users.size(); i = i + 1) {\n"
        "    totalScore = totalScore + users[i]['score'];\n"
        "}\n"
        "averageScore = totalScore / users.size();\n"
        "averageScore;",
        85  // (85 + 92 + 78) / 3 = 85
    );
}

// 14. Test string operations with arrays
TEST_F(RhoAdvancedDataTests, StringArrayOperations) {
    ExecuteRhoAndVerifyString(
        "words = ['Hello', ' ', 'World', '!'];\n"
        "message = '';\n"
        "for (i = 0; i < words.size(); i = i + 1) {\n"
        "    message = message + words[i];\n"
        "}\n"
        "message;",
        "Hello World!");
}

// 15. Test higher order function simulation (passing functions as values)
TEST_F(RhoAdvancedDataTests, HigherOrderFunctions) {
    ExecuteRhoAndVerify<int>(
        "function applyOperation(a, b, operation) {\n"
        "    if (operation == 'add') {\n"
        "        return a + b;\n"
        "    } else if (operation == 'multiply') {\n"
        "        return a * b;\n"
        "    } else if (operation == 'subtract') {\n"
        "        return a - b;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n"
        "result = applyOperation(10, 5, 'multiply');\n"
        "result;",
        50);
}

// 16. Test closures simulation
TEST_F(RhoAdvancedDataTests, ClosureSimulation) {
    ExecuteRhoAndVerify<int>(
        "function makeCounter(start) {\n"
        "    count = start;\n"
        "    function increment() {\n"
        "        count = count + 1;\n"
        "        return count;\n"
        "    }\n"
        "    return increment;\n"
        "}\n"
        "counter = makeCounter(10);\n"
        "counter();\n"  // Returns 11
        "counter();\n"  // Returns 12
        "counter();",   // Returns 13
        13);
}

// 17. Test advanced map manipulations
TEST_F(RhoAdvancedDataTests, AdvancedMapManipulations) {
    ExecuteRhoAndVerifyString(
        "config = {\n"
        "    'server': 'api.example.com',\n"
        "    'port': 8080,\n"
        "    'secure': true,\n"
        "    'timeout': 30\n"
        "};\n"
        "function buildConnectionString(config) {\n"
        "    protocol = config['secure'] ? 'https' : 'http';\n"
        "    return protocol + '://' + config['server'] + ':' + "
        "config['port'];\n"
        "}\n"
        "buildConnectionString(config);",
        "https://api.example.com:8080");
}

// 18. Test array sorting algorithm
TEST_F(RhoAdvancedDataTests, ArraySorting) {
    VerifyArrayResult(
        "function bubbleSort(arr) {\n"
        "    n = arr.size();\n"
        "    for (i = 0; i < n; i = i + 1) {\n"
        "        for (j = 0; j < n - i - 1; j = j + 1) {\n"
        "            if (arr[j] > arr[j + 1]) {\n"
        "                // Swap the elements\n"
        "                temp = arr[j];\n"
        "                arr[j] = arr[j + 1];\n"
        "                arr[j + 1] = temp;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    return arr;\n"
        "}\n"
        "unsortedArray = [64, 34, 25, 12, 22, 11, 90];\n"
        "bubbleSort(unsortedArray);",
        {11, 12, 22, 25, 34, 64, 90});
}

// 19. Test complex return value from function
TEST_F(RhoAdvancedDataTests, ComplexReturnValue) {
    ExecuteRhoAndVerify<int>(
        "function processData(data) {\n"
        "    result = {\n"
        "        'min': data[0],\n"
        "        'max': data[0],\n"
        "        'sum': 0,\n"
        "        'avg': 0\n"
        "    };\n"
        "    \n"
        "    for (i = 0; i < data.size(); i = i + 1) {\n"
        "        value = data[i];\n"
        "        if (value < result['min']) {\n"
        "            result['min'] = value;\n"
        "        }\n"
        "        if (value > result['max']) {\n"
        "            result['max'] = value;\n"
        "        }\n"
        "        result['sum'] = result['sum'] + value;\n"
        "    }\n"
        "    \n"
        "    result['avg'] = result['sum'] / data.size();\n"
        "    return result;\n"
        "}\n"
        "\n"
        "data = [4, 7, 2, 9, 3];\n"
        "result = processData(data);\n"
        "result['max'];",
        9);
}

// 20. Test data transformation pipeline
TEST_F(RhoAdvancedDataTests, DataTransformationPipeline) {
    VerifyArrayResult(
        "function filterEven(arr) {\n"
        "    result = [];\n"
        "    for (i = 0; i < arr.size(); i = i + 1) {\n"
        "        if (arr[i] % 2 == 0) {\n"
        "            result.push(arr[i]);\n"
        "        }\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "function doubleValues(arr) {\n"
        "    result = [];\n"
        "    for (i = 0; i < arr.size(); i = i + 1) {\n"
        "        result.push(arr[i] * 2);\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "function addOffset(arr, offset) {\n"
        "    result = [];\n"
        "    for (i = 0; i < arr.size(); i = i + 1) {\n"
        "        result.push(arr[i] + offset);\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];\n"
        "step1 = filterEven(data);           // [2, 4, 6, 8, 10]\n"
        "step2 = doubleValues(step1);        // [4, 8, 12, 16, 20]\n"
        "step3 = addOffset(step2, 5);        // [9, 13, 17, 21, 25]\n"
        "step3;",
        {9, 13, 17, 21, 25});
}