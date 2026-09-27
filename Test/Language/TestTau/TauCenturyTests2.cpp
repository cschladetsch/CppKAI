// TauCenturyTests2.cpp
//
// 50 more Tau IDL tests, following on from TauCenturyTests.cpp. This round
// moves beyond parse-success checks into actual generated-code content
// verification (Proxy/Agent class naming, event Register/Unregister handler
// codegen, struct exclusion from RPC codegen, and the `_impl->Method(...)`
// dispatch pattern in generated agents), using the exact patterns already
// confirmed working in TauSeparateGenerationTests.cpp, plus a further batch
// of malformed-input and composed-IDL parse-only tests distinct from the
// first century file's coverage.

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/TauParser.h"

using namespace kai;
using namespace kai::tau;
using namespace std;

namespace {

bool ParseTauContent(const string &content) {
    if (content.find_first_not_of(" \t\r\n") == string::npos) return false;
    try {
        Registry registry;
        auto lexer = std::make_shared<TauLexer>(content.c_str(), registry);
        if (!lexer->Process()) return false;

        auto parser = std::make_shared<TauParser>(registry);
        parser->SetStrictMode(true);
        if (!parser->Process(lexer, Structure::Module)) return false;
        if (!parser->error.empty()) return false;

        auto root = parser->GetRoot();
        return root && !root->GetChildren().empty();
    } catch (const std::exception &) {
        return false;
    }
}

}  // namespace

struct TauCenturyTests2 : ::testing::Test {
    void ExpectParses(const string &code) {
        string proxyOutput;
        Generate::GenerateProxy proxy(code.c_str(), proxyOutput);
        ASSERT_FALSE(proxy.failed) << proxy.error;
    }

    std::string GenProxy(const string &code) {
        string output;
        Generate::GenerateProxy proxy(code.c_str(), output);
        EXPECT_FALSE(proxy.failed) << proxy.error;
        return output;
    }

    std::string GenAgent(const string &code) {
        string output;
        Generate::GenerateAgent agent(code.c_str(), output);
        EXPECT_FALSE(agent.failed) << agent.error;
        return output;
    }

    void ExpectContains(const string &haystack, const string &needle) {
        EXPECT_NE(haystack.find(needle), string::npos)
            << "expected to find \"" << needle << "\"";
    }

    void ExpectExcludes(const string &haystack, const string &needle) {
        EXPECT_EQ(haystack.find(needle), string::npos)
            << "expected NOT to find \"" << needle << "\"";
    }
};

// --------------------------------------------------------------------------
// Proxy class naming for `class` definitions (6)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, ClassGeneratesMatchingProxyName) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Calculator {
                int Add(int a, int b);
            }
        }
    )");
    ExpectContains(proxy, "class CalculatorProxy");
}

TEST_F(TauCenturyTests2, DifferentClassNameGeneratesMatchingProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Logger {
                void Log(string message);
            }
        }
    )");
    ExpectContains(proxy, "class LoggerProxy");
}

TEST_F(TauCenturyTests2, ClassWithMultipleMethodsGeneratesOneProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class FileStore {
                bool Save(string path, string data);
                string Load(string path);
                bool Delete(string path);
            }
        }
    )");
    ExpectContains(proxy, "class FileStoreProxy");
}

TEST_F(TauCenturyTests2, MultipleClassesEachGetOwnProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Alpha {
                int DoAlpha();
            }
            class Beta {
                int DoBeta();
            }
        }
    )");
    ExpectContains(proxy, "class AlphaProxy");
    ExpectContains(proxy, "class BetaProxy");
}

TEST_F(TauCenturyTests2, NestedNamespaceClassStillGetsProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            namespace Services {
                class Auth {
                    bool Login(string user, string pass);
                }
            }
        }
    )");
    ExpectContains(proxy, "class AuthProxy");
}

TEST_F(TauCenturyTests2, ClassWithFutureMethodGetsProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Worker {
                Future<int> Compute(int input);
            }
        }
    )");
    ExpectContains(proxy, "class WorkerProxy");
}

