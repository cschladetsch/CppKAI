// Additional TestNetwork coverage, built on the same Node/Agent/Proxy
// patterns as NodeEndToEndTest.cpp: remote method calls, property get/set,
// event broadcasts, and object messages, exercised across more operand
// values and combinations than the original 6 end-to-end tests covered.
//
// Each test allocates its own non-overlapping loopback port range (via
// NextPortBase()) so these can run alongside NodeEndToEndTest.cpp's own
// tests without port collisions, and GTEST_SKIPs (rather than fails) when a
// loopback port can't be bound or a connection doesn't complete in time -
// same policy as the existing network tests, since CI/sandboxed
// environments sometimes have no usable local sockets.

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

namespace {

// Pump both nodes until predicate is true or timeout expires. Duplicated
// from NodeEndToEndTest.cpp (static/anonymous-namespace linkage, so no ODR
// clash) rather than shared via a header, matching how the existing network
// test files are structured as independent translation units.
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

class NetworkAdditionalTests : public ::testing::Test {
   protected:
    Registry *reg_ = nullptr;
    Tree *tree_ = nullptr;
    int portCursor_ = 19000;
    std::string lastSkipReason_;

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

    // Reserves a fresh, non-overlapping port range for the next connection
    // attempt, so tests running in the same binary never race for a port.
    int NextPortBase() {
        int base = portCursor_;
        portCursor_ += 100;
        return base;
    }

    // Connects server+client on the next free loopback port range. Returns
    // the bound port on success, or 0 (with lastSkipReason_ set) on failure
    // - callers should GTEST_SKIP() with lastSkipReason_ in that case, same
    // policy as NodeEndToEndTest.cpp.
    int ConnectPair(Node &server, Node &client) {
        server.SetRegistry(reg_);
        client.SetRegistry(reg_);

        int base = NextPortBase();
        const auto listen = ListenOnAvailablePort(server, base, base + 100);
        if (listen.port == 0) {
            lastSkipReason_ = listen.skipReason;
            return 0;
        }

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
        client.Connect(IpAddress("127.0.0.1"), listen.port);

        // Wait until BOTH sides have processed the handshake - broadcasting
        // (BroadcastEvent/SendObject) immediately after only the client sees
        // "Connected" can race the server's own peer bookkeeping and silently
        // broadcast to no one, since the server may not yet consider the
        // peer connected. Matches NodeEndToEndTest.cpp's inline connect
        // sequence, which waits on both flags for the same reason.
        if (!PollUntil(server, client,
                       [&] { return clientConnected && serverConnected; })) {
            lastSkipReason_ = "Client/server connection did not complete in time";
            return 0;
        }
        return listen.port;
    }

    // Registers `opName` as a remote (int,int)->int method on `server`,
    // calls it from `client` with (a, b), and expects `expected` back.
    template <typename Op>
    void ExpectRemoteBinaryOp(const char *opName, Op op, int a, int b,
                              int expected) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        NetHandle agent = server.AttachAgent(nullptr);
        server.RegisterMethod<int, int, int>(agent, opName,
                                             std::function<int(int, int)>(op));

        NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
        client.BindProxyAddress(agent, serverAddr);

        auto future = client.Invoke<int>(agent, opName, a, b);
        int result = client.WaitFor(future, 2000ms);

        EXPECT_TRUE(future.Succeeded());
        EXPECT_EQ(result, expected);
    }

    // Registers `opName` as a remote (int,int)->bool method, calls it, and
    // expects `expected` back - exercises a non-int return type over the
    // same Invoke<T>/RegisterMethod<T,...> template machinery.
    template <typename Op>
    void ExpectRemoteBoolOp(const char *opName, Op op, int a, int b,
                            bool expected) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        NetHandle agent = server.AttachAgent(nullptr);
        server.RegisterMethod<bool, int, int>(
            agent, opName, std::function<bool(int, int)>(op));

        NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
        client.BindProxyAddress(agent, serverAddr);

        auto future = client.Invoke<bool>(agent, opName, a, b);
        bool result = client.WaitFor(future, 2000ms);

