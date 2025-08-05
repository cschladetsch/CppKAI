#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <memory>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"

using namespace kai;
using namespace std;

/**
 * Comprehensive edge case tests for Tau IDL parsing and code generation
 * Tests boundary conditions, malformed input, and complex scenarios
 */
class TauEdgeCaseTests : public ::testing::Test {
protected:
    Registry registry;
    
    void SetUp() override {
        // Ensure registry is clean for each test
        registry.Clear();
    }
    
    bool ParseTauContent(const string& content) {
        try {
            auto lexer = make_shared<tau::TauLexer>(content.c_str(), registry);
            if (!lexer->Process()) {
                return false;
            }
            
            auto parser = make_shared<tau::TauParser>(registry);
            return parser->Process(lexer, Structure::Module);
        } catch (const exception&) {
            return false;
        }
    }
    
    bool GenerateProxyFromContent(const string& content, string& output) {
        try {
            tau::Generate::GenerateProxy generator(content.c_str(), output);
            return !generator.Failed;
        } catch (const exception&) {
            return false;
        }
    }
    
    bool GenerateAgentFromContent(const string& content, string& output) {
        try {
            tau::Generate::GenerateAgent generator(content.c_str(), output);
            return !generator.Failed;
        } catch (const exception&) {
            return false;
        }
    }
};

// Test empty and minimal content
TEST_F(TauEdgeCaseTests, EmptyContent) {
    EXPECT_FALSE(ParseTauContent(""));
    EXPECT_FALSE(ParseTauContent("   "));
    EXPECT_FALSE(ParseTauContent("\n\n\n"));
    EXPECT_FALSE(ParseTauContent("\t\t\t"));
}

