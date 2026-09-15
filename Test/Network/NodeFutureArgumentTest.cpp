#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <string>
#include <thread>

#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Core/Tree.h"
#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Node.h"

// Regression / feature coverage for passing an UNFULFILLED Future<T> as an
// RPC method argument (not just as a return type). Two shapes are covered:
//
//   - Same-process ("local") calls, where Node::Invoke dispatches directly
//     to a locally registered method without touching the network.
//   - Real cross-network calls between two Node instances talking over a
//     loopback UDP socket in this same test process (following the exact
//     pattern NodeEndToEndTest.cpp already uses), so the future-argument
//     wire path (Node::PackInvokeArg -> Array [resolved?, value-or-id] ->
//     detail::MethodInvoker::ExtractArg -> Node::RegisterPendingFutureImport
//     -> ID_KAI_FUTURE_RESOLVE -> Node::CompletePendingFutureImport) is
//     exercised end to end, not just asserted about in isolation.
//
// In both cases: the caller passes a Future<int> that has NOT completed
// yet. The callee receives a real Future<int> (sharing state with whatever
// the caller eventually resolves) and can act on it once it completes -
// this is the "network continuation" case GenerateProxy/GenerateAgent's
// eager-resolve fix (see TauFutureArgumentCodeGenTests.cpp) deliberately
// does not attempt.

using namespace kai;
using namespace kai::net;
using namespace std::chrono_literals;

namespace {

// Pump both nodes until predicate is true or timeout expires.
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

}  // namespace

class NodeFutureArgumentTest : public ::testing::Test {
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

// Same-process dispatch: no listening, no connecting, just a Node calling
// its own registered method directly (Node::Invoke's "local path").
TEST_F(NodeFutureArgumentTest, LocalCallWithUnfulfilledFutureArgumentResolvesLater) {
    Node node;
    node.SetRegistry(reg_);

    Future<int> receivedFuture;
    bool called = false;

    NetHandle handle = node.AttachAgent(nullptr);
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture",
        std::function<void(Future<int>)>([&](Future<int> f) {
            receivedFuture = f;
            called = true;
        }));

    Future<int> arg;  // deliberately NOT resolved yet
    ASSERT_FALSE(arg.IsComplete());

    auto callFuture = node.Invoke<void>(handle, "TakeFuture", arg);

    ASSERT_TRUE(called) << "Local dispatch should be synchronous";
    ASSERT_FALSE(receivedFuture.IsComplete())
        << "Callee's future must still be pending, matching the caller's";

    // Resolve the caller's copy - since it shares state with what the
    // callee received, this should be enough, with no further Update()
    // pumping needed since this is a purely local (same-process) call.
    arg.SetValue(42);
    arg.SetResponse(ResponseType::Returned);
    arg.SetComplete(true);

    EXPECT_TRUE(receivedFuture.IsComplete())
        << "Callee's future should resolve once the caller's does";
    EXPECT_TRUE(receivedFuture.Succeeded());
    EXPECT_EQ(receivedFuture.GetValue(), 42);
}

// Real network call: an already-resolved Future<int> argument should just
// carry its value straight across on the first packet, no follow-up needed.
TEST_F(NodeFutureArgumentTest, RemoteCallWithAlreadyResolvedFutureArgumentArrivesImmediately) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16600, 16700);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    Future<int> receivedFuture;
    bool called = false;

    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterMethod<void, Future<int>>(
        agentHandle, "TakeFuture",
        std::function<void(Future<int>)>([&](Future<int> f) {
            receivedFuture = f;
            called = true;
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
    if (!PollUntil(server, client, [&] { return connected; })) {
        GTEST_SKIP() << "Client/server connection did not complete in time";
    }

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    Future<int> arg;
    arg.SetValue(7);
    arg.SetResponse(ResponseType::Returned);
    arg.SetComplete(true);

    client.Invoke<void>(agentHandle, "TakeFuture", arg);

    ASSERT_TRUE(PollUntil(server, client, [&] { return called; }))
        << "Server never received the call";
    EXPECT_TRUE(receivedFuture.IsComplete());
    EXPECT_TRUE(receivedFuture.Succeeded());
    EXPECT_EQ(receivedFuture.GetValue(), 7);
}

// The real target case: an UNFULFILLED Future<int> is passed as an
// argument over an actual network connection between two Node instances.
// The server's copy must start out pending and only resolve once the
// client resolves its own future and a resolution message crosses back.
TEST_F(NodeFutureArgumentTest, RemoteCallWithUnfulfilledFutureArgumentResolvesAcrossNetwork) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16700, 16800);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    Future<int> receivedFuture;
    bool called = false;

    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterMethod<void, Future<int>>(
        agentHandle, "TakeFuture",
        std::function<void(Future<int>)>([&](Future<int> f) {
            receivedFuture = f;
            called = true;
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
    if (!PollUntil(server, client, [&] { return connected; })) {
        GTEST_SKIP() << "Client/server connection did not complete in time";
    }

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    Future<int> arg;  // still pending when the call is made
    ASSERT_FALSE(arg.IsComplete());

    client.Invoke<void>(agentHandle, "TakeFuture", arg);

    ASSERT_TRUE(PollUntil(server, client, [&] { return called; }))
        << "Server never received the call";
    EXPECT_FALSE(receivedFuture.IsComplete())
        << "Server's future must still be pending - the client hasn't "
           "resolved its own copy yet";

    // Now resolve the client's future. This should trigger an
    // ID_KAI_FUTURE_RESOLVE message back to the server.
    arg.SetValue(99);
    arg.SetResponse(ResponseType::Returned);
    arg.SetComplete(true);

    ASSERT_TRUE(PollUntil(server, client,
                         [&] { return receivedFuture.IsComplete(); }))
        << "Server's future never resolved after the client's did";
    EXPECT_TRUE(receivedFuture.Succeeded());
    EXPECT_EQ(receivedFuture.GetValue(), 99);
}

// Future<void> arguments carry no value, only a completion signal - make
// sure the pending/resolved encoding still works with nothing to send.
TEST_F(NodeFutureArgumentTest, RemoteCallWithUnfulfilledFutureVoidArgumentResolves) {
    Node server;
    server.SetRegistry(reg_);
    const auto listen = ListenOnAvailablePort(server, 16800, 16900);
    if (listen.port == 0) GTEST_SKIP() << listen.skipReason;
    const int port = listen.port;

    Future<void> receivedFuture;
    bool called = false;

    NetHandle agentHandle = server.AttachAgent(nullptr);
    server.RegisterMethod<void, Future<void>>(
        agentHandle, "TakeSignal",
        std::function<void(Future<void>)>([&](Future<void> f) {
            receivedFuture = f;
            called = true;
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
    if (!PollUntil(server, client, [&] { return connected; })) {
        GTEST_SKIP() << "Client/server connection did not complete in time";
    }

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentHandle, serverAddr);

    Future<void> arg;
    ASSERT_FALSE(arg.IsComplete());

    client.Invoke<void>(agentHandle, "TakeSignal", arg);

    ASSERT_TRUE(PollUntil(server, client, [&] { return called; }));
    EXPECT_FALSE(receivedFuture.IsComplete());

    arg.SetResponse(ResponseType::Returned);
    arg.SetComplete(true);

    ASSERT_TRUE(PollUntil(server, client,
                         [&] { return receivedFuture.IsComplete(); }));
    EXPECT_TRUE(receivedFuture.Succeeded());
}
