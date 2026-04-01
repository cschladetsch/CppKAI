#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <thread>

#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Core/Tree.h"
#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Node.h"

using namespace kai;
using namespace kai::net;
using namespace std::chrono_literals;

static constexpr int kBasePort = 15600;

// Pump both nodes until predicate is true or timeout expires.
static bool PollUntil(Node &a, Node &b, std::function<bool()> pred,
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

class NodeEndToEndTest : public ::testing::Test {
   protected:
    Registry *reg_ = nullptr;
    Tree *tree_ = nullptr;

    void SetUp() override {
        reg_ = new Registry();
        reg_->AddClass<void>();
        reg_->AddClass<bool>();
        reg_->AddClass<int>();
        reg_->AddClass<float>();
        reg_->AddClass<String>();
        reg_->AddClass<StringStream>();
        reg_->AddClass<BinaryStream>();
        reg_->AddClass<Array>();
        reg_->AddClass<Map>();

        tree_ = new Tree();
        Object root = reg_->New<void>();
        tree_->SetRoot(root);
        reg_->SetTree(*tree_);
    }

    void TearDown() override {
        delete tree_;
        delete reg_;
    }
};

// Two nodes in the same process: client calls Add(3,4) on server agent,
// expects 7 back via Future.
TEST_F(NodeEndToEndTest, RemoteMethodCallReturnsCorrectValue) {
    const int port = kBasePort;

    Node server;
    server.SetRegistry(reg_);
    server.Listen(port);

    // Register an Add method on the server.
    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterMethod<int, int, int>(
        agentHandle, "Add",
        std::function<int(int, int)>([](int a, int b) { return a + b; }));

    Node client;
    client.SetRegistry(reg_);

    // Pump the server whenever the client's WaitFor loop ticks.
    client.SetUpdatePump([&server]() { server.Update(); });

    bool connected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) connected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client, [&] { return connected; });
    ASSERT_TRUE(ok) << "Client did not connect within timeout";

    // Tell the client which peer holds this agent handle.
    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    auto future = client.Invoke<int>(agentHandle, "Add", 3, 4);
    int result = client.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, 7);
}

// Server broadcasts a named event; client subscriber receives it.
TEST_F(NodeEndToEndTest, EventBroadcastReachesSubscriber) {
    const int port = kBasePort + 1;

    Node server;
    server.SetRegistry(reg_);
    server.Listen(port);

    Node client;
    client.SetRegistry(reg_);
    client.SetUpdatePump([&server]() { server.Update(); });

    bool clientConnected = false;
    bool serverConnected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) clientConnected = true;
        });
    server.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) serverConnected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client,
                        [&] { return clientConnected && serverConnected; });
    ASSERT_TRUE(ok) << "Nodes did not both connect within timeout";

    bool eventReceived = false;
    client.SubscribeEvent("Ping", [&](BinaryPacket &) {
        eventReceived = true;
    });

    server.BroadcastEvent("Ping");

    ok = PollUntil(server, client, [&] { return eventReceived; });
    EXPECT_TRUE(ok) << "Event was not received within timeout";
}

// Server broadcasts a KAI object; client object-message subscriber receives it.
TEST_F(NodeEndToEndTest, ObjectMessageReachesSubscriber) {
    const int port = kBasePort + 2;

    Node server;
    server.SetRegistry(reg_);
    server.Listen(port);

    Node client;
    client.SetRegistry(reg_);
    client.SetUpdatePump([&server]() { server.Update(); });

    bool clientConnected = false;
    bool serverConnected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) clientConnected = true;
        });
    server.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) serverConnected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client,
                        [&] { return clientConnected && serverConnected; });
    ASSERT_TRUE(ok) << "Nodes did not both connect within timeout";

    int received = 0;
    client.SubscribeObjectMessage([&](const Object &obj) {
        if (obj.Exists() && obj.IsType<int>())
            received = ConstDeref<int>(obj);
    });

    Object payload = reg_->New<int>(42);
    server.SendObject(payload);

    ok = PollUntil(server, client, [&] { return received != 0; });
    EXPECT_TRUE(ok) << "Object message was not received within timeout";
    EXPECT_EQ(received, 42);
}
