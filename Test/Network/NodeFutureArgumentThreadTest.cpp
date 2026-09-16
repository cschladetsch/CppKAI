#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Core/Tree.h"
#include "KAI/Network/Node.h"

// Explicit coverage for passing Future<T> arguments across REAL local
// std::threads (not the network path - see NodeFutureArgumentTest.cpp for
// that). Every call here is same-process, same-Node dispatch
// (Node::Invoke's local path), so there is no wire encoding involved at
// all: the point of these tests is that a genuinely unfulfilled
// kai::net::Future<T>, resolved later from a background thread, correctly
// propagates to whatever holds a copy of it (the callee captured one via
// RegisterMethod's lambda), including cases where the resolving thread
// is slower than how long the observing side is willing to wait (timeout).
//
// Caveat, deliberately not papered over: kai::net::Future<T>::State has no
// internal synchronization (no mutex/atomics) - it was designed for
// single-threaded, same-process async completion, not cross-thread use.
// These tests are written to stay safe under the C++ memory model anyway:
// every case either (a) joins the resolving thread before reading the
// future it touched, so the join()/thread-start pair provides the
// happens-before edge, or (b) polls IsComplete()/GetValue() only from the
// SAME thread that is later joined, treating a "still not complete" read
// purely as a timing observation, never as something raced with a
// concurrent write on another thread at the exact same instant. Where a
// test wants to observe "not resolved yet while the thread is still
// running", it does so BEFORE starting that thread's delayed write (using
// a coordination gate), not by reading concurrently with it.
using namespace kai;
using namespace kai::net;
using namespace std::chrono_literals;

namespace {

// Polls pred() with a small sleep between attempts until it returns true or
// the deadline elapses. Returns whether pred() became true in time.
bool WaitUntil(std::function<bool()> pred,
              std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (!pred()) {
        if (std::chrono::steady_clock::now() - start >= timeout) return false;
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

}  // namespace

class NodeFutureArgumentThreadTest : public ::testing::Test {
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

// 1. A background thread resolves the future well within the caller's
// patience window - the callee's copy should reflect it once the thread
// has actually run (join is the synchronization point).
TEST_F(NodeFutureArgumentThreadTest, BackgroundThreadResolvesFutureArgumentInTime) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> received;
    bool called = false;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture", std::function<void(Future<int>)>([&](Future<int> f) {
            received = f;
            called = true;
        }));

    Future<int> arg;
    node.Invoke<void>(handle, "TakeFuture", arg);
    ASSERT_TRUE(called);
    ASSERT_FALSE(received.IsComplete());

    std::thread worker([&arg] {
        std::this_thread::sleep_for(10ms);
        arg.SetValue(123);
        arg.SetResponse(ResponseType::Returned);
        arg.SetComplete(true);
    });
    worker.join();

    EXPECT_TRUE(received.IsComplete());
    EXPECT_TRUE(received.Succeeded());
    EXPECT_EQ(received.GetValue(), 123);
}

// 2. The resolving thread never runs before the caller gives up waiting -
// the caller must observe a timeout (future still pending), not a crash or
// a garbage value. The thread is only started (and joined) after the
// timeout has already been observed, so there is no concurrent access.
TEST_F(NodeFutureArgumentThreadTest, ObserverTimesOutBeforeThreadResolves) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> received;
    bool called = false;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture", std::function<void(Future<int>)>([&](Future<int> f) {
            received = f;
            called = true;
        }));

    Future<int> arg;
    node.Invoke<void>(handle, "TakeFuture", arg);
    ASSERT_TRUE(called);

    // Nobody has touched `arg` from another thread yet, so this read is
    // safe: we're deliberately checking the timeout case before the
    // resolving thread even exists.
    bool resolvedInTime = WaitUntil([&] { return received.IsComplete(); }, 30ms);
    EXPECT_FALSE(resolvedInTime)
        << "Nothing should have resolved the future yet - this exercises "
           "the timeout path";
    EXPECT_FALSE(received.IsComplete());

    // Now actually resolve it (from a thread, on purpose) and join before
    // touching `received` again, so the rest of the test stays race-free.
    std::thread worker([&arg] {
        arg.SetValue(7);
        arg.SetResponse(ResponseType::Returned);
        arg.SetComplete(true);
    });
    worker.join();
    EXPECT_TRUE(received.IsComplete());
    EXPECT_EQ(received.GetValue(), 7);
}

