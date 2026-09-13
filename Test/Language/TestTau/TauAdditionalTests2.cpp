// Additional Tau IDL coverage, reusing TauNamespaceTests.cpp's own
// TestLexAndParse() lex+parse-only helper: more namespace/class/field/method
// shape variations than the existing namespace test suite exercises.

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProcess.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Same lex+parse-only fixture as TauNamespaceTests.cpp - kept as its own
// copy (rather than sharing a header) to match how the existing Tau test
// files are each self-contained translation units.
struct TauAdditionalTests2 : TestLangCommon {
    void TestLexAndParse(const std::string& script,
                         const std::string& testName) {
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);

        bool lexerSuccess = lex->Process();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lex->Print());

        if (!lexerSuccess) {
            KAI_LOG_WARNING("Lexer for " + testName + " failed: " + lex->Error);
            FAIL() << "Lexer failed for " << testName << ": " << lex->Error;
            return;
        }

        auto parser = std::make_shared<tau::TauParser>(r);
        parser->Process(lex, Structure::Module);

        if (!parser->Error.empty()) {
            KAI_LOG_WARNING("Parser for " + testName +
                            " reported error: " + parser->Error);
            FAIL() << "Parser failed for " << testName << ": " << parser->Error;
            return;
        }

        SUCCEED() << "Successfully parsed " << testName;
    }
};

TEST_F(TauAdditionalTests2, ClassWithSingleIntField) {
    std::string script = R"(
    namespace Basic {
        class Counter {
            int value;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithSingleIntField");
}

TEST_F(TauAdditionalTests2, ClassWithSingleStringField) {
    std::string script = R"(
    namespace Basic {
        class Label {
            string text;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithSingleStringField");
}

TEST_F(TauAdditionalTests2, ClassWithSingleBoolField) {
    std::string script = R"(
    namespace Basic {
        class Flag {
            bool enabled;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithSingleBoolField");
}

TEST_F(TauAdditionalTests2, ClassWithSingleFloatField) {
    std::string script = R"(
    namespace Basic {
        class Measurement {
            float value;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithSingleFloatField");
}

TEST_F(TauAdditionalTests2, ClassWithMultipleMixedFields) {
    std::string script = R"(
    namespace Basic {
        class Record {
            int id;
            string name;
            bool active;
            float score;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithMultipleMixedFields");
}

TEST_F(TauAdditionalTests2, ClassWithNoArgumentMethod) {
    std::string script = R"(
    namespace Basic {
        class Service {
            void Start();
        }
    }
    )";
    TestLexAndParse(script, "ClassWithNoArgumentMethod");
}

TEST_F(TauAdditionalTests2, ClassWithSingleArgumentMethod) {
    std::string script = R"(
    namespace Basic {
        class Greeter {
            string Greet(string name);
        }
    }
    )";
    TestLexAndParse(script, "ClassWithSingleArgumentMethod");
}

TEST_F(TauAdditionalTests2, ClassWithMultipleArgumentMethod) {
    std::string script = R"(
    namespace Basic {
        class Calculator {
            int Add(int a, int b);
            int Subtract(int a, int b);
        }
    }
    )";
    TestLexAndParse(script, "ClassWithMultipleArgumentMethod");
}

TEST_F(TauAdditionalTests2, ClassWithBoolReturningMethod) {
    std::string script = R"(
    namespace Basic {
        class Validator {
            bool IsValid(string input);
        }
    }
    )";
    TestLexAndParse(script, "ClassWithBoolReturningMethod");
}

TEST_F(TauAdditionalTests2, ClassWithFloatReturningMethod) {
    std::string script = R"(
    namespace Basic {
        class Statistics {
            float Average(int a, int b);
        }
    }
    )";
    TestLexAndParse(script, "ClassWithFloatReturningMethod");
}

TEST_F(TauAdditionalTests2, ClassWithFieldsAndMethods) {
    std::string script = R"(
    namespace Basic {
        class Account {
            int id;
            float balance;
            void Deposit(float amount);
            void Withdraw(float amount);
            float GetBalance();
        }
    }
    )";
    TestLexAndParse(script, "ClassWithFieldsAndMethods");
}

TEST_F(TauAdditionalTests2, TwoUnrelatedClassesInSameNamespace) {
    std::string script = R"(
    namespace Basic {
        class First {
            int a;
        }
        class Second {
            int b;
        }
    }
    )";
    TestLexAndParse(script, "TwoUnrelatedClassesInSameNamespace");
}

TEST_F(TauAdditionalTests2, ClassReferencingAnotherClassAsFieldType) {
    std::string script = R"(
    namespace Basic {
        class Address {
            string city;
        }
        class Person {
            string name;
            Address home;
        }
    }
    )";
    TestLexAndParse(script, "ClassReferencingAnotherClassAsFieldType");
}

TEST_F(TauAdditionalTests2, ClassReferencingAnotherClassAsMethodArgument) {
    std::string script = R"(
    namespace Basic {
        class Item {
            int id;
        }
        class Inventory {
            void AddItem(Item item);
            bool RemoveItem(Item item);
        }
    }
    )";
    TestLexAndParse(script, "ClassReferencingAnotherClassAsMethodArgument");
}

TEST_F(TauAdditionalTests2, DeeplyNestedNamespaces) {
    std::string script = R"(
    namespace A {
        namespace B {
            namespace C {
                class Deep {
                    int value;
                }
            }
        }
    }
    )";
    TestLexAndParse(script, "DeeplyNestedNamespaces");
}

