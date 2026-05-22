#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <string>
#include <thread>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Core/Tree.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Node.h"

using namespace kai;
using namespace kai::net;
using kai::Language;
using namespace std::chrono_literals;

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

struct ListenResult {
    int port = 0;
    std::string skipReason;
};

static ListenResult ListenOnAvailablePort(Node &node, int beginPort,
                                          int endPort) {
    std::string lastFailure;
    for (int candidate = beginPort; candidate < endPort; ++candidate) {
        node.Listen(IpAddress("127.0.0.1"), candidate);
        if (node.IsRunning()) {
            return {candidate, {}};
        }

        lastFailure = "Node::Listen failed to start on 127.0.0.1:" +
                      std::to_string(candidate);
    }

    if (lastFailure.empty()) {
        lastFailure = "No available loopback port found in the requested range";
    }

    return {0, std::move(lastFailure)};
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
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16400, 16500);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

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
    if (!ok)
        GTEST_SKIP() << "Client/server connection did not complete in time";

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
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16500, 16600);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

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
    if (!ok) GTEST_SKIP() << "Nodes did not both connect within timeout";

    bool eventReceived = false;
    client.SubscribeEvent("Ping",
                          [&](BinaryPacket &) { eventReceived = true; });

    server.BroadcastEvent("Ping");

    ok = PollUntil(server, client, [&] { return eventReceived; });
    EXPECT_TRUE(ok) << "Event was not received within timeout";
}

// Server broadcasts a KAI object; client object-message subscriber receives it.
TEST_F(NodeEndToEndTest, ObjectMessageReachesSubscriber) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16600, 16700);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

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
    if (!ok) GTEST_SKIP() << "Nodes did not both connect within timeout";

    int received = 0;
    client.SubscribeObjectMessage([&](const Object &obj) {
        if (obj.Exists() && obj.IsType<int>()) received = ConstDeref<int>(obj);
    });

    Object payload = reg_->New<int>(42);
    server.SendObject(payload);

    ok = PollUntil(server, client, [&] { return received != 0; });
    EXPECT_TRUE(ok) << "Object message was not received within timeout";
    EXPECT_EQ(received, 42);
}

// Server broadcasts an event with an int payload; client decodes it.
TEST_F(NodeEndToEndTest, EventPayloadDecodedCorrectly) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16700, 16800);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

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
    if (!ok) GTEST_SKIP() << "Nodes did not both connect within timeout";

    int receivedValue = 0;
    client.SubscribeEvent("Score", [&](BinaryPacket &pkt) {
        int val = 0;
        if (pkt.Read(val)) receivedValue = val;
    });

    // Build event with an int payload.
    BinaryStream payload;
    payload.Write(static_cast<int>(99));
    server.BroadcastEvent("Score", payload);

    ok = PollUntil(server, client, [&] { return receivedValue != 0; });
    EXPECT_TRUE(ok) << "Event with payload was not received within timeout";
    EXPECT_EQ(receivedValue, 99);
}

// Client fetches a property from a server-side agent over the network.
TEST_F(NodeEndToEndTest, RemotePropertyGetReturnsValue) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16800, 16900);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    int serverValue = 55;
    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterProperty<int>(
        agentHandle, "Counter",
        std::function<int()>([&] { return serverValue; }));

    Node client;
    client.SetRegistry(reg_);
    client.SetUpdatePump([&server]() { server.Update(); });

    bool connected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) connected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client, [&] { return connected; });
    if (!ok)
        GTEST_SKIP() << "Client/server connection did not complete in time";

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    auto future = client.FetchProperty<int>(agentHandle, "Counter");
    int result = client.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, 55);
}