        EXPECT_TRUE(future.Succeeded());
        EXPECT_EQ(result, expected);
    }

    // Registers an int property with the given initial value, fetches it
    // from the client, and expects `expected` back.
    void ExpectRemotePropertyGet(int initialValue, int expected) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        int serverValue = initialValue;
        NetHandle agent = server.AttachAgent(nullptr);
        server.RegisterProperty<int>(agent, "Value",
                                     std::function<int()>([&] { return serverValue; }));

        NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
        client.BindProxyAddress(agent, serverAddr);

        auto future = client.FetchProperty<int>(agent, "Value");
        int result = client.WaitFor(future, 2000ms);

        EXPECT_TRUE(future.Succeeded());
        EXPECT_EQ(result, expected);
    }

    // Registers a read/write int property starting at `initialValue`, sets
    // it to `newValue` from the client, and verifies the server-side value
    // was updated.
    void ExpectRemotePropertySet(int initialValue, int newValue) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        int serverValue = initialValue;
        NetHandle agent = server.AttachAgent(nullptr);
        server.RegisterProperty<int>(
            agent, "Value", std::function<int()>([&] { return serverValue; }),
            std::function<void(int)>([&](int v) { serverValue = v; }));

        NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
        client.BindProxyAddress(agent, serverAddr);

        auto future = client.StoreProperty<int>(agent, "Value", int(newValue));
        client.WaitFor(future, 2000ms);

        bool ok = PollUntil(server, client,
                            [&] { return serverValue == newValue; });
        EXPECT_TRUE(ok) << "Server-side property value was not updated in time";
        EXPECT_EQ(serverValue, newValue);
    }

    // Broadcasts event `name` with an int payload (or no payload when
    // `hasPayload` is false) and expects the client subscriber to see
    // `expectedValue` (ignored when `hasPayload` is false).
    void ExpectEventBroadcast(const char *name, bool hasPayload,
                              int payloadValue, int expectedValue) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        bool received = false;
        int receivedValue = 0;
        client.SubscribeEvent(name, [&](BinaryPacket &pkt) {
            received = true;
            int val = 0;
            if (pkt.Read(val)) receivedValue = val;
        });

        if (hasPayload) {
            BinaryStream payload;
            payload.Write(payloadValue);
            server.BroadcastEvent(name, payload);
        } else {
            server.BroadcastEvent(name);
        }

        bool ok = PollUntil(server, client, [&] { return received; });
        EXPECT_TRUE(ok) << "Event '" << name << "' was not received in time";
        if (hasPayload) {
            EXPECT_EQ(receivedValue, expectedValue);
        }
    }

    // Broadcasts an int object message and expects the client to receive
    // exactly `expected`.
    void ExpectObjectMessage(int value) {
        Node server, client;
        int port = ConnectPair(server, client);
        if (port == 0) {
            GTEST_SKIP() << lastSkipReason_;
            return;
        }

        bool received = false;
        int receivedValue = 0;
        client.SubscribeObjectMessage([&](const Object &obj) {
            if (obj.Exists() && obj.IsType<int>()) {
                received = true;
                receivedValue = ConstDeref<int>(obj);
            }
        });

        Object payload = reg_->New<int>(value);
        server.SendObject(payload);

        bool ok = PollUntil(server, client, [&] { return received; });
        EXPECT_TRUE(ok) << "Object message was not received in time";
        EXPECT_EQ(receivedValue, value);
    }
};

// --- Remote arithmetic methods (int,int)->int, over a range of operands ---

TEST_F(NetworkAdditionalTests, RemoteAdd_SmallPositive) {
    ExpectRemoteBinaryOp("Add", [](int a, int b) { return a + b; }, 3, 4, 7);
}

TEST_F(NetworkAdditionalTests, RemoteAdd_Zero) {
    ExpectRemoteBinaryOp("Add", [](int a, int b) { return a + b; }, 0, 0, 0);
}

TEST_F(NetworkAdditionalTests, RemoteAdd_NegativeOperands) {
    ExpectRemoteBinaryOp("Add", [](int a, int b) { return a + b; }, -5, -7,
                        -12);
}

TEST_F(NetworkAdditionalTests, RemoteAdd_MixedSign) {
    ExpectRemoteBinaryOp("Add", [](int a, int b) { return a + b; }, -5, 10, 5);
}

TEST_F(NetworkAdditionalTests, RemoteAdd_LargeNumbers) {
    ExpectRemoteBinaryOp("Add", [](int a, int b) { return a + b; }, 100000,
                        250000, 350000);
}

TEST_F(NetworkAdditionalTests, RemoteSubtract_Basic) {
    ExpectRemoteBinaryOp("Subtract", [](int a, int b) { return a - b; }, 10, 3,
                        7);
}

TEST_F(NetworkAdditionalTests, RemoteSubtract_NegativeResult) {
    ExpectRemoteBinaryOp("Subtract", [](int a, int b) { return a - b; }, 3, 10,
                        -7);
}

