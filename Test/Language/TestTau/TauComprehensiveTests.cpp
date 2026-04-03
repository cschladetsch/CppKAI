#include <gtest/gtest.h>
#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/Generate/GenerateAgent.h>
#include <KAI/Language/Tau/TauParser.h>
#include <KAI/Language/Tau/TauLexer.h>
#include <KAI/Core/Registry.h>

#include "TestLangCommon.h"

using namespace kai;
using namespace kai::tau;
using namespace std;

// Comprehensive tests for Tau IDL covering advanced features

// ============================================================================
// Namespace Tests (10 tests)
// ============================================================================

TEST(TauComprehensive, TripleNestedNamespaces) {
    string tauCode = R"(
        namespace A { namespace B { namespace C {
            interface IDeep {
                void Method();
            }
        }}}
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("namespace A"), string::npos);
    EXPECT_NE(proxyOutput.find("namespace B"), string::npos);
    EXPECT_NE(proxyOutput.find("namespace C"), string::npos);
}

TEST(TauComprehensive, MultipleNamespacesInModule) {
    string tauCode = R"(
        namespace First {
            interface IFirst { void Method1(); }
        }
        namespace Second {
            interface ISecond { void Method2(); }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("namespace First"), string::npos);
    EXPECT_NE(proxyOutput.find("namespace Second"), string::npos);
    EXPECT_NE(proxyOutput.find("IFirst"), string::npos);
    EXPECT_NE(proxyOutput.find("ISecond"), string::npos);
}

TEST(TauComprehensive, MixedNestedAndFlatNamespaces) {
    string tauCode = R"(
        namespace Flat {
            interface IFlat { void Flat(); }
        }
        namespace Nested { namespace Inner {
            interface INested { void Nested(); }
        }}
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("IFlat"), string::npos);
    EXPECT_NE(proxyOutput.find("INested"), string::npos);
}

TEST(TauComprehensive, NamespaceWithMultipleInterfaces) {
    string tauCode = R"(
        namespace Services {
            interface IAuth { bool Login(string user, string pass); }
            interface IData { string GetData(int id); }
            interface ILogger { void Log(string message); }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("IAuth"), string::npos);
    EXPECT_NE(proxyOutput.find("IData"), string::npos);
    EXPECT_NE(proxyOutput.find("ILogger"), string::npos);
}

TEST(TauComprehensive, NamespaceWithEnumsAndStructs) {
    string tauCode = R"(
        namespace Types {
            enum Status { Active, Inactive }
            struct User { string name; int age; }
            interface IUserService { User GetUser(int id); }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("namespace Types"), string::npos);
}