// --------------------------------------------------------------------------
// Proxy class naming for `interface` definitions (6)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, InterfaceGeneratesProxyPreservingIPrefix) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface IStorage {
                bool Put(string key, string value);
            }
        }
    )");
    ExpectContains(proxy, "class IStorageProxy");
}

TEST_F(TauCenturyTests2, InterfaceWithFutureMethodGetsProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface IMath {
                Future<int> Add(int a, int b);
            }
        }
    )");
    ExpectContains(proxy, "class IMathProxy");
}

TEST_F(TauCenturyTests2, MultipleInterfacesEachGetOwnProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface IReader {
                string Read();
            }
            interface IWriter {
                void Write(string data);
            }
        }
    )");
    ExpectContains(proxy, "class IReaderProxy");
    ExpectContains(proxy, "class IWriterProxy");
}

TEST_F(TauCenturyTests2, InheritingInterfaceGetsOwnProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface IBase {
                int BaseMethod();
            }
            interface IExtended : IBase {
                int ExtendedMethod();
            }
        }
    )");
    ExpectContains(proxy, "class IExtendedProxy");
}

TEST_F(TauCenturyTests2, InterfaceAndImplementingClassBothGetProxies) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface INotifier {
                void Notify(string message);
            }
            class EmailNotifier {
                void Notify(string message);
            }
        }
    )");
    ExpectContains(proxy, "class INotifierProxy");
    ExpectContains(proxy, "class EmailNotifierProxy");
}

TEST_F(TauCenturyTests2, InterfaceWithArrayMethodGetsProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface ICatalog {
                string[] ListNames();
            }
        }
    )");
    ExpectContains(proxy, "class ICatalogProxy");
}

// --------------------------------------------------------------------------
// Agent naming (6)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, ClassGeneratesMatchingAgentName) {
    auto agent = GenAgent(R"(
        namespace App {
            class Calculator {
                int Add(int a, int b);
            }
        }
    )");
    ExpectContains(agent, "class CalculatorAgent");
}

TEST_F(TauCenturyTests2, InterfaceGeneratesAgentPreservingIPrefix) {
    auto agent = GenAgent(R"(
        namespace App {
            interface IStorage {
                bool Put(string key, string value);
            }
        }
    )");
    ExpectContains(agent, "class IStorageAgent");
}

TEST_F(TauCenturyTests2, MultipleClassesEachGetOwnAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            class Alpha {
                int DoAlpha();
            }
            class Beta {
                int DoBeta();
            }
        }
    )");
    ExpectContains(agent, "class AlphaAgent");
    ExpectContains(agent, "class BetaAgent");
}

TEST_F(TauCenturyTests2, NestedNamespaceClassStillGetsAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            namespace Services {
                class Auth {
                    bool Login(string user, string pass);
                }
            }
        }
    )");
    ExpectContains(agent, "class AuthAgent");
}

TEST_F(TauCenturyTests2, InterfaceWithFutureMethodGetsAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            interface IMath {
                Future<int> Add(int a, int b);
            }
        }
    )");
    ExpectContains(agent, "class IMathAgent");
}

TEST_F(TauCenturyTests2, InheritingInterfaceGetsOwnAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            interface IBase {
                int BaseMethod();
            }
            interface IExtended : IBase {
                int ExtendedMethod();
            }
        }
    )");
    ExpectContains(agent, "class IExtendedAgent");
}

// --------------------------------------------------------------------------
// Struct exclusion from Proxy/Agent generation (4)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, StructAloneGeneratesNoProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            struct Point {
                float x;
                float y;
            }
        }
    )");
    ExpectExcludes(proxy, "class PointProxy");
}

TEST_F(TauCenturyTests2, StructAloneGeneratesNoAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            struct Point {
                float x;
                float y;
            }
        }
    )");
    ExpectExcludes(agent, "class PointAgent");
}

TEST_F(TauCenturyTests2, StructAlongsideClassStillExcludedFromProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            struct UserData {
                int id;
                string name;
            }
            class UserService {
                UserData GetUser(int id);
            }
        }
    )");
    ExpectContains(proxy, "class UserServiceProxy");
    ExpectExcludes(proxy, "class UserDataProxy");
}