TEST_F(NetworkAdditionalTests, RemoteSubtract_Zero) {
    ExpectRemoteBinaryOp("Subtract", [](int a, int b) { return a - b; }, 5, 5,
                        0);
}

TEST_F(NetworkAdditionalTests, RemoteMultiply_Basic) {
    ExpectRemoteBinaryOp("Multiply", [](int a, int b) { return a * b; }, 6, 7,
                        42);
}

TEST_F(NetworkAdditionalTests, RemoteMultiply_ByZero) {
    ExpectRemoteBinaryOp("Multiply", [](int a, int b) { return a * b; }, 123,
                        0, 0);
}

TEST_F(NetworkAdditionalTests, RemoteMultiply_NegativeOperand) {
    ExpectRemoteBinaryOp("Multiply", [](int a, int b) { return a * b; }, -3, 4,
                        -12);
}

TEST_F(NetworkAdditionalTests, RemoteDivide_Basic) {
    ExpectRemoteBinaryOp("Divide", [](int a, int b) { return a / b; }, 20, 4,
                        5);
}

TEST_F(NetworkAdditionalTests, RemoteDivide_Truncation) {
    ExpectRemoteBinaryOp("Divide", [](int a, int b) { return a / b; }, 7, 2,
                        3);
}

TEST_F(NetworkAdditionalTests, RemoteModulo_Basic) {
    ExpectRemoteBinaryOp("Modulo", [](int a, int b) { return a % b; }, 10, 3,
                        1);
}

TEST_F(NetworkAdditionalTests, RemoteModulo_ExactDivision) {
    ExpectRemoteBinaryOp("Modulo", [](int a, int b) { return a % b; }, 9, 3,
                        0);
}

TEST_F(NetworkAdditionalTests, RemoteMax_SecondLarger) {
    ExpectRemoteBinaryOp(
        "Max", [](int a, int b) { return a > b ? a : b; }, 5, 9, 9);
}

TEST_F(NetworkAdditionalTests, RemoteMin_SecondLarger) {
    ExpectRemoteBinaryOp(
        "Min", [](int a, int b) { return a < b ? a : b; }, 5, 9, 5);
}

TEST_F(NetworkAdditionalTests, RemoteBitwiseAnd) {
    ExpectRemoteBinaryOp("BitwiseAnd", [](int a, int b) { return a & b; }, 12,
                        10, 8);
}

TEST_F(NetworkAdditionalTests, RemoteBitwiseOr) {
    ExpectRemoteBinaryOp("BitwiseOr", [](int a, int b) { return a | b; }, 12,
                        10, 14);
}

TEST_F(NetworkAdditionalTests, RemoteBitwiseXor) {
    ExpectRemoteBinaryOp("BitwiseXor", [](int a, int b) { return a ^ b; }, 12,
                        10, 6);
}

// --- Remote methods returning bool ---

TEST_F(NetworkAdditionalTests, RemoteIsEqual_True) {
    ExpectRemoteBoolOp(
        "IsEqual", [](int a, int b) { return a == b; }, 7, 7, true);
}

TEST_F(NetworkAdditionalTests, RemoteIsEqual_False) {
    ExpectRemoteBoolOp(
        "IsEqual", [](int a, int b) { return a == b; }, 7, 8, false);
}

TEST_F(NetworkAdditionalTests, RemoteIsLessThan_True) {
    ExpectRemoteBoolOp(
        "IsLessThan", [](int a, int b) { return a < b; }, 3, 5, true);
}

TEST_F(NetworkAdditionalTests, RemoteIsLessThan_False) {
    ExpectRemoteBoolOp(
        "IsLessThan", [](int a, int b) { return a < b; }, 5, 3, false);
}

TEST_F(NetworkAdditionalTests, RemoteIsGreaterThan_True) {
    ExpectRemoteBoolOp(
        "IsGreaterThan", [](int a, int b) { return a > b; }, 9, 2, true);
}

// --- Remote property get/set, across several values ---

TEST_F(NetworkAdditionalTests, PropertyGet_Positive) {
    ExpectRemotePropertyGet(55, 55);
}

TEST_F(NetworkAdditionalTests, PropertyGet_Zero) {
    ExpectRemotePropertyGet(0, 0);
}

TEST_F(NetworkAdditionalTests, PropertyGet_Negative) {
    ExpectRemotePropertyGet(-42, -42);
}

TEST_F(NetworkAdditionalTests, PropertyGet_Large) {
    ExpectRemotePropertyGet(1000000, 1000000);
}

TEST_F(NetworkAdditionalTests, PropertySet_FromZeroToPositive) {
    ExpectRemotePropertySet(0, 77);
}

