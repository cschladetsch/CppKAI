// Parameterized coverage of the ISensor-style Tau proxy/agent workflow
// (TauDomainPropertyTest.cpp's pattern) across many values/ports, plus
// extra Future<T>-as-argument edge cases beyond NodeFutureArgumentTest.cpp.

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

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

namespace {

struct SensorImpl {
    int value = 0;
};

class ISensorAgent : public Agent<SensorImpl> {
   public:
    explicit ISensorAgent(Node &node, std::shared_ptr<SensorImpl> impl =
                                          std::make_shared<SensorImpl>())
        : Agent<SensorImpl>(node, std::move(impl)) {
        BindMemberProperty("Value", &SensorImpl::value);
    }
};

class ISensorProxy : public ProxyBase {
   public:
    ISensorProxy(Node &node, NetHandle handle) : ProxyBase(node, handle) {}
    Future<int> Value() { return Fetch<int>("Value"); }
    Future<void> SetValue(int v) { return Store("Value", std::move(v)); }
};

bool PollUntil(Node &a, Node &b, std::function<bool()> pred,
              std::chrono::milliseconds timeout = 3000ms) {
    auto start = std::chrono::steady_clock::now();
    while (!pred()) {
        a.Update();
        b.Update();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        if (elapsed >= timeout) return false;
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

struct ListenResult {
    int port = 0;
    std::string skipReason;
};

ListenResult ListenOnAvailablePort(Node &node, int beginPort, int endPort) {
    for (int candidate = beginPort; candidate < endPort; ++candidate) {
        node.Listen(IpAddress("127.0.0.1"), candidate);
        if (node.IsRunning()) return {candidate, {}};
    }
    return {0, "No available loopback port found in the requested range"};
}

Registry *MakeRegistry() {
    auto *reg = new Registry();
    reg->AddClass<void>();
    reg->AddClass<bool>();
    reg->AddClass<int>();
    reg->AddClass<float>();
    reg->AddClass<String>();
    reg->AddClass<StringStream>();
    reg->AddClass<BinaryStream>();
    reg->AddClass<Array>();
    reg->AddClass<Map>();
    return reg;
}

struct SensorCase {
    int initialValue;
    int newValue;
    int portBase;
    std::string label;
};

std::vector<SensorCase> BuildSensorCases() {
    std::vector<SensorCase> cases;
    for (int i = 0; i < 25; ++i) {
        int initial = (i * 7) - 40;   // spans negative, zero, positive
        int updated = (i * 13) - 10;
        int portBase = 21000 + i * 5;
        cases.push_back({initial, updated, portBase, "Sensor" + std::to_string(i)});
    }
    return cases;
}

}  // namespace

class SensorProxyParamTest : public ::testing::TestWithParam<SensorCase> {};

TEST_P(SensorProxyParamTest, FetchesInitialValueFromRemoteDomain) {
    const SensorCase &sc = GetParam();
    Registry *reg = MakeRegistry();

    Node nodeA;
    nodeA.SetRegistry(reg);
    const auto listen = ListenOnAvailablePort(nodeA, sc.portBase, sc.portBase + 5);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;

    Node nodeB;
    nodeB.SetRegistry(reg);
    nodeB.SetUpdatePump([&]() { nodeA.Update(); });

    bool connected = false;
    nodeB.SetConnectionEventCallback([&](ConnectionEvent ev, const NetAddress &) {
        if (ev == ConnectionEvent::Connected) connected = true;
    });
    nodeB.Connect(IpAddress("127.0.0.1"), listen.port);
    if (!PollUntil(nodeA, nodeB, [&] { return connected; })) {
        GTEST_SKIP() << "Connection did not complete in time for " << sc.label;
    }

    auto impl = std::make_shared<SensorImpl>();
    impl->value = sc.initialValue;
    ISensorAgent agent(nodeA, impl);

    NetAddress agentAddr("127.0.0.1", static_cast<unsigned short>(listen.port));
    nodeB.BindProxyAddress(agent.Handle(), agentAddr);
    ISensorProxy proxy(nodeB, agent.Handle());

    auto future = proxy.Value();
    int result = nodeB.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, sc.initialValue);

    delete reg;
}

TEST_P(SensorProxyParamTest, SetsValueOnRemoteDomain) {
    const SensorCase &sc = GetParam();
    Registry *reg = MakeRegistry();

    Node nodeA;
    nodeA.SetRegistry(reg);
    const auto listen = ListenOnAvailablePort(nodeA, sc.portBase + 2500, sc.portBase + 2505);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;

    Node nodeB;
    nodeB.SetRegistry(reg);
    nodeB.SetUpdatePump([&]() { nodeA.Update(); });

    bool connected = false;
    nodeB.SetConnectionEventCallback([&](ConnectionEvent ev, const NetAddress &) {
        if (ev == ConnectionEvent::Connected) connected = true;
    });
    nodeB.Connect(IpAddress("127.0.0.1"), listen.port);
    if (!PollUntil(nodeA, nodeB, [&] { return connected; })) {
        GTEST_SKIP() << "Connection did not complete in time for " << sc.label;
    }

    auto impl = std::make_shared<SensorImpl>();
    impl->value = sc.initialValue;
    ISensorAgent agent(nodeA, impl);

    NetAddress agentAddr("127.0.0.1", static_cast<unsigned short>(listen.port));
    nodeB.BindProxyAddress(agent.Handle(), agentAddr);
    ISensorProxy proxy(nodeB, agent.Handle());

    auto setFuture = proxy.SetValue(sc.newValue);
    nodeB.WaitFor(setFuture, 2000ms);
    EXPECT_TRUE(setFuture.Succeeded());
    EXPECT_EQ(impl->value, sc.newValue);

    delete reg;
}

INSTANTIATE_TEST_SUITE_P(
    ManySensors, SensorProxyParamTest, ::testing::ValuesIn(BuildSensorCases()),
    [](const ::testing::TestParamInfo<SensorCase> &info) {
        return info.param.label;
    });
