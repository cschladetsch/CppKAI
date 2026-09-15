#include <gtest/gtest.h>

#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Regression coverage for Future<T> handling in the code generators.
//
// History: a Future<T> method PARAMETER parsed fine (the lexer glues
// "Future<T>" into a single Ident token, so it looks like any other type
// name to the parser), but GenerateProxy/GenerateAgent generated code
// against a Node API that never actually existed (_node->SendWithResponseAsync,
// hand-written Handle_MethodName(BinaryStream&, NetAddress&) handlers, etc.),
// so nothing here ever compiled.
//
// The generators were rewritten to target the real, compiled RPC path:
// ProxyBase::Exec<Ty>(name, args...) -> Node::Invoke<R,Args...> on the proxy
// side, and Node::RegisterMethod<R,Args...>/RegisterProperty<T> on the agent
// side. Node::Invoke's own argument packing (Node::PackInvokeArg) already
// knows how to send a Future<T> argument whether it is resolved or still
// pending, and the receiving side's MethodInvoker::ExtractArg reconstructs
// it - so a Future<T> PARAMETER needs *no* special handling in the
// generated code at all: it is passed straight through like any other
// argument. Only a Future<T> RETURN type needs special handling, because
// Exec<T>/RegisterMethod<T,...> already wrap the result in a real Future<T>,
// so an explicitly-spelled "Future<T>" return type must be unwrapped to T
// first or it would double-wrap into Future<Future<T>>.
//
// These tests inspect the actual generated proxy/agent source text rather
// than just checking that the IDL parses, since parsing was never the part
// that was broken.
struct TauFutureArgumentCodeGenTests : TestLangCommon {
    string GenerateProxyCode(const string& idl) {
        string output;
        tau::Generate::GenerateProxy proxy(idl.c_str(), output);
        EXPECT_FALSE(proxy.Failed) << proxy.Error;
        return output;
    }

    string GenerateAgentCode(const string& idl) {
        string output;
        tau::Generate::GenerateAgent agent(idl.c_str(), output);
        EXPECT_FALSE(agent.Failed) << agent.Error;
        return output;
    }
};