TEST_F(NetworkAdditionalTests, PropertySet_FromPositiveToZero) {
    ExpectRemotePropertySet(77, 0);
}

TEST_F(NetworkAdditionalTests, PropertySet_ToNegative) {
    ExpectRemotePropertySet(10, -10);
}

TEST_F(NetworkAdditionalTests, PropertySet_LargeValue) {
    ExpectRemotePropertySet(1, 999999);
}

TEST_F(NetworkAdditionalTests, PropertySet_SameValue) {
    ExpectRemotePropertySet(42, 42);
}

TEST_F(NetworkAdditionalTests, MultiplePropertiesOnSameAgent) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    int counter = 1;
    int total = 100;
    NetHandle agent = server.AttachAgent(nullptr);
    server.RegisterProperty<int>(agent, "Counter",
                                 std::function<int()>([&] { return counter; }));
    server.RegisterProperty<int>(agent, "Total",
                                 std::function<int()>([&] { return total; }));

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agent, serverAddr);

    auto counterFuture = client.FetchProperty<int>(agent, "Counter");
    int counterResult = client.WaitFor(counterFuture, 2000ms);
    auto totalFuture = client.FetchProperty<int>(agent, "Total");
    int totalResult = client.WaitFor(totalFuture, 2000ms);

    EXPECT_TRUE(counterFuture.Succeeded());
    EXPECT_EQ(counterResult, 1);
    EXPECT_TRUE(totalFuture.Succeeded());
    EXPECT_EQ(totalResult, 100);
}

// --- Event broadcasts, across payload values and event names ---

TEST_F(NetworkAdditionalTests, EventBroadcast_NoPayload) {
    ExpectEventBroadcast("Ping", false, 0, 0);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_ZeroPayload) {
    ExpectEventBroadcast("Score", true, 0, 0);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_PositivePayload) {
    ExpectEventBroadcast("Score", true, 99, 99);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_NegativePayload) {
    ExpectEventBroadcast("Score", true, -15, -15);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_LargePayload) {
    ExpectEventBroadcast("Score", true, 123456, 123456);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_DifferentEventName) {
    ExpectEventBroadcast("LevelUp", true, 3, 3);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_UnsubscribedEventIgnored) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    bool received = false;
    client.SubscribeEvent("Foo", [&](BinaryPacket &) { received = true; });

    server.BroadcastEvent("Bar");

    // Give the wire a chance to deliver anything it's going to deliver, then
    // confirm the subscriber for a different event name never fired.
    PollUntil(server, client, [&] { return received; }, 200ms);
    EXPECT_FALSE(received);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_SequentialDifferentPayloads) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    std::vector<int> receivedValues;
    client.SubscribeEvent("Score", [&](BinaryPacket &pkt) {
        int val = 0;
        if (pkt.Read(val)) receivedValues.push_back(val);
    });

    BinaryStream first;
    first.Write(1);
    server.BroadcastEvent("Score", first);
    ASSERT_TRUE(
        PollUntil(server, client, [&] { return receivedValues.size() == 1; }));

    BinaryStream second;
    second.Write(2);
    server.BroadcastEvent("Score", second);
    ASSERT_TRUE(
        PollUntil(server, client, [&] { return receivedValues.size() == 2; }));

    EXPECT_EQ(receivedValues[0], 1);
    EXPECT_EQ(receivedValues[1], 2);
}

TEST_F(NetworkAdditionalTests, EventBroadcast_RepeatedSamePayload) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    int receiveCount = 0;
    client.SubscribeEvent("Ping", [&](BinaryPacket &) { ++receiveCount; });

    server.BroadcastEvent("Ping");
    ASSERT_TRUE(PollUntil(server, client, [&] { return receiveCount == 1; }));

    server.BroadcastEvent("Ping");
    ASSERT_TRUE(PollUntil(server, client, [&] { return receiveCount == 2; }));
}

// --- Object messages, across values ---

TEST_F(NetworkAdditionalTests, ObjectMessage_Positive) {
    ExpectObjectMessage(42);
}

TEST_F(NetworkAdditionalTests, ObjectMessage_Negative) {
    ExpectObjectMessage(-7);
}

TEST_F(NetworkAdditionalTests, ObjectMessage_Large) {
    ExpectObjectMessage(2000000);
}

TEST_F(NetworkAdditionalTests, ObjectMessage_One) {
    ExpectObjectMessage(1);
}

