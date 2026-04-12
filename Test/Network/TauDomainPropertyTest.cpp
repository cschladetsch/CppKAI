// Tests the Tau IDL -> Agent/Proxy workflow across two named domains.
// Domain A hosts an ISensorAgent that exposes a single integer property.
// Domain B holds an ISensorProxy and fetches that property value over the network.

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Core/Tree.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Network/Agent.h"
#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Domain.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/Node.h"
#include "KAI/Network/ProxyBase.h"

using namespace kai;
using namespace kai::net;
using namespace std::chrono_literals;

// The .tau IDL defining a sensor interface with a single read/write property.
static const std::string kSensorTau = R"(
    namespace Sensor {
        interface ISensor {
            int Value;
        }
    }
)";

namespace {

// Servant class - lives in Domain A and holds the actual data.
struct SensorImpl {
    int value = 42;
};

// Agent in Domain A: registers SensorImpl.value as a network-accessible property.
class ISensorAgent : public Agent<SensorImpl> {
   public:
    explicit ISensorAgent(Node &node,
                          std::shared_ptr<SensorImpl> impl = std::make_shared<SensorImpl>())
        : Agent<SensorImpl>(node, std::move(impl)) {
        BindMemberProperty("Value", &SensorImpl::value);
    }
};

// Proxy in Domain B: provides typed access to the remote ISensorAgent.
class ISensorProxy : public ProxyBase {
   public:
    ISensorProxy(Node &node, NetHandle handle) : ProxyBase(node, handle) {}

    Future<int> Value() { return Fetch<int>("Value"); }
    Future<void> SetValue(int v) { return Store("Value", std::move(v)); }
};

// Pump both nodes until pred() becomes true or the timeout expires.
static bool PollUntil(Node &a, Node &b, std::function<bool()> pred,
                      std::chrono::milliseconds timeout = 3000ms) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!pred()) {
        a.Update();
        b.Update();
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

}  // namespace

// Verify that the .tau IDL is parsed and generates the expected class names.
TEST(TauDomainPropertyTest, IdlGeneratesExpectedClassNames) {
    std::string proxyOut;
    tau::Generate::GenerateProxy proxyGen(kSensorTau.c_str(), proxyOut);
    ASSERT_FALSE(proxyGen.Failed) << "Proxy generation failed: " << proxyGen.Error;
    EXPECT_NE(proxyOut.find("ISensorProxy"), std::string::npos)
        << "Generated proxy should contain ISensorProxy:\n" << proxyOut;

    std::string agentOut;
    tau::Generate::GenerateAgent agentGen(kSensorTau.c_str(), agentOut);
    ASSERT_FALSE(agentGen.Failed) << "Agent generation failed: " << agentGen.Error;
    EXPECT_NE(agentOut.find("ISensorAgent"), std::string::npos)
        << "Generated agent should contain ISensorAgent:\n" << agentOut;
}

// Domain A hosts the agent; Domain B connects and fetches the property value.
TEST(TauDomainPropertyTest, DomainBProxyFetchesPropertyFromDomainA) {
    Registry registry;
    registry.AddClass<void>();
    registry.AddClass<bool>();
    registry.AddClass<int>();
    registry.AddClass<float>();
    registry.AddClass<String>();
    registry.AddClass<StringStream>();
    registry.AddClass<BinaryStream>();
    registry.AddClass<Array>();
    registry.AddClass<Map>();

    // Domain A: the agent lives here.
    Node nodeA;
    nodeA.SetRegistry(&registry);

    int port = 0;
    for (int candidate = 16100; candidate < 16200; ++candidate) {
        nodeA.Listen(IpAddress("127.0.0.1"), candidate);
        if (nodeA.IsRunning()) {
            port = candidate;
            break;
        }
    }
    if (port == 0) GTEST_SKIP() << "Local networking is unavailable in this environment";

    // Domain B: the proxy lives here. Pump Domain A whenever Domain B ticks.
    Node nodeB;
    nodeB.SetRegistry(&registry);
    nodeB.SetUpdatePump([&]() { nodeA.Update(); });

    bool connected = false;
    nodeB.SetConnectionEventCallback([&](ConnectionEvent ev, const NetAddress &) {
        if (ev == ConnectionEvent::Connected) connected = true;
    });

    nodeB.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(nodeA, nodeB, [&] { return connected; });
    if (!ok) GTEST_SKIP() << "Domain B did not connect to Domain A within timeout";

    // Create the agent in Domain A. Initial property value is 42.
    ISensorAgent agent(nodeA);

    // Tell Domain B where the agent lives so it can route requests.
    NetAddress agentAddr("127.0.0.1", static_cast<unsigned short>(port));
    nodeB.BindProxyAddress(agent.Handle(), agentAddr);

    ISensorProxy proxy(nodeB, agent.Handle());

    // Domain B fetches the property from Domain A.
    auto future = proxy.Value();
    int result = nodeB.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, 42);
}

// Domain B can also write a property back to Domain A.
TEST(TauDomainPropertyTest, DomainBProxySetsPropertyOnDomainA) {
    Registry registry;
    registry.AddClass<void>();
    registry.AddClass<bool>();
    registry.AddClass<int>();
    registry.AddClass<float>();
    registry.AddClass<String>();
    registry.AddClass<StringStream>();
    registry.AddClass<BinaryStream>();
    registry.AddClass<Array>();
    registry.AddClass<Map>();

    Node nodeA;
    nodeA.SetRegistry(&registry);

    int port = 0;
    for (int candidate = 16200; candidate < 16300; ++candidate) {
        nodeA.Listen(IpAddress("127.0.0.1"), candidate);
        if (nodeA.IsRunning()) {
            port = candidate;
            break;
        }
    }
    if (port == 0) GTEST_SKIP() << "Local networking is unavailable in this environment";

    Node nodeB;
    nodeB.SetRegistry(&registry);
    nodeB.SetUpdatePump([&]() { nodeA.Update(); });

    bool connected = false;
    nodeB.SetConnectionEventCallback([&](ConnectionEvent ev, const NetAddress &) {
        if (ev == ConnectionEvent::Connected) connected = true;
    });

    nodeB.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(nodeA, nodeB, [&] { return connected; });
    if (!ok) GTEST_SKIP() << "Domain B did not connect to Domain A within timeout";

    // Create agent with default value 42, then Domain B overwrites it.
    ISensorAgent agent(nodeA);

    NetAddress agentAddr("127.0.0.1", static_cast<unsigned short>(port));
    nodeB.BindProxyAddress(agent.Handle(), agentAddr);
    ISensorProxy proxy(nodeB, agent.Handle());

    // Set the property from Domain B to 99.
    auto setFuture = proxy.SetValue(99);
    nodeB.WaitFor(setFuture, 2000ms);
    EXPECT_TRUE(setFuture.Succeeded());

    // Verify Domain A sees the updated value.
    EXPECT_EQ(agent.Instance().value, 99);
}