TEST_F(TauCenturyTests2, StructAlongsideClassStillExcludedFromAgent) {
    auto agent = GenAgent(R"(
        namespace App {
            struct UserData {
                int id;
                string name;
            }
            class UserService {
                UserData GetUser(int id);
            }
        }
    )");
    ExpectContains(agent, "class UserServiceAgent");
    ExpectExcludes(agent, "class UserDataAgent");
}

// --------------------------------------------------------------------------
// Event handler codegen (6)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, SingleEventGeneratesRegisterAndUnregister) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Sensor {
                event OnReading(float value);
            }
        }
    )");
    ExpectContains(proxy, "RegisterOnReadingHandler");
    ExpectContains(proxy, "UnregisterOnReadingHandler");
}

TEST_F(TauCenturyTests2, MultipleEventsEachGetHandlers) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Connection {
                event OnOpen();
                event OnClose();
                event OnError(string message);
            }
        }
    )");
    ExpectContains(proxy, "RegisterOnOpenHandler");
    ExpectContains(proxy, "RegisterOnCloseHandler");
    ExpectContains(proxy, "RegisterOnErrorHandler");
}

TEST_F(TauCenturyTests2, InterfaceEventsAlsoGetHandlers) {
    auto proxy = GenProxy(R"(
        namespace App {
            interface IObservable {
                event OnChanged(int newValue);
            }
        }
    )");
    ExpectContains(proxy, "RegisterOnChangedHandler");
    ExpectContains(proxy, "UnregisterOnChangedHandler");
}

TEST_F(TauCenturyTests2, EventWithNoParametersGetsHandlers) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Timer {
                event OnTick();
            }
        }
    )");
    ExpectContains(proxy, "RegisterOnTickHandler");
    ExpectContains(proxy, "UnregisterOnTickHandler");
}

TEST_F(TauCenturyTests2, EventWithMultipleParametersGetsHandlers) {
    auto proxy = GenProxy(R"(
        namespace App {
            class ChatRoom {
                event OnMessage(int userId, string text, int64 timestamp);
            }
        }
    )");
    ExpectContains(proxy, "RegisterOnMessageHandler");
    ExpectContains(proxy, "UnregisterOnMessageHandler");
}

TEST_F(TauCenturyTests2, MethodsAndEventsCoexistInSameProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Uploader {
                bool StartUpload(string path);
                event OnProgress(int percent);
                event OnComplete();
            }
        }
    )");
    ExpectContains(proxy, "class UploaderProxy");
    ExpectContains(proxy, "RegisterOnProgressHandler");
    ExpectContains(proxy, "RegisterOnCompleteHandler");
}

// --------------------------------------------------------------------------
// Agent dispatch pattern (`_impl->Method(args)`) (4)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, AgentDispatchesSingleArgMethodToImpl) {
    auto agent = GenAgent(R"(
        namespace App {
            class Greeter {
                string Greet(string name);
            }
        }
    )");
    ExpectContains(agent, "_impl->Greet(name)");
}

TEST_F(TauCenturyTests2, AgentDispatchesMultiArgMethodToImpl) {
    auto agent = GenAgent(R"(
        namespace App {
            class Calculator {
                int Add(int a, int b);
            }
        }
    )");
    ExpectContains(agent, "_impl->Add(a, b)");
}

TEST_F(TauCenturyTests2, AgentDispatchesEachMethodOfMultiMethodClass) {
    auto agent = GenAgent(R"(
        namespace App {
            class Repository {
                bool Save(int id, string data);
                string Load(int id);
                bool Remove(int id);
            }
        }
    )");
    ExpectContains(agent, "_impl->Save(id, data)");
    ExpectContains(agent, "_impl->Load(id)");
    ExpectContains(agent, "_impl->Remove(id)");
}

TEST_F(TauCenturyTests2, AgentDispatchesNoArgMethodToImpl) {
    auto agent = GenAgent(R"(
        namespace App {
            class Counter {
                int GetValue();
            }
        }
    )");
    ExpectContains(agent, "_impl->GetValue()");
}

