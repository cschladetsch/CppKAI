// Early returns: a `return` inside an `if` block, in a function called from
// a loop body, or where the `if` block calls a function before returning.
// Both used to be lost, so the function carried on after the `if`; see the
// two "Executor:" commits in CppKaiCore.

#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct RhoEarlyReturnInLoop : TestLangCommon {
    int Run(const char *code) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        console_.Execute(code, Structure::Program);
        EXPECT_FALSE(data_->Empty()) << code;
        if (data_->Empty()) return -999;
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<int>()) << code;
        return top.IsType<int>() ? ConstDeref<int>(top) : -999;
    }
};

TEST_F(RhoEarlyReturnInLoop, ForLoop) {
    EXPECT_EQ(Run(R"(
fun f(n)
    if n < 2
        return false
    return true
c = 0
for m = 0; m < 5; m = m + 1
    if f(m)
        c = c + 1
c
)"), 3);
}

TEST_F(RhoEarlyReturnInLoop, WhileLoop) {
    EXPECT_EQ(Run(R"(
fun f(n)
    if n < 2
        return false
    return true
c = 0
m = 0
while m < 5
    if f(m)
        c = c + 1
    m = m + 1
c
)"), 3);
}

TEST_F(RhoEarlyReturnInLoop, EarlyReturnValue) {
    EXPECT_EQ(Run(R"(
fun sign(n)
    if n < 0
        return 0 - 1
    if n == 0
        return 0
    return 1
s = 0
for m = 0; m < 5; m = m + 1
    s = s + sign(m - 2)
s
)"), 0);
}

// The loop body calls a function and then returns from the enclosing
// function itself: that return must still end the loop.
TEST_F(RhoEarlyReturnInLoop, ReturnFromLoopBodyAfterCall) {
    EXPECT_EQ(Run(R"(
fun twice(n)
    return n * 2
fun firstOver(limit)
    for i = 0; i < 10; i = i + 1
        x = twice(i)
        if x > limit
            return i
    return 0 - 1
firstOver(4)
)"), 3);
}

TEST_F(RhoEarlyReturnInLoop, BreakAfterCall) {
    EXPECT_EQ(Run(R"(
fun twice(n)
    return n * 2
c = 0
for i = 0; i < 10; i = i + 1
    c = c + twice(i)
    if i == 2
        break
c
)"), 6);
}

// The same, with no loop: the `if` block calls a function before it
// returns, so it is resumed from the context stack after that call, and its
// Return used to land back in the enclosing function instead of leaving it.
TEST_F(RhoEarlyReturnInLoop, ReturnOfCallInsideIf) {
    EXPECT_EQ(Run(R"(
fun tens(n)
    return n * 10
fun f(n)
    if n > 1
        return tens(n)
    return 0
f(4) + f(1)
)"), 40);
}

TEST_F(RhoEarlyReturnInLoop, ReturnAfterCallInsideIf) {
    EXPECT_EQ(Run(R"(
fun tens(n)
    return n * 10
fun f(n)
    if n > 1
        x = tens(n)
        return x + 1
    return 0
f(4)
)"), 41);
}

TEST_F(RhoEarlyReturnInLoop, ReturnOfCallInsideNestedIf) {
    EXPECT_EQ(Run(R"(
fun tens(n)
    return n * 10
fun f(n)
    if n > 1
        if n > 3
            return tens(n)
        return 1
    return 0
f(5) + f(2) + f(0)
)"), 51);
}

TEST_F(RhoEarlyReturnInLoop, ReturnOfCallInsideElse) {
    EXPECT_EQ(Run(R"(
fun tens(n)
    return n * 10
fun f(n)
    if n > 1
        return 1
    else
        return tens(n)
    return 99
f(3) + f(1)
)"), 11);
}

TEST_F(RhoEarlyReturnInLoop, ReturnOfCallInsideIfFromLoop) {
    EXPECT_EQ(Run(R"(
fun tens(n)
    return n * 10
fun f(n)
    if n > 1
        return tens(n)
    return 0
c = 0
for i = 0; i < 4; i = i + 1
    c = c + f(i)
c
)"), 50);
}

TEST_F(RhoEarlyReturnInLoop, RecursionThroughIf) {
    EXPECT_EQ(Run(R"(
fun fact(n)
    if n > 1
        return n * fact(n - 1)
    return 1
fact(6)
)"), 720);
}