TEST_F(TauFutureArgumentCodeGenTests, ProxyPassesFutureArgumentStraightThrough) {
    const string idl = R"(
        namespace Test {
            interface IAggregator {
                int Sum(Future<int> a, Future<int> b);
            }
        }
    )";

    string proxy = GenerateProxyCode(idl);
    ASSERT_FALSE(proxy.empty());

    // The Future<T> argument is forwarded unchanged into Exec<T> - Node::Invoke
    // (via PackInvokeArg) is what knows how to encode a resolved-or-pending
    // Future<T>, not the generated proxy code.
    EXPECT_NE(proxy.find("Exec<int>(\"Sum\", a, b)"), string::npos)
        << "Proxy should forward Future<T> arguments straight into Exec<T>";
    EXPECT_NE(proxy.find("const Future<int>& a"), string::npos)
        << "Proxy method should declare the parameter as a Future<T> "
           "reference";
    EXPECT_NE(proxy.find("const Future<int>& b"), string::npos);

    // The method itself still returns a real Future<int>.
    EXPECT_NE(proxy.find("Future<int> Sum("), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, ProxyUnwrapsExplicitFutureReturnType) {
    const string idl = R"(
        namespace Test {
            interface IEcho {
                Future<string> Echo(string message);
            }
        }
    )";

    string proxy = GenerateProxyCode(idl);
    ASSERT_FALSE(proxy.empty());

    // Exec<T> already returns Future<T>; an IDL-declared "Future<string>"
    // return type must be unwrapped to "string" first, or this would
    // double-wrap into Future<Future<string>>.
    EXPECT_NE(proxy.find("Future<string> Echo("), string::npos)
        << "Declared return type should stay Future<string>, not "
           "Future<Future<string>>";
    EXPECT_NE(proxy.find("Exec<string>(\"Echo\""), string::npos)
        << "Exec<T> should be called with the unwrapped inner type";
    EXPECT_EQ(proxy.find("Future<Future<string>>"), string::npos);
    EXPECT_EQ(proxy.find("Exec<Future<string>>"), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, AgentRegistersFutureArgumentMethodDirectly) {
    const string idl = R"(
        namespace Test {
            interface IAggregator {
                int Sum(Future<int> a, Future<int> b);
            }
        }
    )";

    string agent = GenerateAgentCode(idl);
    ASSERT_FALSE(agent.empty());

    // RegisterMethod's template arguments (and the std::function/lambda
    // parameter types) name Future<int> directly for a/b - Node's dispatch
    // (MethodInvoker::ExtractArg) is what reconstructs a real Future<int>
    // from the wire encoding, not any code the generator emits.
    EXPECT_NE(agent.find("GetNode().RegisterMethod<int, Future<int>, Future<int>>("),
              string::npos)
        << "Agent should register the method with Future<int> argument types "
           "unchanged";
    EXPECT_NE(agent.find("std::function<int(Future<int>, Future<int>)>"),
              string::npos);
    EXPECT_NE(agent.find("_impl->Sum(a, b)"), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, AgentUnwrapsExplicitFutureReturnType) {
    const string idl = R"(
        namespace Test {
            interface IEcho {
                Future<string> Echo(string message);
            }
        }
    )";

    string agent = GenerateAgentCode(idl);
    ASSERT_FALSE(agent.empty());

    // The implementation (_impl) is expected to be a plain, synchronous
    // method returning "string", not Future<string> - the Future<T> wrapping
    // is entirely the RPC layer's job (Exec<T>/RegisterMethod<T,...>).
    EXPECT_NE(agent.find("GetNode().RegisterMethod<string, string>("),
              string::npos)
        << "Agent should register with the unwrapped return type";
    EXPECT_NE(agent.find("std::function<string(string)>"), string::npos);
    EXPECT_EQ(agent.find("RegisterMethod<Future<string>"), string::npos);
    EXPECT_EQ(agent.find("std::function<Future<string>"), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, FutureVoidArgumentAndReturnBothWork) {
    const string idl = R"(
        namespace Test {
            interface ITrigger {
                void Fire(Future<void> onReady);
            }
        }
    )";

    string proxy = GenerateProxyCode(idl);
    string agent = GenerateAgentCode(idl);
    ASSERT_FALSE(proxy.empty());
    ASSERT_FALSE(agent.empty());

    EXPECT_NE(proxy.find("Future<void> Fire("), string::npos);
    EXPECT_NE(proxy.find("Exec<void>(\"Fire\", onReady)"), string::npos);
    EXPECT_NE(agent.find("GetNode().RegisterMethod<void, Future<void>>("),
              string::npos);
    EXPECT_NE(agent.find("_impl->Fire(onReady)"), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, MixedPlainAndFutureArgumentsBothWork) {
    const string idl = R"(
        namespace Test {
            interface IConditionalRelay {
                int RelayIf(bool enabled, Future<int> value);
            }
        }
    )";

    string proxy = GenerateProxyCode(idl);
    string agent = GenerateAgentCode(idl);
    ASSERT_FALSE(proxy.empty());
    ASSERT_FALSE(agent.empty());

    // Plain and Future arguments are treated identically by the generator -
    // both are just forwarded through Exec<T> / RegisterMethod<T,...>.
    EXPECT_NE(proxy.find("Exec<int>(\"RelayIf\", enabled, value)"),
              string::npos);
    EXPECT_NE(agent.find("GetNode().RegisterMethod<int, bool, Future<int>>("),
              string::npos);
    EXPECT_NE(agent.find("_impl->RelayIf(enabled, value)"), string::npos);
}

TEST_F(TauFutureArgumentCodeGenTests, PropertyWithExplicitFutureTypeIsUnwrapped) {
    const string idl = R"(
        namespace Test {
            interface ICounter {
                Future<int> Count;
            }
        }
    )";

    string proxy = GenerateProxyCode(idl);
    string agent = GenerateAgentCode(idl);
    ASSERT_FALSE(proxy.empty());
    ASSERT_FALSE(agent.empty());

    // Fetch<T>/RegisterProperty<T> already wrap the property value in a
    // Future<T>, so an explicit "Future<int>" property type must likewise be
    // unwrapped on both sides.
    EXPECT_NE(proxy.find("Future<int> Count()"), string::npos);
    EXPECT_NE(proxy.find("Fetch<int>(\"Count\")"), string::npos);
    EXPECT_NE(agent.find("RegisterProperty<int>("), string::npos);
    EXPECT_EQ(proxy.find("Fetch<Future<int>>"), string::npos);
    EXPECT_EQ(agent.find("RegisterProperty<Future<int>>"), string::npos);
}