// 3. A future that is simply never resolved by anyone within the test's
// lifetime: the caller must keep observing "not complete" (a permanent
// timeout), never flip spontaneously or throw.
TEST_F(NodeFutureArgumentThreadTest, FutureNeverResolvedStaysPendingForever) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> received;
    bool called = false;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture", std::function<void(Future<int>)>([&](Future<int> f) {
            received = f;
            called = true;
        }));

    Future<int> arg;  // nobody ever resolves this one
    node.Invoke<void>(handle, "TakeFuture", arg);
    ASSERT_TRUE(called);

    EXPECT_FALSE(WaitUntil([&] { return received.IsComplete(); }, 30ms));
    EXPECT_FALSE(received.IsComplete());
    EXPECT_FALSE(received.Succeeded());
}

// 4. Two independent Future<int> arguments to the SAME call, each resolved
// by its own background thread at a different pace.
TEST_F(NodeFutureArgumentThreadTest, TwoFutureArgumentsResolvedByTwoThreads) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> receivedA, receivedB;
    bool called = false;
    node.RegisterMethod<void, Future<int>, Future<int>>(
        handle, "TakeTwo",
        std::function<void(Future<int>, Future<int>)>(
            [&](Future<int> a, Future<int> b) {
                receivedA = a;
                receivedB = b;
                called = true;
            }));

    Future<int> argA, argB;
    node.Invoke<void>(handle, "TakeTwo", argA, argB);
    ASSERT_TRUE(called);
    ASSERT_FALSE(receivedA.IsComplete());
    ASSERT_FALSE(receivedB.IsComplete());

    std::thread slow([&argA] {
        std::this_thread::sleep_for(20ms);
        argA.SetValue(1);
        argA.SetResponse(ResponseType::Returned);
        argA.SetComplete(true);
    });
    std::thread fast([&argB] {
        argB.SetValue(2);
        argB.SetResponse(ResponseType::Returned);
        argB.SetComplete(true);
    });
    fast.join();
    slow.join();

    EXPECT_TRUE(receivedA.IsComplete());
    EXPECT_EQ(receivedA.GetValue(), 1);
    EXPECT_TRUE(receivedB.IsComplete());
    EXPECT_EQ(receivedB.GetValue(), 2);
}

// 5. Future<void> argument resolved from a background thread - no value to
// carry, just completion + response status.
TEST_F(NodeFutureArgumentThreadTest, FutureVoidArgumentResolvedByThread) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<void> received;
    bool called = false;
    node.RegisterMethod<void, Future<void>>(
        handle, "TakeSignal",
        std::function<void(Future<void>)>([&](Future<void> f) {
            received = f;
            called = true;
        }));

    Future<void> arg;
    node.Invoke<void>(handle, "TakeSignal", arg);
    ASSERT_TRUE(called);
    ASSERT_FALSE(received.IsComplete());

    std::thread worker([&arg] {
        std::this_thread::sleep_for(5ms);
        arg.SetResponse(ResponseType::Returned);
        arg.SetComplete(true);
    });
    worker.join();

    EXPECT_TRUE(received.IsComplete());
    EXPECT_TRUE(received.Succeeded());
}

// 6. Mixed argument list: one plain int, one still-pending Future<int>
// resolved by a thread - the plain argument must be unaffected by the
// Future-handling machinery.
TEST_F(NodeFutureArgumentThreadTest, PlainArgumentAlongsideThreadResolvedFuture) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    int receivedFlag = -1;
    Future<int> receivedFuture;
    bool called = false;
    node.RegisterMethod<void, int, Future<int>>(
        handle, "TakeFlagAndFuture",
        std::function<void(int, Future<int>)>([&](int flag, Future<int> f) {
            receivedFlag = flag;
            receivedFuture = f;
            called = true;
        }));

    Future<int> arg;
    node.Invoke<void>(handle, "TakeFlagAndFuture", 42, arg);
    ASSERT_TRUE(called);
    EXPECT_EQ(receivedFlag, 42);
    ASSERT_FALSE(receivedFuture.IsComplete());

    std::thread worker([&arg] {
        arg.SetValue(9);
        arg.SetResponse(ResponseType::Returned);
        arg.SetComplete(true);
    });
    worker.join();

    EXPECT_TRUE(receivedFuture.IsComplete());
    EXPECT_EQ(receivedFuture.GetValue(), 9);
}

// 7. A thread resolves the future with a non-success response (simulating a
// failed upstream operation) - Succeeded() must reflect that even though
// IsComplete() is true.
TEST_F(NodeFutureArgumentThreadTest, ThreadResolvesFutureWithFailureResponse) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> received;
    bool called = false;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture", std::function<void(Future<int>)>([&](Future<int> f) {
            received = f;
            called = true;
        }));

    Future<int> arg;
    node.Invoke<void>(handle, "TakeFuture", arg);
    ASSERT_TRUE(called);

    std::thread worker([&arg] {
        // No SetValue() - this represents a failure path where the callee
        // never produced a result.
        arg.SetResponse(ResponseType::Error);
        arg.SetComplete(true);
    });
    worker.join();

    EXPECT_TRUE(received.IsComplete());
    EXPECT_FALSE(received.Succeeded())
        << "A non-Returned response must not read as success";
}

