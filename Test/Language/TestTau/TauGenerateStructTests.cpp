#include <KAI/Language/Tau/Generate/GenerateStruct.h>

#include <regex>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

TAU_BEGIN

struct TauGenerateStructTests : TestLangCommon {};

TEST_F(TauGenerateStructTests, TestBasicStructGeneration) {
    string input = R"(
    namespace Test {
        struct Point {
            int x;
            int y;
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());
    std::cout << "Generated output:\n" << output << std::endl;
    std::cout << "Expected to find: 'struct Point', 'int x;', 'int y;'"
              << std::endl;

    EXPECT_TRUE(output.find("struct Point") != string::npos);
    EXPECT_TRUE(output.find("int x;") != string::npos);
    EXPECT_TRUE(output.find("int y;") != string::npos);
}

TEST_F(TauGenerateStructTests, TestStructWithMethods) {
    string input = R"(
    namespace Graphics {
        struct Rectangle {
            float width;
            float height;
            float GetArea();
            void SetSize(float w, float h);
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());
    EXPECT_TRUE(output.find("struct Rectangle") != string::npos);
    EXPECT_TRUE(output.find("float width;") != string::npos);
    EXPECT_TRUE(output.find("float height;") != string::npos);
    EXPECT_TRUE(output.find("float GetArea();") != string::npos);
    EXPECT_TRUE(output.find("void SetSize(float w, float h);") != string::npos);
}

TEST_F(TauGenerateStructTests, TestNestedStructs) {
    string input = R"(
    namespace Data {
        struct Inner {
            int value;
        }
        
        struct Outer {
            Inner inner;
            string name;
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());
    EXPECT_TRUE(output.find("struct Inner") != string::npos);
    EXPECT_TRUE(output.find("struct Outer") != string::npos);
    EXPECT_TRUE(output.find("int value;") != string::npos);
    EXPECT_TRUE(output.find("Inner inner;") != string::npos);
    EXPECT_TRUE(output.find("string name;") != string::npos);
}

TEST_F(TauGenerateStructTests, TestMultipleNamespacesWithStructs) {
    string input = R"(
    namespace Model {
        struct User {
            int id;
            string name;
        }
    }
    
    namespace Network {
        struct Packet {
            int size;
            string data;
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Check both namespaces are present
    EXPECT_TRUE(output.find("namespace Model") != string::npos);
    EXPECT_TRUE(output.find("namespace Network") != string::npos);

    // Check both structs are present
    EXPECT_TRUE(output.find("struct User") != string::npos);
    EXPECT_TRUE(output.find("struct Packet") != string::npos);
}

TEST_F(TauGenerateStructTests, TestClassTreatedAsStruct) {
    string input = R"(
    namespace Test {
        class DataClass {
            int value;
            string text;
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());
    // GenerateStruct should treat classes as structs
    EXPECT_TRUE(output.find("struct DataClass") != string::npos);
    EXPECT_TRUE(output.find("int value;") != string::npos);
    EXPECT_TRUE(output.find("string text;") != string::npos);
}

TEST_F(TauGenerateStructTests, TestStructsIgnoreInterfaces) {
    string input = R"(
    namespace Test {
        struct Data {
            int id;
        }
        
        interface IService {
            void Process();
        }
    }
    )";

    string output;
    Generate::GenerateStruct generator(input.c_str(), output);

    ASSERT_FALSE(output.empty());

    // Should have the struct
    EXPECT_TRUE(output.find("struct Data") != string::npos);

    // Should NOT have interface-related content
    EXPECT_TRUE(output.find("interface") == string::npos);
    EXPECT_TRUE(output.find("IService") == string::npos);
}

TAU_END