// Parameterized coverage of Future<Future<T>> across varied values, extending
// NestedFutureTest.cpp's two hand-written cases with a wider sweep.
//
// Scope note (same as NestedFutureTest.cpp): this exercises only the
// Future<T> class itself (SetValue/SetComplete/GetValue/OnResolved) via its
// shared_ptr<State> plumbing. It does NOT exercise the network RPC
// argument-passing path (PackInvokeArg/RegisterPendingFutureImport), which
// has no evidence of supporting Future<T> as an element type — that would
// need its own investigation before being assumed to work.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "KAI/Network/Future.h"

using namespace kai;
using namespace kai::net;

namespace {

struct NestedFutureCase {
    int value;
    std::string label;
};

std::vector<NestedFutureCase> BuildNestedFutureCases() {
    std::vector<NestedFutureCase> cases;
    // Spread across negative, zero, small, and large values.
    int values[] = {-1000, -7, -1, 0, 1, 7, 42, 1000, 123456, -123456};
    for (int i = 0; i < 10; ++i) {
        cases.push_back({values[i], "Value" + std::to_string(i)});
    }
    return cases;
}

}  // namespace

class NestedFutureParamTest
    : public ::testing::TestWithParam<NestedFutureCase> {};

// Outer resolves first, carrying a still-pending inner future; the inner
// future is resolved afterward and that resolution is visible through the
// unwrapped copy (shared state via shared_ptr).
TEST_P(NestedFutureParamTest, InnerResolvesAfterOuter) {
    const NestedFutureCase &tc = GetParam();

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
        << tc.label << ": inner should still be pending right after outer resolves";

    inner.SetValue(tc.value);
    inner.SetResponse(ResponseType::Returned);
    inner.SetComplete(true);

    EXPECT_TRUE(unwrapped.IsComplete()) << tc.label;
    EXPECT_TRUE(unwrapped.Succeeded()) << tc.label;
    EXPECT_EQ(unwrapped.GetValue(), tc.value) << tc.label;
}

// Inner future is already fully resolved before the outer future is ever
// touched; the value should come through immediately once the outer is
// resolved and unwrapped.
TEST_P(NestedFutureParamTest, InnerAlreadyResolvedBeforeOuter) {
    const NestedFutureCase &tc = GetParam();

    Future<int> inner;
    inner.SetValue(tc.value);
    inner.SetResponse(ResponseType::Returned);
    inner.SetComplete(true);
    ASSERT_TRUE(inner.IsComplete());

    Future<Future<int>> outer;
    outer.SetValue(inner);
    outer.SetResponse(ResponseType::Returned);
    outer.SetComplete(true);

    ASSERT_TRUE(outer.IsComplete()) << tc.label;
    EXPECT_TRUE(outer.Succeeded()) << tc.label;

    Future<int> unwrapped = outer.GetValue();
    EXPECT_TRUE(unwrapped.IsComplete()) << tc.label;
    EXPECT_TRUE(unwrapped.Succeeded()) << tc.label;
    EXPECT_EQ(unwrapped.GetValue(), tc.value) << tc.label;
}

INSTANTIATE_TEST_SUITE_P(
    ManyValues, NestedFutureParamTest, ::testing::ValuesIn(BuildNestedFutureCases()),
    [](const ::testing::TestParamInfo<NestedFutureCase> &info) {
        return info.param.label;
    });