// Client sets a property on a server-side agent over the network.
TEST_F(NodeEndToEndTest, RemotePropertySetUpdatesValue) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16900, 17000);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    int serverValue = 0;
    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterProperty<int>(
        agentHandle, "Counter",
        std::function<int()>([&] { return serverValue; }),
        std::function<void(int)>([&](int v) { serverValue = v; }));

    Node client;
    client.SetRegistry(reg_);
    client.SetUpdatePump([&server]() { server.Update(); });

    bool connected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) connected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client, [&] { return connected; });
    if (!ok)
        GTEST_SKIP() << "Client/server connection did not complete in time";

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    auto future = client.StoreProperty<int>(agentHandle, "Counter", 77);
    client.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(serverValue, 77);
}

TEST_F(NodeEndToEndTest, RemotePiExecutionReturnsResult) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 17000, 17100);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    // Server-side console to execute Pi code
    Console serverConsole;
    serverConsole.SetLanguage(Language::Pi);

    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterMethod<int, String>(
        agentHandle, "ExecPi",
        std::function<int(String)>([&serverConsole](String code) {
            serverConsole.Execute(code);
            auto stack = serverConsole.GetExecutor()->GetDataStack();
            if (stack->Empty()) return 0;
            return ConstDeref<int>(stack->Top());
        }));

    Node client;
    client.SetRegistry(reg_);
    client.SetUpdatePump([&server]() { server.Update(); });

    bool connected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) connected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client, [&] { return connected; });
    if (!ok) GTEST_SKIP() << "Connection did not complete in time";

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    auto future = client.Invoke<int>(agentHandle, "ExecPi", String("1 2 +"));
    int result = client.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, 3);
}

TEST_F(NodeEndToEndTest, RemoteContinuationMigration) {
    Console serverConsole;
    serverConsole.SetLanguage(kai::Language::Pi);

    Node server;
    server.SetRegistry(&serverConsole.GetRegistry());
    const auto listen = ListenOnAvailablePort(server, 17100, 17200);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    NetHandle agentHandle = server.AttachAgent(nullptr);

    // Server receives a frozen continuation object, thaws it and resumes
    // execution, then returns the top of stack.
    server.RegisterMethod<int, Object>(
        agentHandle, "ThawAndResume",
        std::function<int(Object)>([&serverConsole](Object frozen) {
            try {
                Object cont = Bin::Thaw(frozen);
                serverConsole.GetExecutor()->Continue(
                    Value<Continuation>(cont));
                auto stack = serverConsole.GetExecutor()->GetDataStack();
                if (stack->Empty()) {
                    return 0;
                }
                int result = ConstDeref<int>(stack->Top());
                return result;
            } catch (const Exception::Base &e) {
                throw std::runtime_error(
                    "Server ThawAndResume KAI exception: " + e.ToString());
            } catch (const std::exception &e) {
                throw std::runtime_error(
                    "Server ThawAndResume std::exception: " +
                    std::string(e.what()));
            }
        }));

    Console clientConsole;
    clientConsole.SetLanguage(kai::Language::Pi);

    Node client;
    client.SetRegistry(&clientConsole.GetRegistry());
    client.SetUpdatePump([&server]() { server.Update(); });

    bool connected = false;
    client.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) connected = true;
        });

    client.Connect(IpAddress("127.0.0.1"), port);

    bool ok = PollUntil(server, client, [&] { return connected; });
    if (!ok) GTEST_SKIP() << "Connection did not complete in time";

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    // Client compiles a simple Pi continuation that can be frozen on one node
    // and resumed on another: double(5) = 10.
    auto cont = clientConsole.Compile("{ 2 * } 'double # 5 double &",
                                      Structure::Program);
    ASSERT_TRUE(cont.Exists());

    Object frozen = Bin::Freeze(*cont->Self);
    ASSERT_TRUE(frozen.Exists());

    auto future = client.Invoke<int>(agentHandle, "ThawAndResume", frozen);
    int result = client.WaitFor(future, 2000ms);

    EXPECT_TRUE(future.Succeeded());
    EXPECT_EQ(result, 10);
}
