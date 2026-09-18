// Future<Future<Future<T>>> — three levels of nesting. Future<T> is just a
// shared_ptr<State<T>> wrapper, so nothing stops it nesting arbitrarily deep;
// this exercises that directly, extending NestedFutureTest.cpp and
// NestedFutureParamTests.cpp's two-level coverage by one more level.
//
// Scope note (same as the other NestedFuture* files): this is Future<T>'s
// own class mechanics only, not the network RPC argument-passing path.

#include <gtest/gtest.h>

#include "KAI/Network/Future.h"

using namespace kai;
using namespace kai::net;

// All three futures resolved outside-in, one at a time, with the two
// still-pending inner layers checked after each step.
TEST(NestedFutureTripleTest, ResolvesOutsideInOneLayerAtATime) {
    Future<int> innermost;
    Future<Future<int>> middle;
    Future<Future<Future<int>>> outer;

    ASSERT_FALSE(innermost.IsComplete());
    ASSERT_FALSE(middle.IsComplete());
    ASSERT_FALSE(outer.IsComplete());

    // Resolve the outermost future first; it carries the still-unresolved
    // middle future.
    outer.SetValue(middle);
    outer.SetResponse(ResponseType::Returned);
    outer.SetComplete(true);

    ASSERT_TRUE(outer.IsComplete());
    EXPECT_TRUE(outer.Succeeded());

    Future<Future<int>> unwrappedMiddle = outer.GetValue();
    EXPECT_FALSE(unwrappedMiddle.IsComplete())
        << "middle should still be pending right after outer resolves";

    // Resolve the middle future; it carries the still-unresolved innermost
    // future. Because Future<T> shares state via shared_ptr, this resolution
    // is visible through unwrappedMiddle too.
    middle.SetValue(innermost);
    middle.SetResponse(ResponseType::Returned);
    middle.SetComplete(true);

    EXPECT_TRUE(unwrappedMiddle.IsComplete());
    EXPECT_TRUE(unwrappedMiddle.Succeeded());

    Future<int> unwrappedInner = unwrappedMiddle.GetValue();
    EXPECT_FALSE(unwrappedInner.IsComplete())
        << "innermost should still be pending right after middle resolves";

    // Finally resolve the innermost future with the real value.
    innermost.SetValue(99);
    innermost.SetResponse(ResponseType::Returned);
    innermost.SetComplete(true);

    EXPECT_TRUE(unwrappedInner.IsComplete());
    EXPECT_TRUE(unwrappedInner.Succeeded());
    EXPECT_EQ(unwrappedInner.GetValue(), 99);
}

// All three futures are fully resolved before the outer future is ever
// touched or unwrapped; unwrapping should surface the value immediately at
// every layer with no waiting.
TEST(NestedFutureTripleTest, AllThreeLayersAlreadyResolvedBeforeUnwrapping) {
    Future<int> innermost;
    innermost.SetValue(-55);
    innermost.SetResponse(ResponseType::Returned);
    innermost.SetComplete(true);

    Future<Future<int>> middle;
    middle.SetValue(innermost);
    middle.SetResponse(ResponseType::Returned);
    middle.SetComplete(true);

    Future<Future<Future<int>>> outer;
    outer.SetValue(middle);
    outer.SetResponse(ResponseType::Returned);
    outer.SetComplete(true);

    ASSERT_TRUE(outer.IsComplete());
    EXPECT_TRUE(outer.Succeeded());

    Future<Future<int>> unwrappedMiddle = outer.GetValue();
    ASSERT_TRUE(unwrappedMiddle.IsComplete());
    EXPECT_TRUE(unwrappedMiddle.Succeeded());

    Future<int> unwrappedInner = unwrappedMiddle.GetValue();
    ASSERT_TRUE(unwrappedInner.IsComplete());
    EXPECT_TRUE(unwrappedInner.Succeeded());
    EXPECT_EQ(unwrappedInner.GetValue(), -55);
}