// --------------------------------------------------------------------------
// Further malformed-input rejection, distinct from TauCenturyTests.cpp (10)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, RejectsUnterminatedComment) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            /* never closed
            struct Data { int id; }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsClassMissingClosingBrace) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            class Foo {
                int Add(int a, int b);
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsEnumMissingClosingBrace) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            enum Status {
                Active, Inactive
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsNumberAsClassName) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            class 123Bad {
                int id;
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsNestedNamespaceMissingInnerClose) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Outer {
            namespace Inner {
                struct Data { int id; }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsFieldWithUnknownBracketedType) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct Data {
                int[5] fixedArray;
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsDoubleCommaInEventParams) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            interface IBroken {
                event OnSomething(int a,, int b);
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsClassMissingName) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            class {
                int value;
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsEventMissingParens) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            interface IBroken {
                event OnSomething;
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, RejectsMethodMissingSemicolonInClass) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            class Foo {
                int Add(int a, int b)
            }
        }
    )"));
}

// --------------------------------------------------------------------------
// Further composed IDL parse-only tests (4)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, MixedClassInterfaceStructEnumInOneNamespace) {
    ExpectParses(R"(
        namespace Mixed {
            enum Priority { Low, Medium, High }
            struct Task {
                int id;
                string title;
                Priority priority;
            }
            interface ITaskQueue {
                Future<bool> Enqueue(Task task);
                Future<Task> Dequeue();
            }
            class InMemoryTaskQueue {
                Future<bool> Enqueue(Task task);
                Future<Task> Dequeue();
                event TaskAdded(int id);
            }
        }
    )");
}

TEST_F(TauCenturyTests2, ClassImplementingInterfaceWithSharedEventName) {
    ExpectParses(R"(
        namespace Shared {
            interface IPublisher {
                event OnPublish(string topic);
            }
            class Broker {
                event OnPublish(string topic);
                Future<bool> Publish(string topic, string payload);
            }
        }
    )");
}

TEST_F(TauCenturyTests2, DeeplyNestedNamespaceWithClassAndEvents) {
    ExpectParses(R"(
        namespace A {
            namespace B {
                namespace C {
                    class Beacon {
                        event OnPing(int sequence);
                        Future<bool> Ping();
                    }
                }
            }
        }
    )");
}

TEST_F(TauCenturyTests2, LargeMixedFileWithManyTopLevelDeclarations) {
    ExpectParses(R"(
        namespace Large {
            enum Kind { A, B, C }
            struct Small { int v; }
            struct Bigger { Small a; Small b; Kind kind; }
            interface IOne { Future<int> One(); }
            interface ITwo { Future<int> Two(); }
            class ImplOne { Future<int> One(); }
            class ImplTwo { Future<int> Two(); event OnDone(); }
        }
    )");
}

// --------------------------------------------------------------------------
// Extra coverage to round out the century (4)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests2, ClassWithConstFieldStillGeneratesProxy) {
    auto proxy = GenProxy(R"(
        namespace App {
            class Config {
                const int MaxRetries = 3;
                bool Load(string path);
            }
        }
    )");
    ExpectContains(proxy, "class ConfigProxy");
}

TEST_F(TauCenturyTests2, AgentDispatchesMethodReturningFutureToImpl) {
    auto agent = GenAgent(R"(
        namespace App {
            class Downloader {
                Future<bool> Fetch(string url);
            }
        }
    )");
    ExpectContains(agent, "_impl->Fetch(url)");
}

TEST_F(TauCenturyTests2, RejectsStructMissingName) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct {
                int value;
            }
        }
    )"));
}

TEST_F(TauCenturyTests2, InterfaceWithConstAndEventAndMethodParsesTogether) {
    ExpectParses(R"(
        namespace App {
            interface IThermostat {
                const int DefaultTarget = 20;
                event OnTemperatureChanged(float degrees);
                Future<bool> SetTarget(int degrees);
                Future<float> GetCurrent();
            }
        }
    )");
}