TEST_F(TauAdditionalTests2, ThreeMethodsSameClass) {
    std::string script = R"(
    namespace Basic {
        class Toolbox {
            void First();
            void Second();
            void Third();
        }
    }
    )";
    TestLexAndParse(script, "ThreeMethodsSameClass");
}

TEST_F(TauAdditionalTests2, MethodWithManyIntArguments) {
    std::string script = R"(
    namespace Basic {
        class Grid {
            void SetCell(int row, int col, int value);
        }
    }
    )";
    TestLexAndParse(script, "MethodWithManyIntArguments");
}

TEST_F(TauAdditionalTests2, ClassWithOnlyFields_NoMethods) {
    std::string script = R"(
    namespace Basic {
        class PlainData {
            int a;
            int b;
            int c;
        }
    }
    )";
    TestLexAndParse(script, "ClassWithOnlyFields_NoMethods");
}

TEST_F(TauAdditionalTests2, ClassWithOnlyMethods_NoFields) {
    std::string script = R"(
    namespace Basic {
        class PureService {
            void DoWork();
            int GetStatus();
        }
    }
    )";
    TestLexAndParse(script, "ClassWithOnlyMethods_NoFields");
}

TEST_F(TauAdditionalTests2, ManyClassesInOneNamespace) {
    std::string script = R"(
    namespace Big {
        class A { int x; }
        class B { int x; }
        class C { int x; }
        class D { int x; }
        class E { int x; }
    }
    )";
    TestLexAndParse(script, "ManyClassesInOneNamespace");
}

TEST_F(TauAdditionalTests2, ClassNamesWithNumbers) {
    std::string script = R"(
    namespace Versioned {
        class ServiceV2 {
            int apiVersion;
        }
    }
    )";
    TestLexAndParse(script, "ClassNamesWithNumbers");
}

TEST_F(TauAdditionalTests2, LongDescriptiveClassAndMethodNames) {
    std::string script = R"(
    namespace Descriptive {
        class UserAuthenticationService {
            bool AuthenticateUserWithCredentials(string username, string password);
        }
    }
    )";
    TestLexAndParse(script, "LongDescriptiveClassAndMethodNames");
}

TEST_F(TauAdditionalTests2, SingleCharacterFieldAndMethodNames) {
    std::string script = R"(
    namespace Terse {
        class P {
            int x;
            int y;
            void Set(int a, int b);
        }
    }
    )";
    TestLexAndParse(script, "SingleCharacterFieldAndMethodNames");
}

TEST_F(TauAdditionalTests2, RepeatedFieldTypesAcrossClasses) {
    std::string script = R"(
    namespace Repeated {
        class Point2D {
            float x;
            float y;
        }
        class Point3D {
            float x;
            float y;
            float z;
        }
    }
    )";
    TestLexAndParse(script, "RepeatedFieldTypesAcrossClasses");
}

TEST_F(TauAdditionalTests2, MethodOverloadStyleDifferentArgCounts) {
    std::string script = R"(
    namespace Overload {
        class Printer {
            void Print(string message);
            void PrintWithPrefix(string prefix, string message);
        }
    }
    )";
    TestLexAndParse(script, "MethodOverloadStyleDifferentArgCounts");
}

TEST_F(TauAdditionalTests2, ClassWithBooleanFieldAndPredicateMethod) {
    std::string script = R"(
    namespace Basic {
        class FeatureFlag {
            bool enabled;
            bool IsEnabled();
        }
    }
    )";
    TestLexAndParse(script, "ClassWithBooleanFieldAndPredicateMethod");
}

TEST_F(TauAdditionalTests2, MultipleNamespacesEachWithOneClass) {
    std::string script = R"(
    namespace First {
        class Alpha {
            int value;
        }
    }
    namespace Second {
        class Beta {
            int value;
        }
    }
    namespace Third {
        class Gamma {
            int value;
        }
    }
    )";
    TestLexAndParse(script, "MultipleNamespacesEachWithOneClass");
}

TEST_F(TauAdditionalTests2, ClassWithGetterAndSetterPair) {
    std::string script = R"(
    namespace Basic {
        class Setting {
            int GetValue();
            void SetValue(int value);
        }
    }
    )";
    TestLexAndParse(script, "ClassWithGetterAndSetterPair");
}

TEST_F(TauAdditionalTests2, WideStructOfPrimitiveFields) {
    std::string script = R"(
    namespace Basic {
        class Wide {
            int a;
            int b;
            float c;
            float d;
            string e;
            bool f;
        }
    }
    )";
    TestLexAndParse(script, "WideStructOfPrimitiveFields");
}

TEST_F(TauAdditionalTests2, ClassWithMethodReturningReferencedType) {
    std::string script = R"(
    namespace Basic {
        class Result {
            bool success;
        }
        class Operation {
            Result Execute();
        }
    }
    )";
    TestLexAndParse(script, "ClassWithMethodReturningReferencedType");
}
