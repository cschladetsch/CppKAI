// Future<T> is a thin template around a shared_ptr<State<T>>, so nesting it
// should instantiate cleanly. This exercises only the Future<T> class itself
// (SetValue/SetComplete/GetValue) — NOT the network RPC argument-passing
// path (PackInvokeArg/RegisterPendingFutureImport), which assumes T can be
// turned into a Registry Object and has no evidence of supporting Future<T>
// as an element type. A real cross-network Future<Future<T>> argument test
// would need to be written and checked against actual compiler output
// first, not assumed to work.

#include <gtest/gtest.h>

#include "KAI/Network/Future.h"

using namespace kai;
using namespace kai::net;

TEST(NestedFutureTest, OuterFutureCarriesInnerUnresolvedFuture) {
    Future<int> inner;
    ASSERT_FALSE(inner.IsComplete());

    Future<Future<int>> outer;
    ASSERT_FALSE(outer.IsComplete());

    outer.SetValue(inner);
    outer.SetResponse(ResponseType::Returned);
    outer.SetComplete(true);

    ASSERT_TRUE(outer.IsComplete());
    EXPECT_TRUE(outer.Succeeded());

    Future<int> unwrapped = outer.GetValue();
    EXPECT_FALSE(unwrapped.IsComplete())
        << "The inner future should still be pending — resolving the "
           "outer future resolves delivery of the inner future, not the "
           "inner future's own value";

    // Resolving the original inner future should be visible through the
    // unwrapped copy too, since Future<T> shares state via shared_ptr.
    inner.SetValue(123);
    inner.SetResponse(ResponseType::Returned);
    inner.SetComplete(true);

    EXPECT_TRUE(unwrapped.IsComplete());
    EXPECT_TRUE(unwrapped.Succeeded());
    EXPECT_EQ(unwrapped.GetValue(), 123);
}

TEST(NestedFutureTest, OuterFutureCarriesAlreadyResolvedInnerFuture) {
    Future<int> inner;
    inner.SetValue(7);
    inner.SetResponse(ResponseType::Returned);
    inner.SetComplete(true);

    Future<Future<int>> outer;
    outer.SetValue(inner);
    outer.SetResponse(ResponseType::Returned);
    outer.SetComplete(true);

    Future<int> unwrapped = outer.GetValue();
    EXPECT_TRUE(unwrapped.IsComplete());
    EXPECT_TRUE(unwrapped.Succeeded());
    EXPECT_EQ(unwrapped.GetValue(), 7);
}