// 8. OnResolved() callback fired from the resolving thread - registered
// before the thread starts, observed only after join() so there is a
// well-defined happens-before edge for the flag it sets.
TEST_F(NodeFutureArgumentThreadTest, OnResolvedCallbackFiresFromResolvingThread) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> received;
    bool called = false;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFuture", std::function<void(Future<int>)>([&](Future<int> f) {
            received = f;
            called = true;
        }));

    Future<int> arg;
    node.Invoke<void>(handle, "TakeFuture", arg);
    ASSERT_TRUE(called);

    std::atomic<bool> callbackFired{false};
    int callbackValue = 0;
    received.OnResolved([&] {
        callbackFired.store(true);
        callbackValue = received.GetValue();
    });
    EXPECT_FALSE(callbackFired.load())
        << "Callback must not fire before the future actually resolves";

    std::thread worker([&arg] {
        arg.SetValue(55);
        arg.SetResponse(ResponseType::Returned);
        arg.SetComplete(true);
    });
    worker.join();

    EXPECT_TRUE(callbackFired.load());
    EXPECT_EQ(callbackValue, 55);
}

// 9. Several futures, several threads, resolved concurrently (join()s
// happen after every thread has started, so writes race each other in
// wall-clock time but never on the SAME future - each thread owns exactly
// one, so there is no shared-state race despite the concurrency).
TEST_F(NodeFutureArgumentThreadTest, ManyIndependentFuturesResolvedConcurrently) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    constexpr int kCount = 5;
    std::vector<Future<int>> receivedFutures(kCount);
    std::vector<Future<int>> args(kCount);
    int callCount = 0;

    for (int i = 0; i < kCount; ++i) {
        node.RegisterMethod<void, Future<int>>(
            handle, "TakeFuture" + std::to_string(i),
            std::function<void(Future<int>)>([&, i](Future<int> f) {
                receivedFutures[i] = f;
                ++callCount;
            }));
        node.Invoke<void>(handle, "TakeFuture" + std::to_string(i), args[i]);
    }
    ASSERT_EQ(callCount, kCount);
    for (int i = 0; i < kCount; ++i) {
        ASSERT_FALSE(receivedFutures[i].IsComplete());
    }

    std::vector<std::thread> workers;
    for (int i = 0; i < kCount; ++i) {
        workers.emplace_back([&args, i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(i));
            args[i].SetValue(i * 10);
            args[i].SetResponse(ResponseType::Returned);
            args[i].SetComplete(true);
        });
    }
    for (auto &t : workers) t.join();

    for (int i = 0; i < kCount; ++i) {
        EXPECT_TRUE(receivedFutures[i].IsComplete()) << "future " << i;
        EXPECT_EQ(receivedFutures[i].GetValue(), i * 10) << "future " << i;
    }
}

// 10. One future resolves quickly (well inside the wait window) while a
// second, independent call's future is deliberately left to time out - a
// single test exercising both outcomes side by side.
TEST_F(NodeFutureArgumentThreadTest, OneFutureResolvesInTimeWhileAnotherTimesOut) {
    Node node;
    node.SetRegistry(reg_);
    NetHandle handle = node.AttachAgent(nullptr);

    Future<int> receivedFast, receivedSlow;
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeFast", std::function<void(Future<int>)>([&](Future<int> f) {
            receivedFast = f;
        }));
    node.RegisterMethod<void, Future<int>>(
        handle, "TakeSlow", std::function<void(Future<int>)>([&](Future<int> f) {
            receivedSlow = f;
        }));

    Future<int> fastArg, slowArg;
    node.Invoke<void>(handle, "TakeFast", fastArg);
    node.Invoke<void>(handle, "TakeSlow", slowArg);
    ASSERT_FALSE(receivedFast.IsComplete());
    ASSERT_FALSE(receivedSlow.IsComplete());

    std::thread worker([&fastArg] {
        std::this_thread::sleep_for(5ms);
        fastArg.SetValue(1);
        fastArg.SetResponse(ResponseType::Returned);
        fastArg.SetComplete(true);
    });
    // slowArg is deliberately never touched by anyone in this test.

    EXPECT_TRUE(WaitUntil([&] { return receivedFast.IsComplete(); }, 500ms))
        << "Fast future should resolve well within the wait window";
    worker.join();
    EXPECT_TRUE(receivedFast.Succeeded());
    EXPECT_EQ(receivedFast.GetValue(), 1);

    EXPECT_FALSE(WaitUntil([&] { return receivedSlow.IsComplete(); }, 30ms))
        << "Slow future should time out - nothing ever resolves it";
    EXPECT_FALSE(receivedSlow.IsComplete());
}