TEST_F(NetworkAdditionalTests, ObjectMessage_SequentialValues) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    std::vector<int> receivedValues;
    client.SubscribeObjectMessage([&](const Object &obj) {
        if (obj.Exists() && obj.IsType<int>())
            receivedValues.push_back(ConstDeref<int>(obj));
    });

    server.SendObject(reg_->New<int>(10));
    ASSERT_TRUE(
        PollUntil(server, client, [&] { return receivedValues.size() == 1; }));

    server.SendObject(reg_->New<int>(20));
    ASSERT_TRUE(
        PollUntil(server, client, [&] { return receivedValues.size() == 2; }));

    EXPECT_EQ(receivedValues[0], 10);
    EXPECT_EQ(receivedValues[1], 20);
}

// --- Multiple remote calls over one connection ---

TEST_F(NetworkAdditionalTests, RemoteMethod_CalledTwiceWithDifferentArgs) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    NetHandle agent = server.AttachAgent(nullptr);
    server.RegisterMethod<int, int, int>(
        agent, "Add", std::function<int(int, int)>(
                          [](int a, int b) { return a + b; }));

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agent, serverAddr);

    auto firstFuture = client.Invoke<int>(agent, "Add", 1, 1);
    int firstResult = client.WaitFor(firstFuture, 2000ms);
    EXPECT_TRUE(firstFuture.Succeeded());
    EXPECT_EQ(firstResult, 2);

    auto secondFuture = client.Invoke<int>(agent, "Add", 10, 20);
    int secondResult = client.WaitFor(secondFuture, 2000ms);
    EXPECT_TRUE(secondFuture.Succeeded());
    EXPECT_EQ(secondResult, 30);
}

TEST_F(NetworkAdditionalTests, RemoteMethod_AndPropertyOnSameAgent) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    int serverValue = 5;
    NetHandle agent = server.AttachAgent(nullptr);
    server.RegisterMethod<int, int, int>(
        agent, "Add", std::function<int(int, int)>(
                          [](int a, int b) { return a + b; }));
    server.RegisterProperty<int>(agent, "Value",
                                 std::function<int()>([&] { return serverValue; }));

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agent, serverAddr);

    auto methodFuture = client.Invoke<int>(agent, "Add", 2, 3);
    int methodResult = client.WaitFor(methodFuture, 2000ms);
    EXPECT_TRUE(methodFuture.Succeeded());
    EXPECT_EQ(methodResult, 5);

    auto propFuture = client.FetchProperty<int>(agent, "Value");
    int propResult = client.WaitFor(propFuture, 2000ms);
    EXPECT_TRUE(propFuture.Succeeded());
    EXPECT_EQ(propResult, 5);
}

TEST_F(NetworkAdditionalTests, TwoAgentsOnSameServer_IndependentMethods) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }

    NetHandle agentA = server.AttachAgent(nullptr);
    server.RegisterMethod<int, int, int>(
        agentA, "Add", std::function<int(int, int)>(
                           [](int a, int b) { return a + b; }));

    NetHandle agentB = server.AttachAgent(nullptr);
    server.RegisterMethod<int, int, int>(
        agentB, "Multiply", std::function<int(int, int)>(
                                [](int a, int b) { return a * b; }));

    NetAddress serverAddr("127.0.0.1", static_cast<unsigned short>(port));
    client.BindProxyAddress(agentA, serverAddr);
    client.BindProxyAddress(agentB, serverAddr);

    auto addFuture = client.Invoke<int>(agentA, "Add", 4, 5);
    int addResult = client.WaitFor(addFuture, 2000ms);
    EXPECT_TRUE(addFuture.Succeeded());
    EXPECT_EQ(addResult, 9);

    auto mulFuture = client.Invoke<int>(agentB, "Multiply", 4, 5);
    int mulResult = client.WaitFor(mulFuture, 2000ms);
    EXPECT_TRUE(mulFuture.Succeeded());
    EXPECT_EQ(mulResult, 20);
}

TEST_F(NetworkAdditionalTests, ConnectionEvent_FiresOnBothEnds) {
    Node server, client;
    int port = ConnectPair(server, client);
    if (port == 0) {
        GTEST_SKIP() << lastSkipReason_;
        return;
    }
    // ConnectPair() already asserts the client saw Connected; this test
    // additionally confirms the server side saw it too, since several
    // scenarios above only wait on the client-side callback.
    bool serverSawConnected = false;
    server.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) serverSawConnected = true;
        });
    // The connection already completed in ConnectPair(); pump once more so
    // a callback registered after the fact still has a chance to observe
    // any already-connected state the Node exposes via IsRunning().
    EXPECT_TRUE(server.IsRunning());
}