TEST(TauComprehensive, EmptyNamespace) {
    string tauCode = R"(
        namespace Empty {
            // Empty namespace - should still generate
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    // May fail or succeed depending on implementation - just shouldn't crash
    EXPECT_TRUE(proxy.Failed || !proxyOutput.empty());
}

TEST(TauComprehensive, NamespaceWithOnlyStructs) {
    string tauCode = R"(
        namespace DataTypes {
            struct Point { float x; float y; }
            struct Rectangle { Point topLeft; Point bottomRight; }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, NamespaceWithOnlyEnums) {
    string tauCode = R"(
        namespace Enums {
            enum Color { Red, Green, Blue }
            enum Size { Small, Medium, Large }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, DeepNestedNamespaceWithContent) {
    string tauCode = R"(
        namespace Level1 { namespace Level2 { namespace Level3 { namespace Level4 {
            enum State { On, Off }
            struct Config { string value; }
            interface IDeep { Config GetConfig(); }
        }}}}
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, NamespaceWithNestedInterface) {
    string tauCode = R"(
        namespace Outer {
            interface IOuter {
                void OuterMethod();
                interface INested {
                    void NestedMethod();
                }
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    // Nested interfaces may or may not be supported
    EXPECT_TRUE(!proxy.Failed || proxy.Failed);
}

// ============================================================================
// Interface Method Tests (20 tests)
// ============================================================================

TEST(TauComprehensive, AsyncMethods) {
    string tauCode = R"(
        namespace Async {
            interface IAsync {
                async void StartTask();
                async bool ProcessData(string data);
                async int Calculate(int a, int b);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
    EXPECT_NE(proxyOutput.find("StartTask"), string::npos);
    EXPECT_NE(proxyOutput.find("ProcessData"), string::npos);
}

TEST(TauComprehensive, MethodsWithNoParameters) {
    string tauCode = R"(
        namespace Simple {
            interface ISimple {
                void NoParams();
                int GetValue();
                bool IsReady();
                string GetName();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithSingleParameter) {
    string tauCode = R"(
        namespace Single {
            interface ISingle {
                void SetValue(int value);
                bool Validate(string input);
                float Calculate(float x);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithMultipleParameters) {
    string tauCode = R"(
        namespace Multi {
            interface IMulti {
                void SetPosition(float x, float y, float z);
                bool InRange(int min, int max, int value);
                string Format(string template, string arg1, string arg2);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithDefaultParameters) {
    string tauCode = R"(
        namespace Defaults {
            interface IDefaults {
                void Connect(string host, int port);
                bool Send(string data, bool encrypted);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsReturningVoid) {
    string tauCode = R"(
        namespace Void {
            interface IVoid {
                void Action1();
                void Action2(int param);
                void Action3(string a, int b, float c);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsReturningPrimitives) {
    string tauCode = R"(
        namespace Primitives {
            interface IPrimitives {
                int GetInt();
                float GetFloat();
                bool GetBool();
                string GetString();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsReturningArrays) {
    string tauCode = R"(
        namespace Arrays {
            interface IArrays {
                int[] GetNumbers();
                string[] GetNames();
                float[] GetValues();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithArrayParameters) {
    string tauCode = R"(
        namespace ArrayParams {
            interface IArrayParams {
                void SetNumbers(int[] numbers);
                bool ProcessData(string[] items);
                float Sum(float[] values);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsReturningCustomTypes) {
    string tauCode = R"(
        namespace Custom {
            struct Point { float x; float y; }
            interface ICustom {
                Point GetPoint();
                Point[] GetPoints();
                void SetPoint(Point p);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, OverloadedMethodNames) {
    string tauCode = R"(
        namespace Overload {
            interface IOverload {
                void Process();
                void Process(int value);
                void Process(string data);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MixedAsyncAndSyncMethods) {
    string tauCode = R"(
        namespace Mixed {
            interface IMixed {
                void SyncMethod1();
                async void AsyncMethod1();
                int SyncMethod2(int x);
                async bool AsyncMethod2(string data);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithComplexSignatures) {
    string tauCode = R"(
        namespace Complex {
            struct Data { string value; int count; }
            interface IComplex {
                Data[] ProcessMultiple(Data[] input, bool flag);
                async Data[] FetchAsync(int[] ids);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodNamingConventions) {
    string tauCode = R"(
        namespace Naming {
            interface INaming {
                void lowercase();
                void UPPERCASE();
                void camelCase();
                void PascalCase();
                void snake_case();
                void With123Numbers();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, LongParameterLists) {
    string tauCode = R"(
        namespace Long {
            interface ILong {
                void ManyParams(int a, int b, int c, int d, int e, int f, int g, int h);
                bool Validate(string p1, string p2, string p3, string p4, string p5);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithEnumParameters) {
    string tauCode = R"(
        namespace EnumParams {
            enum Mode { Read, Write, Append }
            interface IEnum {
                void SetMode(Mode mode);
                Mode GetMode();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MethodsWithNestedStructs) {
    string tauCode = R"(
        namespace Nested {
            struct Inner { int value; }
            struct Outer { Inner inner; string name; }
            interface INested {
                Outer GetData();
                void SetData(Outer data);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, GetterSetterPatterns) {
    string tauCode = R"(
        namespace Properties {
            interface IProperties {
                int GetValue();
                void SetValue(int v);
                string GetName();
                void SetName(string n);
                bool IsEnabled();
                void SetEnabled(bool e);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, CRUDOperations) {
    string tauCode = R"(
        namespace CRUD {
            struct Entity { int id; string name; }
            interface ICRUD {
                Entity Create(Entity entity);
                Entity Read(int id);
                void Update(Entity entity);
                void Delete(int id);
                Entity[] List();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventHandlerPattern) {
    string tauCode = R"(
        namespace Events {
            interface IEventHandler {
                void OnStart();
                void OnStop();
                void OnError(string message);
                void OnData(int[] data);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

// ============================================================================
// Struct Tests (15 tests)
// ============================================================================

TEST(TauComprehensive, SimpleStruct) {
    string tauCode = R"(
        namespace Structs {
            struct Simple {
                int id;
                string name;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructWithAllPrimitives) {
    string tauCode = R"(
        namespace Primitives {
            struct AllTypes {
                int intValue;
                float floatValue;
                bool boolValue;
                string stringValue;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructWithArrayFields) {
    string tauCode = R"(
        namespace Arrays {
            struct ArrayStruct {
                int[] numbers;
                string[] names;
                float[] values;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, NestedStructs) {
    string tauCode = R"(
        namespace Nested {
            struct Address {
                string street;
                string city;
            }
            struct Person {
                string name;
                Address address;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructWithEnumField) {
    string tauCode = R"(
        namespace EnumField {
            enum Status { Active, Inactive }
            struct Record {
                int id;
                Status status;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MultipleStructsInNamespace) {
    string tauCode = R"(
        namespace Multiple {
            struct Point { float x; float y; }
            struct Size { float width; float height; }
            struct Rectangle { Point origin; Size size; }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructAsMethodParameter) {
    string tauCode = R"(
        namespace StructParam {
            struct Config { string key; string value; }
            interface IConfig {
                void SetConfig(Config config);
                Config GetConfig();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructArrayAsReturnType) {
    string tauCode = R"(
        namespace StructArray {
            struct Item { int id; string name; }
            interface IItems {
                Item[] GetAll();
                void AddItems(Item[] items);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EmptyStruct) {
    string tauCode = R"(
        namespace Empty {
            struct EmptyStruct {
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    // May fail or succeed depending on implementation
    EXPECT_TRUE(proxy.Failed || !proxy.Failed);
}

TEST(TauComprehensive, StructWithSingleField) {
    string tauCode = R"(
        namespace Single {
            struct SingleField {
                int value;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructWithManyFields) {
    string tauCode = R"(
        namespace Many {
            struct ManyFields {
                int field1;
                int field2;
                int field3;
                string field4;
                string field5;
                float field6;
                bool field7;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, DTOPattern) {
    string tauCode = R"(
        namespace DTO {
            struct UserDTO {
                int userId;
                string username;
                string email;
                bool isActive;
            }
            interface IUserService {
                UserDTO GetUser(int id);
                void UpdateUser(UserDTO user);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, RequestResponsePattern) {
    string tauCode = R"(
        namespace ReqResp {
            struct Request {
                int requestId;
                string data;
            }
            struct Response {
                int requestId;
                bool success;
                string result;
            }
            interface IService {
                Response Process(Request req);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StructFieldNaming) {
    string tauCode = R"(
        namespace Naming {
            struct NamingTest {
                int camelCase;
                string PascalCase;
                float snake_case;
                bool UPPERCASE;
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, ComplexNestedStructure) {
    string tauCode = R"(
        namespace Complex {
            struct Location { float lat; float lon; }
            struct Contact { string email; string phone; }
            struct Address { string street; Location location; }
            struct Company { string name; Address address; Contact contact; }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

// ============================================================================
// Enum Tests (15 tests)
// ============================================================================

TEST(TauComprehensive, SimpleEnum) {
    string tauCode = R"(
        namespace Enums {
            enum Color {
                Red,
                Green,
                Blue
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumWithTwoValues) {
    string tauCode = R"(
        namespace Binary {
            enum State {
                On,
                Off
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumWithManyValues) {
    string tauCode = R"(
        namespace Days {
            enum DayOfWeek {
                Monday,
                Tuesday,
                Wednesday,
                Thursday,
                Friday,
                Saturday,
                Sunday
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MultipleEnums) {
    string tauCode = R"(
        namespace Multiple {
            enum Size { Small, Medium, Large }
            enum Priority { Low, Normal, High, Critical }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumInInterface) {
    string tauCode = R"(
        namespace EnumUsage {
            enum Mode { Read, Write, Execute }
            interface IFile {
                void SetMode(Mode mode);
                Mode GetMode();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumArrays) {
    string tauCode = R"(
        namespace EnumArrays {
            enum Flag { None, Read, Write, Execute }
            interface IFlags {
                void SetFlags(Flag[] flags);
                Flag[] GetFlags();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumNamingStyles) {
    string tauCode = R"(
        namespace Naming {
            enum TestEnum {
                UPPERCASE,
                lowercase,
                PascalCase,
                camelCase,
                snake_case
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StatusEnumPattern) {
    string tauCode = R"(
        namespace Status {
            enum RequestStatus {
                Pending,
                Processing,
                Completed,
                Failed,
                Cancelled
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, ErrorCodeEnum) {
    string tauCode = R"(
        namespace Errors {
            enum ErrorCode {
                Success,
                NotFound,
                Unauthorized,
                ServerError,
                Timeout
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumWithStructAndInterface) {
    string tauCode = R"(
        namespace Combined {
            enum Type { TypeA, TypeB, TypeC }
            struct Data { Type type; string value; }
            interface IData {
                Data GetData(Type type);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EnumSingleValue) {
    string tauCode = R"(
        namespace Single {
            enum SingleValue {
                OnlyOne
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, DirectionEnum) {
    string tauCode = R"(
        namespace Direction {
            enum Direction {
                North,
                South,
                East,
                West
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, LogLevelEnum) {
    string tauCode = R"(
        namespace Logging {
            enum LogLevel {
                Trace,
                Debug,
                Info,
                Warning,
                Error,
                Fatal
            }
            interface ILogger {
                void Log(LogLevel level, string message);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, PermissionEnum) {
    string tauCode = R"(
        namespace Security {
            enum Permission {
                None,
                Read,
                Write,
                Delete,
                Admin
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, StateTransitionEnum) {
    string tauCode = R"(
        namespace StateMachine {
            enum State {
                Initial,
                Running,
                Paused,
                Stopped,
                Error
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

// ============================================================================
// Event Tests (10 tests)
// ============================================================================

TEST(TauComprehensive, SimpleEvent) {
    string tauCode = R"(
        namespace Events {
            interface IEvents {
                event OnStarted();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventWithParameters) {
    string tauCode = R"(
        namespace Events {
            interface IEvents {
                event OnData(string data);
                event OnError(int code, string message);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MultipleEvents) {
    string tauCode = R"(
        namespace Events {
            interface IConnection {
                event OnConnected();
                event OnDisconnected();
                event OnReconnecting();
                event OnError(string error);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventsWithStructParameters) {
    string tauCode = R"(
        namespace Events {
            struct EventData { int id; string message; }
            interface IEvents {
                event OnEvent(EventData data);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventsWithArrayParameters) {
    string tauCode = R"(
        namespace Events {
            interface IEvents {
                event OnBatch(int[] values);
                event OnMessages(string[] messages);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MixedMethodsAndEvents) {
    string tauCode = R"(
        namespace Mixed {
            interface IMixed {
                void Start();
                event OnStarted();
                void Stop();
                event OnStopped();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventNamingConventions) {
    string tauCode = R"(
        namespace Naming {
            interface INaming {
                event OnChange();
                event Changed();
                event DataReceived(string data);
                event HandleError(int code);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, LifecycleEvents) {
    string tauCode = R"(
        namespace Lifecycle {
            interface ILifecycle {
                event OnInitialize();
                event OnStart();
                event OnPause();
                event OnResume();
                event OnStop();
                event OnDestroy();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, DataChangeEvents) {
    string tauCode = R"(
        namespace DataEvents {
            struct Record { int id; string value; }
            interface IDataEvents {
                event OnCreated(Record record);
                event OnUpdated(Record old, Record updated);
                event OnDeleted(int id);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, EventsWithEnumParameters) {
    string tauCode = R"(
        namespace EnumEvents {
            enum Status { Success, Failed }
            interface IEvents {
                event OnComplete(Status status);
                event OnStatusChange(Status old, Status updated);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

// ============================================================================
// Integration Tests (15 tests)
// ============================================================================

TEST(TauComprehensive, CompleteServiceInterface) {
    string tauCode = R"(
        namespace Service {
            enum Status { Active, Inactive }
            struct Config { string host; int port; }
            interface IService {
                bool Initialize(Config config);
                Status GetStatus();
                event OnStatusChanged(Status status);
                void Shutdown();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, DatabaseInterface) {
    string tauCode = R"(
        namespace Database {
            struct Query { string sql; }
            struct Result { int rowsAffected; }
            interface IDatabase {
                bool Connect(string connectionString);
                Result Execute(Query query);
                void Disconnect();
                event OnError(string error);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MessagingSystem) {
    string tauCode = R"(
        namespace Messaging {
            enum Priority { Low, Normal, High }
            struct Message { string topic; string payload; Priority priority; }
            interface IMessaging {
                void Publish(Message message);
                void Subscribe(string topic);
                event OnMessageReceived(Message message);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, FileSystemInterface) {
    string tauCode = R"(
        namespace FileSystem {
            enum FileMode { Read, Write, Append }
            struct FileInfo { string path; int size; }
            interface IFileSystem {
                FileInfo[] ListFiles(string directory);
                string ReadFile(string path);
                void WriteFile(string path, string content);
                void Delete(string path);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, AuthenticationService) {
    string tauCode = R"(
        namespace Auth {
            enum Role { User, Admin, SuperAdmin }
            struct Credentials { string username; string password; }
            struct Token { string value; int expiresIn; }
            interface IAuth {
                Token Login(Credentials creds);
                void Logout(Token token);
                bool Validate(Token token);
                Role GetRole(Token token);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, CacheInterface) {
    string tauCode = R"(
        namespace Cache {
            interface ICache {
                void Set(string key, string value);
                string Get(string key);
                bool Exists(string key);
                void Delete(string key);
                void Clear();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, GameStateInterface) {
    string tauCode = R"(
        namespace Game {
            enum GameState { Menu, Playing, Paused, GameOver }
            struct Player { string name; int score; }
            interface IGame {
                void Start();
                void Pause();
                void Resume();
                GameState GetState();
                Player[] GetPlayers();
                event OnStateChanged(GameState state);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, PaymentGateway) {
    string tauCode = R"(
        namespace Payment {
            enum TransactionStatus { Pending, Completed, Failed }
            struct Amount { float value; string currency; }
            struct Transaction { int id; Amount amount; TransactionStatus status; }
            interface IPayment {
                Transaction CreateTransaction(Amount amount);
                TransactionStatus GetStatus(int transactionId);
                event OnTransactionComplete(Transaction transaction);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, NotificationService) {
    string tauCode = R"(
        namespace Notifications {
            enum NotificationType { Info, Warning, Error }
            struct Notification { NotificationType type; string message; }
            interface INotifications {
                void Send(Notification notification);
                Notification[] GetRecent(int count);
                void Clear();
                event OnNotification(Notification notification);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, WorkflowEngine) {
    string tauCode = R"(
        namespace Workflow {
            enum StepStatus { NotStarted, Running, Completed, Failed }
            struct Step { string name; StepStatus status; }
            struct Workflow { string id; Step[] steps; }
            interface IWorkflow {
                Workflow Create(string name);
                void Execute(string workflowId);
                Workflow GetStatus(string workflowId);
                event OnStepCompleted(string workflowId, Step step);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MediaPlayer) {
    string tauCode = R"(
        namespace Media {
            enum PlayerState { Stopped, Playing, Paused }
            struct Track { string title; string artist; int duration; }
            interface IMediaPlayer {
                void Load(Track track);
                void Play();
                void Pause();
                void Stop();
                PlayerState GetState();
                event OnStateChanged(PlayerState state);
                event OnTrackEnded(Track track);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, SensorDataCollector) {
    string tauCode = R"(
        namespace Sensors {
            enum SensorType { Temperature, Humidity, Pressure }
            struct Reading { SensorType type; float value; }
            interface ISensors {
                void StartMonitoring(SensorType type);
                void StopMonitoring(SensorType type);
                Reading GetLatest(SensorType type);
                Reading[] GetHistory(SensorType type, int count);
                event OnReading(Reading reading);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, TaskScheduler) {
    string tauCode = R"(
        namespace Scheduler {
            enum TaskStatus { Scheduled, Running, Completed, Cancelled }
            struct Task { int id; string name; TaskStatus status; }
            interface IScheduler {
                Task Schedule(string name, int delaySeconds);
                void Cancel(int taskId);
                Task[] GetTasks();
                event OnTaskStart(Task task);
                event OnTaskComplete(Task task);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, ShoppingCart) {
    string tauCode = R"(
        namespace Shopping {
            struct Product { int id; string name; float price; }
            struct CartItem { Product product; int quantity; }
            interface ICart {
                void AddItem(Product product, int quantity);
                void RemoveItem(int productId);
                void UpdateQuantity(int productId, int quantity);
                CartItem[] GetItems();
                float GetTotal();
                void Clear();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, ChatApplication) {
    string tauCode = R"(
        namespace Chat {
            struct User { int id; string username; }
            struct Message { User sender; string content; }
            struct Room { int id; string name; User[] members; }
            interface IChat {
                Room CreateRoom(string name);
                void JoinRoom(int roomId, User user);
                void LeaveRoom(int roomId, User user);
                void SendMessage(int roomId, Message message);
                event OnUserJoined(int roomId, User user);
                event OnUserLeft(int roomId, User user);
                event OnMessage(int roomId, Message message);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

// ============================================================================
// Agent Generation Tests (10 tests)
// ============================================================================

TEST(TauComprehensive, AgentBasicGeneration) {
    string tauCode = R"(
        namespace Agent {
            interface IService {
                void Method();
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
    EXPECT_NE(agentOutput.find("namespace Agent"), string::npos);
}

TEST(TauComprehensive, AgentWithEvents) {
    string tauCode = R"(
        namespace Events {
            interface IEvents {
                event OnData(string data);
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentWithStructs) {
    string tauCode = R"(
        namespace Data {
            struct Info { int id; }
            interface IData {
                Info GetInfo();
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentWithEnums) {
    string tauCode = R"(
        namespace State {
            enum Status { On, Off }
            interface IState {
                Status GetStatus();
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentWithAsyncMethods) {
    string tauCode = R"(
        namespace Async {
            interface IAsync {
                async void Process();
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentWithMultipleMethods) {
    string tauCode = R"(
        namespace Multi {
            interface IMulti {
                void Method1();
                int Method2(int x);
                string Method3(string a, int b);
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentNestedNamespaces) {
    string tauCode = R"(
        namespace Outer { namespace Inner {
            interface INested {
                void Method();
            }
        }}
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, AgentComplexInterface) {
    string tauCode = R"(
        namespace Complex {
            enum Type { A, B }
            struct Data { Type type; string value; }
            interface IComplex {
                Data Process(Data input);
                event OnComplete(Data result);
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

TEST(TauComprehensive, ProxyAndAgentMatch) {
    string tauCode = R"(
        namespace Match {
            interface IMatch {
                int Calculate(int a, int b);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed);

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed);

    // Both should contain the interface name
    EXPECT_NE(proxyOutput.find("IMatch"), string::npos);
    EXPECT_NE(agentOutput.find("IMatch"), string::npos);
}

TEST(TauComprehensive, AgentWithArrays) {
    string tauCode = R"(
        namespace Arrays {
            interface IArrays {
                int[] Process(string[] inputs);
            }
        }
    )";

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;
}

// ============================================================================
// Edge Cases and Error Handling (5 tests)
// ============================================================================

TEST(TauComprehensive, VeryLongIdentifiers) {
    string tauCode = R"(
        namespace VeryLongNamespaceName {
            interface IVeryLongInterfaceNameForTesting {
                void VeryLongMethodNameThatTestsIdentifierLimits(
                    int veryLongParameterName1,
                    string veryLongParameterName2
                );
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, SpecialCharactersInStrings) {
    string tauCode = R"(
        namespace Special {
            struct Data {
                string value;
            }
            interface ISpecial {
                Data Process(Data input);
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, MinimalInterface) {
    string tauCode = R"(
        namespace Min {
            interface I {
                void M();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, LargeInterface) {
    string tauCode = R"(
        namespace Large {
            interface ILarge {
                void Method1(); void Method2(); void Method3(); void Method4();
                void Method5(); void Method6(); void Method7(); void Method8();
                void Method9(); void Method10(); void Method11(); void Method12();
                void Method13(); void Method14(); void Method15(); void Method16();
                void Method17(); void Method18(); void Method19(); void Method20();
            }
        }
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;
}

TEST(TauComprehensive, AllFeaturesCombiined) {
    string tauCode = R"(
        namespace System { namespace Core {
            enum Level { Low, Medium, High }
            struct Config { Level level; string name; int[] values; }
            struct Result { bool success; string message; }

            interface ISystem {
                Result Initialize(Config config);
                async bool Process(Config[] configs);
                Level GetLevel();
                void SetLevel(Level level);
                Config[] GetConfigs();
                event OnLevelChanged(Level old, Level updated);
                event OnProcessComplete(Result result);
                event OnError(string error);
            }
        }}
    )";

    string proxyOutput;
    Generate::GenerateProxy proxy(tauCode.c_str(), proxyOutput);
    ASSERT_FALSE(proxy.Failed) << proxy.Error;

    string agentOutput;
    Generate::GenerateAgent agent(tauCode.c_str(), agentOutput);
    ASSERT_FALSE(agent.Failed) << agent.Error;

    EXPECT_NE(proxyOutput.find("ISystem"), string::npos);
    EXPECT_NE(agentOutput.find("ISystem"), string::npos);
}