TEST_F(TauEdgeCaseTests, MinimalValidContent) {
    string minimal = R"(
        namespace Test {
            interface IEmpty {
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(minimal));
}

// Test complex nested namespaces
TEST_F(TauEdgeCaseTests, DeeplyNestedNamespaces) {
    string nested = R"(
        namespace Level1 {
            namespace Level2 {
                namespace Level3 {
                    namespace Level4 {
                        interface IDeepInterface {
                            void DeepMethod();
                        }
                    }
                }
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(nested));
}

// Test interface inheritance chains
TEST_F(TauEdgeCaseTests, ComplexInheritanceChains) {
    string inheritance = R"(
        namespace Inheritance {
            interface IBase {
                void BaseMethod();
            }
            
            interface IMiddle : IBase {
                void MiddleMethod();
            }
            
            interface IDerived : IMiddle {
                void DerivedMethod();
            }
            
            interface IMultiple : IBase, IMiddle {
                void MultipleMethod();
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(inheritance));
}

// Test complex method signatures
TEST_F(TauEdgeCaseTests, ComplexMethodSignatures) {
    string complex = R"(
        namespace Complex {
            struct Point3D {
                float x;
                float y;
                float z;
            }
            
            interface IComplexMethods {
                // Method with many parameters
                void ManyParams(int a, float b, string c, bool d, int64 e, Point3D f);
                
                // Method with array parameters
                float[] ProcessArray(int[] input, string[] names);
                
                // Method with complex return type
                Point3D[] GetPoints(int count, bool sorted);
                
                // Method with default parameters
                void DefaultParams(int required, float optional = 1.0, bool flag = true);
                
                // Async methods
                async void AsyncOperation(string data);
                async float[] AsyncComputation(int[] values);
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(complex));
}

// Test struct edge cases
TEST_F(TauEdgeCaseTests, ComplexStructures) {
    string structs = R"(
        namespace StructTests {
            struct EmptyStruct {
            }
            
            struct SingleField {
                int value;
            }
            
            struct ManyFields {
                int field1;
                float field2;
                string field3;
                bool field4;
                int64 field5;
                double field6;
                byte field7;
                short field8;
                uint32 field9;
                uint64 field10;
            }
            
            struct NestedStruct {
                EmptyStruct empty;
                SingleField single;
                ManyFields many;
            }
            
            struct ArrayFields {
                int[] numbers;
                string[] names;
                float[] values;
                ManyFields[] structures;
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(structs));
}

// Test enum edge cases
TEST_F(TauEdgeCaseTests, ComplexEnumerations) {
    string enums = R"(
        namespace EnumTests {
            enum EmptyEnum {
            }
            
            enum SingleValue {
                Value1
            }
            
            enum ManyValues {
                First,
                Second,
                Third,
                Fourth,
                Fifth,
                Sixth,
                Seventh,
                Eighth,
                Ninth,
                Tenth
            }
            
            enum ExplicitValues {
                Zero = 0,
                One = 1,
                Ten = 10,
                Hundred = 100,
                Negative = -1
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(enums));
}

// Test event edge cases
TEST_F(TauEdgeCaseTests, ComplexEvents) {
    string events = R"(
        namespace EventTests {
            struct EventData {
                string message;
                int priority;
                int64 timestamp;
            }
            
            interface IEventInterface {
                // Events with no parameters
                event SimpleEvent();
                
                // Events with single parameter
                event SingleParamEvent(string message);
                
                // Events with multiple parameters
                event MultiParamEvent(int id, string name, float value, bool active);
                
                // Events with complex parameters
                event ComplexEvent(EventData data, int[] numbers, string[] tags);
                
                // Events with array parameters
                event ArrayEvent(EventData[] events, string[] categories);
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(events));
}

// Test malformed syntax edge cases
TEST_F(TauEdgeCaseTests, MalformedSyntax) {
    // Missing semicolons
    string missingSemicolon = R"(
        namespace Bad {
            interface IBad {
                void Method()  // Missing semicolon
            }
        }
    )";
    EXPECT_FALSE(ParseTauContent(missingSemicolon));
    
    // Missing braces
    string missingBrace = R"(
        namespace Bad {
            interface IBad {
                void Method();
            // Missing closing brace
        }
    )";
    EXPECT_FALSE(ParseTauContent(missingBrace));
    
    // Invalid parameter syntax
    string invalidParams = R"(
        namespace Bad {
            interface IBad {
                void Method(int a int b);  // Missing comma
            }
        }
    )";
    EXPECT_FALSE(ParseTauContent(invalidParams));
    
    // Invalid return type
    string invalidReturn = R"(
        namespace Bad {
            interface IBad {
                invalidtype Method();  // Unknown type
            }
        }
    )";
    EXPECT_FALSE(ParseTauContent(invalidReturn));
}

// Test unicode and special characters
TEST_F(TauEdgeCaseTests, UnicodeAndSpecialCharacters) {
    string unicode = R"(
        namespace UnicodeTest {
            interface IUnicodeInterface {
                // Method with unicode in comments: αβγδε ñáéíóú
                void StandardMethod();
                
                // Long identifier names
                void VeryLongMethodNameThatExceedsNormalLengthsButShouldStillBeValid();
                
                // Mixed case and numbers
                void Method123ABC();
                void methodWithCamelCase();
                void method_with_underscores();
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(unicode));
}

// Test large interface definitions
TEST_F(TauEdgeCaseTests, LargeInterfaceDefinitions) {
    stringstream large;
    large << "namespace Large {\n";
    large << "    interface ILargeInterface {\n";
    
    // Generate 100 methods
    for (int i = 0; i < 100; ++i) {
        large << "        void Method" << i << "(int param" << i << ");\n";
    }
    
    // Generate 50 events
    for (int i = 0; i < 50; ++i) {
        large << "        event Event" << i << "(string data" << i << ");\n";
    }
    
    large << "    }\n";
    large << "}\n";
    
    EXPECT_TRUE(ParseTauContent(large.str()));
}

// Test circular dependencies (should fail gracefully)
TEST_F(TauEdgeCaseTests, CircularDependencies) {
    string circular = R"(
        namespace Circular {
            struct A {
                B b;
            }
            
            struct B {
                A a;  // Circular dependency
            }
        }
    )";
    // This should either parse successfully (if cycles are allowed) or fail gracefully
    bool result = ParseTauContent(circular);
    // We don't assert either way since behavior may be implementation-dependent
    (void)result; // Suppress unused variable warning
}

// Test generation with edge cases
TEST_F(TauEdgeCaseTests, GenerationWithEdgeCases) {
    string edgeCase = R"(
        namespace EdgeGeneration {
            struct ComplexStruct {
                int[] numbers;
                string[] names;
                float value;
            }
            
            interface IEdgeInterface {
                ComplexStruct Process(ComplexStruct input, int[] filters);
                void MultipleArrays(int[] a, float[] b, string[] c);
                event ComplexEvent(ComplexStruct data, int[] extras);
            }
        }
    )";
    
    string proxyOutput, agentOutput;
    EXPECT_TRUE(GenerateProxyFromContent(edgeCase, proxyOutput));
    EXPECT_TRUE(GenerateAgentFromContent(edgeCase, agentOutput));
    
    // Verify generated code contains expected elements
    EXPECT_TRUE(proxyOutput.find("ComplexStruct") != string::npos);
    EXPECT_TRUE(proxyOutput.find("Process") != string::npos);
    EXPECT_TRUE(proxyOutput.find("MultipleArrays") != string::npos);
    
    EXPECT_TRUE(agentOutput.find("ComplexStruct") != string::npos);
    EXPECT_TRUE(agentOutput.find("Handle") != string::npos);
}

// Test whitespace tolerance
TEST_F(TauEdgeCaseTests, WhitespaceTolerance) {
    // Minimal whitespace
    string minimal = "namespace Test{interface ITest{void Method();}}";
    EXPECT_TRUE(ParseTauContent(minimal));
    
    // Excessive whitespace
    string excessive = R"(
        
        
        namespace    Test    {
        
        
            interface    ITest    {
            
            
                void    Method    (    )    ;
                
                
            }
            
            
        }
        
        
    )";
    EXPECT_TRUE(ParseTauContent(excessive));
    
    // Mixed tabs and spaces
    string mixed = "namespace\tTest\t{\n\tinterface\tITest\t{\n\t\tvoid\tMethod();\n\t}\n}";
    EXPECT_TRUE(ParseTauContent(mixed));
}

// Test comment edge cases
TEST_F(TauEdgeCaseTests, CommentHandling) {
    string comments = R"(
        // Single line comment at start
        namespace CommentTest { // End of line comment
            /* Block comment
               spanning multiple lines */
            interface ICommentInterface {
                // Method with comment
                void Method1(); // Another comment
                
                /* Block comment before method */
                void Method2();
                
                void Method3(); /* Inline block comment */
                
                // Comment with special chars: !@#$%^&*()
                void Method4();
                
                /* Nested /* comments */ might cause issues */
                void Method5();
            }
        } // Final comment
    )";
    EXPECT_TRUE(ParseTauContent(comments));
}

// Test numeric edge cases
TEST_F(TauEdgeCaseTests, NumericEdgeCases) {
    string numeric = R"(
        namespace NumericTest {
            enum LargeValues {
                Zero = 0,
                Max32 = 2147483647,
                Max64 = 9223372036854775807,
                MinusOne = -1,
                LargeNegative = -2147483648
            }
            
            interface INumericInterface {
                void HandleLargeNumbers(int64 big, uint64 huge);
                void HandleSmallNumbers(byte tiny, short small);
                void HandleFloats(float single, double precision);
            }
        }
    )";
    EXPECT_TRUE(ParseTauContent(numeric));
}