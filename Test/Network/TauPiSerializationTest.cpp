#include <gtest/gtest.h>

#include <cstdint>
#include <chrono>
#include <memory>
#include <string>

#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Object/Label.h"
#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Network/Address.h"
#include "KAI/Network/Agent.h"
#include "KAI/Network/Node.h"
#include "KAI/Network/ProxyBase.h"

using namespace kai;
using namespace kai::net;

namespace {
void RegisterMinimalTypes(Registry &reg) {
    reg.AddClass<int>();
    String::Register(reg);
    Array::Register(reg);
    BinaryStream::Register(reg);
    Label::Register(reg);
}

class Calculator {
   public:
    int Add(int a, int b) { return a + b; }
};

class ICalcAgent : public Agent<Calculator> {
   public:
    explicit ICalcAgent(Node &node)
        : Agent<Calculator>(node, std::make_shared<Calculator>()) {
        BindMethod("Add", &Calculator::Add);
    }
};

class ICalcProxy : public ProxyBase {
   public:
    using ProxyBase::StreamType;
    ICalcProxy(Node &node, NetHandle handle) : ProxyBase(node, handle) {}

    Future<int> Add(int a, int b) { return Exec<int>("Add", a, b); }
};

struct ListenResult {
    int port = 0;
    std::string skipReason;
};

static ListenResult ListenOnAvailablePort(Node &node, int beginPort,
                                          int endPort) {
    for (int candidate = beginPort; candidate < endPort; ++candidate) {
        node.Listen(IpAddress("127.0.0.1"), candidate);
        if (node.IsRunning()) {
            return {candidate, {}};
        }
    }

    return {0, "No available loopback port found in the requested range"};
}
}  // namespace

TEST(TauPiSerializationTest, LocalNodeRoundTrip) {
    const std::string tauScript = R"(
        namespace Test {
            interface ICalc {
                Future<int> Add(int a, int b);
            }
        }
    )";

    Registry registry;
    RegisterMinimalTypes(registry);

    Node nodeA;
    Node nodeB;
    nodeA.SetRegistry(&registry);
    nodeB.SetRegistry(&registry);

    const auto listen = ListenOnAvailablePort(nodeA, 20000, 20100);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;
    nodeB.Connect(IpAddress("127.0.0.1"), port);

    nodeA.SetUpdatePump([&]() { nodeB.Update(); });
    nodeB.SetUpdatePump([&]() { nodeA.Update(); });

    const auto start = std::chrono::steady_clock::now();
    while (nodeA.GetConnectionCount() == 0 ||
           nodeB.GetConnectionCount() == 0) {
        nodeA.Update();
        nodeB.Update();
        if (std::chrono::steady_clock::now() - start >
            std::chrono::seconds(2)) {
            FAIL() << "Timed out waiting for node connection";
        }
    }

    std::string proxyOutput;
    tau::Generate::GenerateProxy proxyGen(tauScript.c_str(), proxyOutput);
    ASSERT_FALSE(proxyGen.Failed);
    ASSERT_NE(proxyOutput.find("ICalcProxy"), std::string::npos);

    std::string agentOutput;
    tau::Generate::GenerateAgent agentGen(tauScript.c_str(), agentOutput);
    ASSERT_FALSE(agentGen.Failed);
    ASSERT_NE(agentOutput.find("ICalcAgent"), std::string::npos);

    ICalcAgent agent(nodeA);
    ICalcProxy proxy(nodeB, agent.Handle());

    auto future = proxy.Add(2, 3);
    int result = nodeB.WaitFor(future);
    EXPECT_EQ(result, 5);
}
